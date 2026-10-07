//----------------------------------------------------------------------------
//! @file   SoundSystem.cpp
//! @brief  効果音のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/SoundSystem.hpp>

#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/JsonReader.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Audio/AudioManager.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/Asset/Audio/AudioAsset.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    SoundSystem::SoundSystem(Tsukino::ECS::EventBus& eventBus, const std::string& path)
        : m_eventBus(eventBus) {
        Load(path);

        // ハンドラでは積むだけにして、鳴らすのは Update で行う
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.zone == DropZone::Gutter)
                m_pending.push_back("gutter");
            else
                m_pending.push_back(e.kind == PrizeKind::Coin ? "coinPayout" : "fruitGet");
        });
        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent&) { m_pending.push_back("zukanNew"); });
        m_castConnection  = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent&) { m_pending.push_back("magicCast"); });
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    void SoundSystem::Load(const std::string& path) {
        if(!Parse(path, m_sounds, m_music, m_masterVolume))
            Tsukino::Core::Log::Warn("SoundSystem: cannot read \"sounds\" from " + path + ". No sound will play.");
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルに書かれた効果音のファイルを列挙します。
    //----------------------------------------------------------------------------
    std::vector<std::string> SoundSystem::ListSoundFiles(const std::string& path) {
        std::unordered_map<std::string, Sound> sounds;
        Sound                                  music;
        float                                  masterVolume = 0.0f;
        Parse(path, sounds, music, masterVolume);

        std::vector<std::string> files;
        for(const auto& [name, sound] : sounds) {
            if(std::find(files.begin(), files.end(), sound.file) == files.end())
                files.push_back(sound.file);
        }
        // BGM は大きく、初回の変換に時間がかかるので、ロード画面で先に変換しておく
        if(!music.file.empty())
            files.push_back(music.file);
        return files;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み、効果音の設定と全体の音量を取り出します。
    //----------------------------------------------------------------------------
    bool SoundSystem::Parse(const std::string& path, std::unordered_map<std::string, Sound>& sounds, Sound& music, float& masterVolume) {
        Json::Document     doc;
        const Json::Value* list = Json::ParseFile(path, doc, "SoundSystem") ? Json::FindObject(doc, "sounds") : nullptr;
        if(!list)
            return false;
        Json::Read(doc, "masterVolume", masterVolume);
        if(const Json::Value* bgm = Json::FindObject(doc, "music")) {
            Json::Read(*bgm, "file", music.file);
            Json::Read(*bgm, "volume", music.volume);
        }

        for(auto it = list->MemberBegin(); it != list->MemberEnd(); ++it) {
            if(!it->value.IsObject())
                continue;
            Sound sound;
            if(!Json::Read(it->value, "file", sound.file))
                continue;
            Json::Read(it->value, "volume", sound.volume);
            Json::Read(it->value, "minInterval", sound.minInterval);
            sounds[it->name.GetString()] = sound;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! 出来事に合わせて効果音を鳴らします。
    //----------------------------------------------------------------------------
    void SoundSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!ctx || !ctx->assetManager || !ctx->audioManager) {
            m_pending.clear();
            return;
        }

        //--------------------------------------------------------------
        // 最初の Update で音を読み込み、全体の音量を決める
        //--------------------------------------------------------------
        if(!m_loaded) {
            int loaded = 0;
            for(auto& [name, sound] : m_sounds) {
                sound.handle = ctx->assetManager->Load(Tsukino::Core::Path(sound.file));
                if(sound.handle.IsValid())
                    ++loaded;
                else
                    Tsukino::Core::Log::Warn("SoundSystem: cannot load " + sound.file + ".");
            }
            Tsukino::Core::Log::Info("SoundSystem: loaded " + std::to_string(loaded) + " / " + std::to_string(m_sounds.size()) + " sounds.");

            // BGM（初回は mp3 からの変換があるので、かかった時間も出す）
            if(!m_music.file.empty()) {
                const auto start = std::chrono::steady_clock::now();
                m_music.handle   = ctx->assetManager->Load(Tsukino::Core::Path(m_music.file));
                const auto ms    = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                if(m_music.handle.IsValid())
                    Tsukino::Core::Log::Info("SoundSystem: loaded music " + m_music.file + " (" + std::to_string(ms) + " ms).");
                else
                    Tsukino::Core::Log::Warn("SoundSystem: cannot load music " + m_music.file + ".");
            }

            const bool muted = registry.HasContext<Settings>() && registry.GetContext<Settings>().muted;
            ctx->audioManager->SetMasterVolume(muted ? 0.0f : m_masterVolume);
            m_appliedMuted = muted;
            m_loaded       = true;
        }

        //--------------------------------------------------------------
        // BGM: 設定の音量で流す。音量が変わったら少し待ってから流し直す（−/＋を続けて押しても1回だけ）
        //--------------------------------------------------------------
        if(m_music.handle.IsValid()) {
            const float bgm    = registry.HasContext<Settings>() ? registry.GetContext<Settings>().bgmVolume : 1.0f;
            const float volume = m_music.volume * bgm;
            if(m_musicVolume < 0.0f) {
                PlayMusic(registry, volume);
            } else if(std::abs(volume - m_musicVolume) > 0.001f) {
                if(m_musicRestart <= 0.0f)
                    m_musicRestart = GetUiConfig(registry).musicRestartDelay;
                m_musicRestart -= deltaTime;
                if(m_musicRestart <= 0.0f) {
                    PlayMusic(registry, volume);
                    m_musicRestart = 0.0f;
                }
            }
        }

        for(auto& [name, sound] : m_sounds)
            sound.cooldown = std::max(0.0f, sound.cooldown - deltaTime);

        //--------------------------------------------------------------
        // M キーで消音を切り替える
        //--------------------------------------------------------------
        if(ctx->inputSystem && ctx->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::M) && registry.HasContext<Settings>()) {
            Settings& settings = registry.GetContext<Settings>();
            settings.muted     = !settings.muted;
            settings.Save();
            m_eventBus.Publish(NoticeEvent{GetTexts(registry).Get(settings.muted ? "notice.muted" : "notice.unmuted"), GetUiConfig(registry).muteNoticeSeconds});
        }

        // 消音（M キーとオプション画面のどちらで変えても）を全体の音量に反映する
        const bool muted = registry.HasContext<Settings>() && registry.GetContext<Settings>().muted;
        if(muted != m_appliedMuted) {
            ctx->audioManager->SetMasterVolume(muted ? 0.0f : m_masterVolume);
            m_appliedMuted = muted;
        }

        //--------------------------------------------------------------
        // 状態の変化で知る出来事
        //--------------------------------------------------------------
        if(registry.HasContext<PlayStats>()) {
            const int launched = registry.GetContext<PlayStats>().coinsLaunched;
            if(launched > m_lastLaunched)
                m_pending.push_back("coinLaunch");
            m_lastLaunched = launched;
        }
        if(registry.HasContext<GameState>()) {
            int levels = 0;
            for(const auto& [id, level] : registry.GetContext<GameState>().upgradeLevels)
                levels += level;
            if(m_lastUpgradeLevels >= 0 && levels > m_lastUpgradeLevels)
                m_pending.push_back("upgrade");
            m_lastUpgradeLevels = levels;
        }
        if(registry.HasContext<MenuState>()) {
            const MenuKind menu = registry.GetContext<MenuState>().open;
            if(menu != m_lastMenu)
                m_pending.push_back("button");
            m_lastMenu = menu;
        }
        if(registry.HasContext<RouletteState>()) {
            const RouletteState& roulette = registry.GetContext<RouletteState>();
            if(roulette.phase != m_lastPhase) {
                // 段階が変わった瞬間に結果の音
                if(roulette.phase == RoulettePhase::Result)
                    m_pending.push_back((roulette.resultHit || roulette.resultCoins > 0) ? "rouletteHit" : "rouletteMiss");
                else if(roulette.phase == RoulettePhase::JackpotSpin)
                    m_pending.push_back("jackpotChance");
                else if(roulette.phase == RoulettePhase::JackpotResult)
                    m_pending.push_back(roulette.jackpotWin ? "jackpotWin" : "rouletteMiss");
            }

            //--------------------------------------------------------------
            // スロットが回っている間: 一定の間隔でチッ、列が止まるたびにカチッ、2列そろったらリーチの音
            //--------------------------------------------------------------
            const bool spinning = roulette.phase == RoulettePhase::Spinning || roulette.phase == RoulettePhase::JackpotSpin;
            if(roulette.spinId != m_lastSpinId) {
                m_lastSpinId       = roulette.spinId;
                m_lastReelsStopped = 0;
                m_lastTick         = -1;
            }
            if(spinning) {
                if(roulette.reelsStopped > m_lastReelsStopped) {
                    m_pending.push_back("reelStop");
                    if(roulette.reach && roulette.reelsStopped == kReelCount - 1)
                        m_pending.push_back("reach");
                } else if(roulette.reelsStopped < kReelCount) {
                    constexpr float kTickInterval = 0.09f;    // 絵柄が1つ流れるくらいの間隔
                    const int       tick          = static_cast<int>(roulette.spinTime / kTickInterval);
                    if(tick != m_lastTick)
                        m_pending.push_back("rouletteTick");
                    m_lastTick = tick;
                }
                m_lastReelsStopped = roulette.reelsStopped;
            }
            m_lastPhase = roulette.phase;
        }

        for(const std::string& name : m_pending)
            Play(registry, name);
        m_pending.clear();
    }

    //----------------------------------------------------------------------------
    //! 音を鳴らします。
    //----------------------------------------------------------------------------
    void SoundSystem::Play(Tsukino::ECS::Registry& registry, const std::string& name) {
        auto it = m_sounds.find(name);
        if(it == m_sounds.end() || it->second.cooldown > 0.0f || !it->second.handle.IsValid())
            return;

        Tsukino::EngineIntegration::EngineContext* ctx   = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        auto                                       asset = std::dynamic_pointer_cast<Tsukino::Asset::AudioAsset>(ctx->assetManager->Get(it->second.handle));
        if(!asset)
            return;

        // 効果音の音量の設定を掛ける（0 なら鳴らさない）
        const float se = registry.HasContext<Settings>() ? registry.GetContext<Settings>().seVolume : 1.0f;
        if(se <= 0.0f)
            return;
        ctx->audioManager->Play(*asset, false, it->second.volume * se);
        it->second.cooldown = it->second.minInterval;
    }

    //----------------------------------------------------------------------------
    //! BGM を設定の音量で流します。
    //----------------------------------------------------------------------------
    void SoundSystem::PlayMusic(Tsukino::ECS::Registry& registry, float volume) {
        Tsukino::EngineIntegration::EngineContext* ctx   = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        auto                                       asset = std::dynamic_pointer_cast<Tsukino::Asset::AudioAsset>(ctx->assetManager->Get(m_music.handle));
        m_musicVolume                                    = volume;
        if(!asset)
            return;

        // 再生中の音量は変えられないので、止めてから新しい音量で流し直す
        ctx->audioManager->Stop(*asset);
        if(volume > 0.0f)
            ctx->audioManager->Play(*asset, true, volume);
    }
}    // namespace FruitMagic::ECS

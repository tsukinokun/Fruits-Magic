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
        if(!Parse(path, m_sounds, m_masterVolume))
            Tsukino::Core::Log::Warn("SoundSystem: cannot read \"sounds\" from " + path + ". No sound will play.");
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルに書かれた効果音のファイルを列挙します。
    //----------------------------------------------------------------------------
    std::vector<std::string> SoundSystem::ListSoundFiles(const std::string& path) {
        std::unordered_map<std::string, Sound> sounds;
        float                                  masterVolume = 0.0f;
        Parse(path, sounds, masterVolume);

        std::vector<std::string> files;
        for(const auto& [name, sound] : sounds) {
            if(std::find(files.begin(), files.end(), sound.file) == files.end())
                files.push_back(sound.file);
        }
        return files;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み、効果音の設定と全体の音量を取り出します。
    //----------------------------------------------------------------------------
    bool SoundSystem::Parse(const std::string& path, std::unordered_map<std::string, Sound>& sounds, float& masterVolume) {
        Json::Document     doc;
        const Json::Value* list = Json::ParseFile(path, doc, "SoundSystem") ? Json::FindObject(doc, "sounds") : nullptr;
        if(!list)
            return false;
        Json::Read(doc, "masterVolume", masterVolume);

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
            ctx->audioManager->SetMasterVolume(m_masterVolume);
            Tsukino::Core::Log::Info("SoundSystem: loaded " + std::to_string(loaded) + " / " + std::to_string(m_sounds.size()) + " sounds.");
            m_loaded = true;
        }

        for(auto& [name, sound] : m_sounds)
            sound.cooldown = std::max(0.0f, sound.cooldown - deltaTime);

        //--------------------------------------------------------------
        // M キーで消音を切り替える
        //--------------------------------------------------------------
        if(ctx->inputSystem && ctx->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::M)) {
            m_muted = !m_muted;
            ctx->audioManager->SetMasterVolume(m_muted ? 0.0f : m_masterVolume);
            m_eventBus.Publish(NoticeEvent{GetTexts(registry).Get(m_muted ? "notice.muted" : "notice.unmuted"), GetUiConfig(registry).muteNoticeSeconds});
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
            } else if(roulette.phase == RoulettePhase::Spinning && roulette.displayFruit != m_lastDisplayFruit) {
                // 回転中は表示が切り替わるたびにチッ
                m_pending.push_back("rouletteTick");
            } else if(roulette.phase == RoulettePhase::JackpotSpin && roulette.jackpotDisplay != m_lastJackpotDisplay) {
                m_pending.push_back("rouletteTick");
            }
            m_lastPhase          = roulette.phase;
            m_lastDisplayFruit   = roulette.displayFruit;
            m_lastJackpotDisplay = roulette.jackpotDisplay;
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

        ctx->audioManager->Play(*asset, false, it->second.volume);
        it->second.cooldown = it->second.minInterval;
    }
}    // namespace FruitMagic::ECS

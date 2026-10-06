//----------------------------------------------------------------------------
//! @file   PusherScene.cpp
//! @brief  コインプッシャー台のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/PusherScene.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/EffectsConfig.hpp>
#include <FruitMagic/Game/EconomyConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/JackpotConfig.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicEffects.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/ManaConfig.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/OfflineReward.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/ReliefState.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>
#include <FruitMagic/Game/SaveData.hpp>
#include <FruitMagic/Game/OptionsState.hpp>
#include <FruitMagic/Game/SceneRequest.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/StageConfig.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UiFonts.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>
#include <FruitMagic/ECS/Component/CheckerComponent.hpp>
#include <FruitMagic/ECS/Component/CoinLauncherComponent.hpp>
#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Component/ManaGaugeComponent.hpp>
#include <FruitMagic/ECS/Component/MenuComponent.hpp>
#include <FruitMagic/ECS/Component/OptionsDialogComponent.hpp>
#include <FruitMagic/ECS/Component/OptionsElementComponent.hpp>
#include <FruitMagic/ECS/Component/ReliefGaugeComponent.hpp>
#include <FruitMagic/ECS/Component/UpgradeElementComponent.hpp>
#include <FruitMagic/ECS/Component/ZukanElementComponent.hpp>
#include <FruitMagic/ECS/System/AutoPlaySystem.hpp>
#include <FruitMagic/ECS/System/BalanceProbeSystem.hpp>
#include <FruitMagic/ECS/System/CheckerSystem.hpp>
#include <FruitMagic/ECS/System/CoinShowerSystem.hpp>
#include <FruitMagic/ECS/System/CoinLauncherSystem.hpp>
#include <FruitMagic/ECS/System/FairySystem.hpp>
#include <FruitMagic/ECS/System/HarvestSystem.hpp>
#include <FruitMagic/ECS/System/EffectsSystem.hpp>
#include <FruitMagic/ECS/System/GrowMagicSystem.hpp>
#include <FruitMagic/ECS/System/HudSystem.hpp>
#include <FruitMagic/ECS/System/MagicInputSystem.hpp>
#include <FruitMagic/ECS/System/ManaSystem.hpp>
#include <FruitMagic/ECS/System/MenuSystem.hpp>
#include <FruitMagic/ECS/System/OptionsSystem.hpp>
#include <FruitMagic/ECS/System/MeteorMagicSystem.hpp>
#include <FruitMagic/ECS/System/PopupSystem.hpp>
#include <FruitMagic/ECS/System/ShakeMagicSystem.hpp>
#include <FruitMagic/ECS/System/SoundSystem.hpp>
#include <FruitMagic/ECS/System/SwellMagicSystem.hpp>
#include <FruitMagic/ECS/System/WallMagicSystem.hpp>
#include <FruitMagic/ECS/System/PrizeDropSystem.hpp>
#include <FruitMagic/ECS/System/RouletteSystem.hpp>
#include <FruitMagic/ECS/System/SaveSystem.hpp>
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>
#include <FruitMagic/ECS/System/WalletSystem.hpp>
#include <FruitMagic/ECS/System/ZukanSystem.hpp>
#ifdef _DEBUG
#include <FruitMagic/ECS/System/DebugResourceSystem.hpp>
#endif

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/Audio/AudioManager.hpp>
#include <Tsukino/Core/Window.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/PhysicsSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/LightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/InteractionSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ScrollViewSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/AmbientParticleSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/WorldAnchorSystem.hpp>
#ifdef _DEBUG
#include <Tsukino/EngineIntegration/ECS/System/DebugCameraSystem.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraTag.hpp>
#endif

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScrollBarComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScrollViewComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DirectionalLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SkyAtmosphereComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/AmbientParticleComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UIClipComponent.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Core/Log.hpp>

#include <entt/entt.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //--------------------------------------------------------------
        //! バランス計測用の自動プレイで起動したかを返します（環境変数 FRUITMAGIC_AUTOPLAY=1）。
        //! @return 自動プレイなら true
        //--------------------------------------------------------------
        bool IsAutoPlay() {
            char*       value  = nullptr;
            size_t      length = 0;
            const bool  found  = _dupenv_s(&value, &length, "FRUITMAGIC_AUTOPLAY") == 0 && value != nullptr;
            const bool  on     = found && std::string(value) == "1";
            std::free(value);
            return on;
        }

        //--------------------------------------------------------------
        //! UI の揃え方をエンジンの揃え方にします。
        //! @param  [in] align UI の揃え方
        //! @return エンジンの揃え方
        //--------------------------------------------------------------
        Tsukino::BuiltIn::ECS::HorizontalAlign ToEngineAlign(UiAlign align) {
            switch(align) {
                case UiAlign::Center: return Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
                case UiAlign::Right: return Tsukino::BuiltIn::ECS::HorizontalAlign::Right;
                case UiAlign::Left:
                default: return Tsukino::BuiltIn::ECS::HorizontalAlign::Left;
            }
        }

        //--------------------------------------------------------------
        //! 輪郭の光り方を付けます。
        //! @param  [in] registry レジストリ
        //! @param  [in] entity   付けるエンティティ
        //! @param  [in] style    光り方
        //--------------------------------------------------------------
        void AddGlow(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, const RimGlowStyle& style) {
            Tsukino::BuiltIn::ECS::RimGlowComponent& glow = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(entity);
            glow.active                                   = true;
            glow.rimColor                                 = style.color;
            glow.rimIntensity                             = style.intensity;
            glow.glow                                     = style.glow;
        }

        //--------------------------------------------------------------
        // HUD の文字の種類と、Ui.json の "hud" のキーの対応
        //--------------------------------------------------------------
        struct HudKindName {
            ECS::HudTextKind kind;
            const char*      name;
        };
        constexpr HudKindName kHudKinds[] = {
            {ECS::HudTextKind::Coins, "coins"},
            {ECS::HudTextKind::Mana, "mana"},
            {ECS::HudTextKind::DropPopup, "dropPopup"},
            {ECS::HudTextKind::HarvestTotal, "harvestTotal"},
            {ECS::HudTextKind::HarvestPopup, "harvestPopup"},
            {ECS::HudTextKind::Relief, "relief"},
            {ECS::HudTextKind::Roulette, "roulette"},
            {ECS::HudTextKind::ControlsHint, "controlsHint"},
            {ECS::HudTextKind::Notice, "notice"},
        };
    }    // namespace

    //----------------------------------------------------------------------------
    //! シーン固有の初期化処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) {
        Tsukino::EngineIntegration::EngineContext* context  = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();
        Tsukino::ECS::EventBus&                    eventBus = m_scene.GetEventBus();
        Tsukino::ECS::Registry&                    registry = m_scene.GetRegistry();

        // 定義データ（Debug は作業ディレクトリ、Release は exe の隣が基準）
        const std::string dataRoot = (Tsukino::IO::FileSystem::GetAssetRootPath() / "Assets/Data").string();

        // 台の寸法と物理の設定（物理のシステムの設定に使うので、システムより先に読む）
        TableLayout& layout = registry.SetContext<TableLayout>();
        layout.Load(dataRoot + "/Table.json");

        //--------------------------------------------------------------
        // システムの生成と追加
        //--------------------------------------------------------------
        enum class SystemPriority : int {
#ifdef _DEBUG
            DebugResource = -1,
#endif
            Interaction = 0,     // マウスの下の UI（魔法ボタン・画面）を先に決め、同じフレームの入力処理が読めるようにする
            Menu,                // 画面の開閉と表示切替。中身は続く Zukan・Upgrade が書く
            ScrollView,          // 開いている画面の行のスクロール（Menu が決めた開閉の後、Transform の前）
            Zukan,
            Upgrade,
            Options,
            MagicInput,
            AutoPlay,            // （計測用）投入・魔法・強化
            CoinLauncher,        // 投入したコインを今フレームの Transform・物理に乗せる
            Fairy,
            CoinShower,
            Transform,
            Physics,             // Transform 確定後に剛体を進め、結果を Transform へ書き戻す
            PrizeDrop,           // 物理の結果で落下を判定する
            Checker,             // 払い出し口に落ちたコインがチェッカーに入ったかを、同じフレームの落下イベントで判定する
            Wallet,
            Harvest,
            Mana,
            Roulette,            // 当たりで果物を生成する（次のフレームの Transform・物理に乗る）
            ShakeMagic,          // 衝撃の要求を付ける（次のフレームの Physics で反映）。カメラも揺らす
            SwellMagic,          // 振幅のボーナスを書く（次のフレームの OnUpdate で反映）
            WallMagic,           // 壁を作って動かす（次のフレームの Physics で反映）
            GrowMagic,           // 果物を作り直す（次のフレームの Physics に乗る）
            MeteorMagic,         // シャワーを依頼する（次のフレームの CoinShower で降る）
            Effects,             // 光の粒・画面の光・ぽよん（このフレームの出来事の分を出す）
            Popup,               // 落ちたときのポップ
            Sound,               // このフレームの出来事の効果音
            Hud,
            BalanceProbe,
            Save,
            Light,
            SkyAtmosphere,
            AmbientParticle,
#ifdef _DEBUG
            DebugCamera,
#endif
            Camera,
            WorldAnchor,         // カメラが決まった後に、ワールドの一点に追従する文字の画面位置を決める
            Font,
            Sprite,
            Render,
        };

#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<ECS::DebugResourceSystem>(), (int)SystemPriority::DebugResource);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::InteractionSystem>(), (int)SystemPriority::Interaction);
        m_scene.AddSystem(std::make_shared<ECS::MenuSystem>(), (int)SystemPriority::Menu);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ScrollViewSystem>(), (int)SystemPriority::ScrollView);
        m_scene.AddSystem(std::make_shared<ECS::ZukanSystem>(), (int)SystemPriority::Zukan);
        m_scene.AddSystem(std::make_shared<ECS::UpgradeSystem>(), (int)SystemPriority::Upgrade);
        m_scene.AddSystem(std::make_shared<ECS::OptionsSystem>(), (int)SystemPriority::Options);
        m_scene.AddSystem(std::make_shared<ECS::MagicInputSystem>(eventBus), (int)SystemPriority::MagicInput);
        m_scene.AddSystem(std::make_shared<ECS::CoinLauncherSystem>(), (int)SystemPriority::CoinLauncher);
        m_scene.AddSystem(std::make_shared<ECS::FairySystem>(), (int)SystemPriority::Fairy);
        m_scene.AddSystem(std::make_shared<ECS::CoinShowerSystem>(eventBus), (int)SystemPriority::CoinShower);
        m_scene.AddSystem(std::make_shared<ECS::CheckerSystem>(eventBus), (int)SystemPriority::Checker);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), (int)SystemPriority::Transform);
        auto physicsSystem = std::make_shared<Tsukino::BuiltIn::ECS::PhysicsSystem>(eventBus);
        // このゲームは 1unit=1cm。物理エンジンの重力・接触の許容値をcmに合わせる
        physicsSystem->SetUnitsPerMeter(100.0f);
        // 上の換算でめり込みの許容値なども 2cm 相当になり、厚み 0.8cm のコインに果物が沈むので、cm 向けに小さくする
        physicsSystem->SetContactTolerances(layout.penetrationSlop, layout.speculativeContactDistance);
#ifdef _DEBUG
        // コリジョンのワイヤーフレームを起動時から表示する（F5 で切り替え）
        physicsSystem->SetDebugDrawEnabled(true);
#endif
        m_scene.AddSystem(physicsSystem, (int)SystemPriority::Physics);
        m_scene.AddSystem(std::make_shared<ECS::PrizeDropSystem>(eventBus), (int)SystemPriority::PrizeDrop);
        m_scene.AddSystem(std::make_shared<ECS::WalletSystem>(eventBus), (int)SystemPriority::Wallet);
        m_scene.AddSystem(std::make_shared<ECS::HarvestSystem>(eventBus), (int)SystemPriority::Harvest);
        m_scene.AddSystem(std::make_shared<ECS::ManaSystem>(eventBus), (int)SystemPriority::Mana);
        m_scene.AddSystem(std::make_shared<ECS::RouletteSystem>(eventBus), (int)SystemPriority::Roulette);
        m_scene.AddSystem(std::make_shared<ECS::ShakeMagicSystem>(eventBus), (int)SystemPriority::ShakeMagic);
        m_scene.AddSystem(std::make_shared<ECS::SwellMagicSystem>(eventBus), (int)SystemPriority::SwellMagic);
        m_scene.AddSystem(std::make_shared<ECS::WallMagicSystem>(eventBus), (int)SystemPriority::WallMagic);
        m_scene.AddSystem(std::make_shared<ECS::GrowMagicSystem>(eventBus), (int)SystemPriority::GrowMagic);
        m_scene.AddSystem(std::make_shared<ECS::MeteorMagicSystem>(eventBus), (int)SystemPriority::MeteorMagic);
        m_scene.AddSystem(std::make_shared<ECS::EffectsSystem>(eventBus), (int)SystemPriority::Effects);
        m_scene.AddSystem(std::make_shared<ECS::PopupSystem>(eventBus), (int)SystemPriority::Popup);
        m_scene.AddSystem(std::make_shared<ECS::SoundSystem>(eventBus, (Tsukino::IO::FileSystem::GetAssetRootPath() / "Assets/Data/Sounds.json").string()),
                          (int)SystemPriority::Sound);
        m_scene.AddSystem(std::make_shared<ECS::HudSystem>(eventBus), (int)SystemPriority::Hud);
        m_scene.AddSystem(std::make_shared<ECS::SaveSystem>(), (int)SystemPriority::Save);

        // バランス計測用の自動プレイ（セーブは読み書きしない）
        const bool autoPlay = IsAutoPlay();
        if(autoPlay) {
            m_scene.AddSystem(std::make_shared<ECS::AutoPlaySystem>(eventBus), (int)SystemPriority::AutoPlay);
            m_scene.AddSystem(std::make_shared<ECS::BalanceProbeSystem>(eventBus), (int)SystemPriority::BalanceProbe);
            Tsukino::Core::Log::Info("PusherScene: auto play for balance measurement. The save file is not used.");
        }
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::LightSystem>(), (int)SystemPriority::Light);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SkyAtmosphereSystem>(), (int)SystemPriority::SkyAtmosphere);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::AmbientParticleSystem>(), (int)SystemPriority::AmbientParticle);
#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DebugCameraSystem>(), (int)SystemPriority::DebugCamera);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), (int)SystemPriority::Camera);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::WorldAnchorSystem>(), (int)SystemPriority::WorldAnchor);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), (int)SystemPriority::Font);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), (int)SystemPriority::Sprite);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ModelSystem>(), (int)SystemPriority::Render);

        //--------------------------------------------------------------
        // ゲーム全体で共有するデータをレジストリのコンテキストへ置く
        //--------------------------------------------------------------
        // 見た目・UI・文言（台やほかの設定を作る前に読む）
        StageConfig& stage = registry.SetContext<StageConfig>();
        stage.Load(dataRoot + "/Stage.json");
        registry.SetContext<UiConfig>().Load(dataRoot + "/Ui.json");
        registry.SetContext<Texts>().Load(dataRoot + "/Texts.json");
        registry.SetContext<UiFonts>().Load(*context->assetManager, GetUiConfig(registry));    // ロード画面で読んであるので、ここではキャッシュから取るだけ

        FruitCatalog& catalog = registry.SetContext<FruitCatalog>();
        if(!catalog.Load(dataRoot)) {
            Tsukino::Core::Log::Error("PusherScene: no fruit data could be loaded from " + dataRoot + ". Fruits will not appear.");
        }
        registry.SetContext<RouletteConfig>().Load(dataRoot + "/Roulette.json");
        registry.SetContext<RouletteState>();
        ManaConfig& manaConfig = registry.SetContext<ManaConfig>();
        manaConfig.Load(dataRoot + "/Mana.json");
        MagicCatalog& magics = registry.SetContext<MagicCatalog>();
        magics.Load(dataRoot + "/Magic.json");
        registry.SetContext<MagicState>().remaining.assign(magics.Magics().size(), 0.0f);
        registry.SetContext<MagicEffects>();
        registry.SetContext<CoinShowerState>();
        registry.SetContext<JackpotConfig>().Load(dataRoot + "/Jackpot.json");
        registry.SetContext<PlayStats>();
        registry.SetContext<ReliefState>();
        registry.SetContext<EffectsConfig>().Load(dataRoot + "/Effects.json");
        CollectionConfig& collection = registry.SetContext<CollectionConfig>();
        collection.Load(dataRoot + "/Collection.json");
        MenuState& menu = registry.SetContext<MenuState>();
        registry.SetContext<UpgradeCatalog>().Load(dataRoot + "/Upgrades.json");
        registry.SetContext<TableStats>().pusherAmplitude = layout.pusherAmplitude;
        OfflineConfig& offline = registry.SetContext<OfflineConfig>();
        offline.Load(dataRoot + "/Offline.json");
        offline.savePath = autoPlay ? std::string() : SaveData::DefaultPath();

        // オプションの設定（セーブデータとは別のファイル。自動プレイでは読み書きしない）
        registry.SetContext<Settings>().Load(autoPlay ? std::string() : Settings::DefaultPath());
        registry.SetContext<SceneRequest>();
        registry.SetContext<OptionsState>();

        EconomyConfig& economy = registry.SetContext<EconomyConfig>();
        economy.Load(dataRoot + "/Economy.json");

        GameState& state = registry.SetContext<GameState>();
        state.coins      = economy.startCoins;    // セーブがあれば下で上書きされる
        state.maxMana    = manaConfig.maxMana;
        state.harvestCounts.assign(catalog.Fruits().size(), std::vector<int>(collection.Variants().size(), 0));

        //--------------------------------------------------------------
        // セーブを読み、強化を台の性能と果樹の段階へ反映してから、閉じていた間の報酬を受け取る
        // （報酬の計算に強化の値を使うため、この順番）
        //--------------------------------------------------------------
        long long  savedAt = 0;
        const bool loaded  = !offline.savePath.empty() && SaveData::Load(registry, offline.savePath, savedAt);
        ECS::UpgradeSystem::ApplyUpgrades(registry);
        m_pusherAmplitude = registry.GetContext<TableStats>().pusherAmplitude;

        OfflineReport& report = registry.SetContext<OfflineReport>();
        if(loaded) {
            std::mt19937 rng(std::random_device{}());
            report = GrantOfflineReward(registry, SaveData::NowSeconds() - savedAt, rng);
            if(report.awaySeconds >= offline.minSeconds)
                menu.open = MenuKind::Welcome;
        }

        PrizeFactory& factory = registry.SetContext<PrizeFactory>();
        factory.Initialize(*context->assetManager, layout, stage);

        CreateCabinet(factory);
        CreateInitialPrizes(factory);
        CreateEnvironment();
        CreatePlayerInterface(factory);

        Tsukino::Core::Log::Info("PusherScene: initialized.");
    }

    //----------------------------------------------------------------------------
    //! シーンを更新します。
    //----------------------------------------------------------------------------
    void PusherScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const TableLayout&      layout   = GetTableLayout(registry);

        // PhysicsSystem は1フレーム1ステップなので、起動直後のアセット読み込み等で
        // フレームが重くなったときに大きく進めないよう上限を設ける（その分ゲーム内時間は遅れる）。
        // プッシャーの移動にも同じ時間を使い、物理の1ステップで動く距離と食い違わないようにする
        const float simulationStep = std::min(deltaTime, layout.maxSimulationStep);

        //--------------------------------------------------------------
        // プッシャーを前後に往復させる
        // m_scene.Update() より前に書き込むことで、同じフレームの PhysicsSystem が
        // 位置の差分から速度を求め、乗っている景品を押す
        //--------------------------------------------------------------
        if(m_pusherEntity != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity)) {
            m_pusherTime = std::fmod(m_pusherTime + simulationStep, layout.pusherPeriod);

            // 押し幅の強化と魔法「ふくらむ」は、振幅を少しずつ近づけて反映する（一気に変えると速度が跳ねて景品を弾き飛ばす）
            if(registry.HasContext<TableStats>()) {
                const float bonus  = registry.HasContext<MagicEffects>() ? registry.GetContext<MagicEffects>().pusherAmplitudeBonus : 0.0f;
                const float target = std::min(registry.GetContext<TableStats>().pusherAmplitude + bonus, layout.pusherMaxAmplitude);
                const float step   = layout.pusherAmplitudeSpeed * simulationStep;
                m_pusherAmplitude  = std::clamp(target, m_pusherAmplitude - step, m_pusherAmplitude + step);
            }

            // 最も引っ込んだ位置は振幅によらず同じで、振幅が増えた分だけ前に出る
            const float phase = 2.0f * kPi * m_pusherTime / layout.pusherPeriod;

            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity);
            t.position = hlslpp::float3(0.0f, layout.PusherCenterY(), layout.PusherCenterZ(m_pusherAmplitude) + std::sin(phase) * m_pusherAmplitude);
            t.dirty    = true;
        }

        m_scene.Update(simulationStep);

        //--------------------------------------------------------------
        // オプション画面からの頼みごと（システムの更新が終わってから行う）
        //--------------------------------------------------------------
        if(registry.HasContext<SceneRequest>()) {
            SceneRequest& request = registry.GetContext<SceneRequest>();
            if(request.restart) {
                // セーブを消し、このシーンの終了時に保存し直さないようにしてから、新しい台で始め直す
                request.restart = false;
                if(registry.HasContext<OfflineConfig>() && !registry.GetContext<OfflineConfig>().savePath.empty()) {
                    std::error_code error;
                    std::filesystem::remove(registry.GetContext<OfflineConfig>().savePath, error);
                }
                m_skipSaveOnExit = true;
                Tsukino::Core::Log::Info("PusherScene: restarting with a new game.");
                api.ChangeScene(std::make_unique<PusherScene>());
            } else if(request.quit) {
                // 普通にウィンドウを閉じたのと同じ流れで終わる（終了時に OnExit でセーブされる）
                request.quit = false;
                Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
                if(ctx && ctx->window)
                    PostMessageW(ctx->window->GetHWND(), WM_CLOSE, 0, 0);
            }
        }
    }

    //----------------------------------------------------------------------------
    //! シーンの終了処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnExit() {
        Tsukino::Core::Log::Info("PusherScene: exiting.");
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();

        // 鳴っている音（BGM のループ）を止める。最初からやり直すとき、次のシーンの BGM と重ならないように
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(ctx && ctx->audioManager)
            ctx->audioManager->StopAll();

        if(m_skipSaveOnExit)
            return;
        if(!registry.HasContext<OfflineConfig>() || registry.GetContext<OfflineConfig>().savePath.empty())
            return;
        if(SaveData::Save(registry, registry.GetContext<OfflineConfig>().savePath))
            Tsukino::Core::Log::Info("PusherScene: saved.");
    }

    //----------------------------------------------------------------------------
    //! 筐体（床・壁・背面パネル・景品受け）とプッシャーを生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateCabinet(PrizeFactory& factory) {
        using Tsukino::BuiltIn::ECS::RigidbodyType;

        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const TableLayout&      layout   = GetTableLayout(registry);
        const StageConfig&      stage    = GetStageConfig(registry);
        const float             thick    = layout.staticThickness;

        // プレイフィールドの床（上面が y = 0）。
        // 物理は離散ステップなので、フレームが重いと薄い床は景品がすり抜ける。
        // 見た目より厚めにして、1ステップで突き抜けないようにする
        const float fieldHalfDepth = (layout.fieldFrontZ - layout.fieldBackZ) * 0.5f;
        const float fieldCenterZ   = layout.fieldBackZ + fieldHalfDepth;
        factory.CreateBox(registry, hlslpp::float3(0.0f, -thick, fieldCenterZ), hlslpp::float3(layout.fieldHalfWidth, thick, fieldHalfDepth),
                          RigidbodyType::Static, stage.floorColor);

        // 左右の側壁。プッシャーの横（奥側）だけを囲い、手前側は開けておく（端に寄った景品が横の溝へこぼれる）
        const float wallHalfDepth = (layout.SideWallFrontZ() - layout.fieldBackZ) * 0.5f;
        const float wallCenterZ   = layout.fieldBackZ + wallHalfDepth;
        for(float side : {-1.0f, 1.0f}) {
            factory.CreateBox(registry,
                              hlslpp::float3(side * (layout.fieldHalfWidth + layout.sideWallHalfThickness), layout.sideWallHeight - thick, wallCenterZ),
                              hlslpp::float3(layout.sideWallHalfThickness, layout.sideWallHeight + thick, wallHalfDepth), RigidbodyType::Static,
                              stage.sideWallColor);
        }

        // 背面パネル。プッシャー上面のすぐ上に置き、プッシャーが引っ込むときに
        // 上面に乗った景品を手前へ掻き落とす（実機のプッシャーと同じ仕組み）。
        // 隙間は作らず、パネルの下端をプッシャーの中へ少し食い込ませる（静的な物とKinematicは衝突しない）。
        // 隙間があると、引っ込むプッシャーに挟まれたコインが押し込まれて下をくぐり、奥へ運ばれてしまう
        factory.CreateBox(registry, hlslpp::float3(0.0f, layout.PusherTopY() - layout.backPanelSink + layout.backPanelHalfHeight, layout.BackPanelZ()),
                          hlslpp::float3(layout.fieldHalfWidth, layout.backPanelHalfHeight, layout.backPanelHalfThickness), RigidbodyType::Static,
                          stage.backPanelColor);

        //--------------------------------------------------------------
        // 景品受け。手前の端から落ちた物は幅のどこでも取得なので、手前のトレイは台の幅いっぱいにする。
        // 側壁の無い所の外側には一段低い横の溝を置く（判定は PrizeDropSystem が落ちた位置で行うので、ここは見た目と受け止め用）
        //--------------------------------------------------------------
        const float trayCenterZ  = layout.fieldFrontZ + layout.trayOffsetZ;
        const float outerHalfX   = layout.fieldHalfWidth + layout.gutterExtraWidth;
        factory.CreateBox(registry, hlslpp::float3(0.0f, layout.trayTopY - thick, trayCenterZ), hlslpp::float3(outerHalfX, thick, layout.trayHalfDepth),
                          RigidbodyType::Static, stage.trayColor);

        const float sideGutterBackZ  = layout.SideWallFrontZ();
        const float sideGutterFrontZ = trayCenterZ - layout.trayHalfDepth;
        if(sideGutterFrontZ > sideGutterBackZ) {
            for(float side : {-1.0f, 1.0f}) {
                factory.CreateBox(registry,
                                  hlslpp::float3(side * (layout.fieldHalfWidth + layout.gutterExtraWidth * 0.5f), layout.trayTopY - layout.gutterDepth - thick,
                                                 (sideGutterBackZ + sideGutterFrontZ) * 0.5f),
                                  hlslpp::float3(layout.gutterExtraWidth * 0.5f, thick, (sideGutterFrontZ - sideGutterBackZ) * 0.5f), RigidbodyType::Static,
                                  stage.gutterColor);
            }
        }

        //--------------------------------------------------------------
        // プッシャー（Kinematic。OnUpdate で位置を直接動かす）。起動時は強化前の振幅の位置に置く
        //--------------------------------------------------------------
        m_pusherEntity = factory.CreateBox(registry, hlslpp::float3(0.0f, layout.PusherCenterY(), layout.PusherCenterZ(layout.pusherAmplitude)),
                                           hlslpp::float3(layout.PusherHalfWidth(), layout.pusherHalfHeight, layout.pusherHalfDepth),
                                           RigidbodyType::Kinematic, stage.pusherColor);
        registry.AddComponent<ECS::PusherComponent>(m_pusherEntity);    // 魔法「ふくらむ」の間、輪郭を光らせる目印

        CreateStall(factory);
    }

    //----------------------------------------------------------------------------
    //! 屋台の飾り（しましまの屋根・柱・ちょうちん）を生成します。見た目だけで、当たり判定は持ちません。
    //----------------------------------------------------------------------------
    void PusherScene::CreateStall(PrizeFactory& factory) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const TableLayout&      layout   = GetTableLayout(registry);
        const StageConfig&      stage    = GetStageConfig(registry);

        //--------------------------------------------------------------
        // しましまの屋根。背面パネルの上から手前へ少し下がるように傾ける。
        // 高い位置にあるので、プレイヤーの目線から台の上は隠れない
        //--------------------------------------------------------------
        const int   stripes     = stage.awningStripes;
        const float tilt        = stage.awningTiltDegrees * kPi / 180.0f;
        const float awningHalfW = layout.fieldHalfWidth + stage.awningOverhang;
        const float stripeHalfW = awningHalfW / static_cast<float>(stripes);
        for(int i = 0; i < stripes; ++i) {
            const float          x = -awningHalfW + stripeHalfW * (2.0f * static_cast<float>(i) + 1.0f);
            Tsukino::ECS::Entity e = factory.CreateVisualBox(registry, hlslpp::float3(x, stage.awningY, stage.awningZ),
                                                             hlslpp::float3(stripeHalfW, stage.awningHalfThickness, stage.awningHalfDepth), 1.0f,
                                                             (i % 2 == 0) ? stage.awningRedColor : stage.awningWhiteColor);
            auto&                t = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.rotation             = PrizeFactory::EulerDegrees(hlslpp::float3(stage.awningTiltDegrees, 0.0f, 0.0f));
        }

        // 屋根の手前の縁の飾り（赤白の玉を並べる）
        const float frontEdgeZ = stage.awningZ + stage.awningHalfDepth * std::cos(tilt);
        const float frontEdgeY = stage.awningY - stage.awningHalfDepth * std::sin(tilt) - stage.awningBallDrop;
        const float ballRadius = stage.awningBallRadius;
        for(int i = 0; i <= stripes * 2; ++i) {
            const float x = -awningHalfW + awningHalfW * static_cast<float>(i) / static_cast<float>(stripes);
            factory.CreateVisualBall(registry, hlslpp::float3(x, frontEdgeY, frontEdgeZ), hlslpp::float3(ballRadius, ballRadius, ballRadius),
                                     (i % 2 == 0) ? stage.awningRedColor : stage.awningWhiteColor);
        }

        //--------------------------------------------------------------
        // 屋根を支える柱（台の左右の外側）
        //--------------------------------------------------------------
        for(float side : {-1.0f, 1.0f}) {
            factory.CreateVisualBox(registry, hlslpp::float3(side * (awningHalfW - stage.postInset), stage.awningY * 0.5f, frontEdgeZ - stage.postBack),
                                    hlslpp::float3(stage.postHalfSize, stage.awningY * 0.5f, stage.postHalfSize), 1.0f, stage.postColor);
        }

        //--------------------------------------------------------------
        // 電飾など（Stage.json の props。見た目だけで当たり判定は無い）
        //--------------------------------------------------------------
        for(const StageProp& prop : stage.props) {
            Tsukino::ECS::Entity e = factory.CreateVisualModel(registry, prop.model, prop.position, prop.rotation, prop.scale);
            if(prop.glow.intensity > 0.0f || prop.glow.glow > 0.0f)
                AddGlow(registry, e, prop.glow);
        }

        //--------------------------------------------------------------
        // 屋根の下に下がるちょうちん（モデルと、温かい色の点光源）
        //--------------------------------------------------------------
        if(stage.lanternModel.empty())
            return;
        for(const hlslpp::float3& position : stage.lanternPositions) {
            Tsukino::ECS::Entity e = factory.CreateVisualModel(registry, stage.lanternModel, position, stage.lanternRotation, stage.lanternScale);
            AddGlow(registry, e, stage.lanternGlow);

            // 点光源は灯りの位置に置く（モデルの拡大の影響を受けないよう、別のエンティティにする）
            Tsukino::ECS::Entity                       light     = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(light);
            transform.position                                   = position + stage.lanternLightOffset;
            transform.dirty                                      = true;

            Tsukino::BuiltIn::ECS::PointLightComponent& point = registry.AddComponent<Tsukino::BuiltIn::ECS::PointLightComponent>(light);
            point.color                                      = stage.lanternLightColor;
            point.intensity                                  = stage.lanternLightIntensity;
            point.range                                      = stage.lanternLightRange;
            point.enabled                                    = true;
        }
    }

    //----------------------------------------------------------------------------
    //! 起動時に台に置いておく景品を生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateInitialPrizes(PrizeFactory& factory) {
        Tsukino::ECS::Registry& registry  = m_scene.GetRegistry();
        const TableLayout&      layout    = GetTableLayout(registry);
        const hlslpp::float3    coinHalf  = factory.CoinHalfExtent();
        const float             coinHalfX = coinHalf.x;
        const float             coinHalfY = coinHalf.y;
        const float             lift      = layout.initialLift;    // 床・コインから浮かせる高さ（めり込み防止）

        // プッシャー前の床にコインを手前端まで敷き詰める（実機と同じく、押せばすぐ縁から落ちる状態にしておく）。
        // 最前列はプッシャーが届く位置から
        const float pusherFrontMax = layout.pusherMinFrontZ + layout.pusherAmplitude * 2.0f;
        // 隙間があると押した分が隙間に吸われて縁まで伝わらないので、ほぼ接するくらいに詰める
        const float firstRowZ      = pusherFrontMax - layout.initialCoinFrontBack;
        const float pitch          = coinHalfX * 2.0f + layout.initialCoinGap;
        const int   rowCount       = static_cast<int>((layout.fieldFrontZ - coinHalfX - firstRowZ) / pitch) + 1;
        const int   halfColCount   = static_cast<int>((layout.fieldHalfWidth - coinHalfX - layout.initialCoinSideMargin) / pitch);
        for(int row = 0; row < rowCount; ++row) {
            for(int col = -halfColCount; col <= halfColCount; ++col) {
                const float x = static_cast<float>(col) * pitch;
                const float z = firstRowZ + static_cast<float>(row) * pitch;
                factory.CreateCoin(registry, hlslpp::float3(x, coinHalfY + lift, z));
            }
        }

        // プッシャーの上にもコインを数枚
        const float pusherStartZ = layout.PusherCenterZ(layout.pusherAmplitude);
        for(int col = -layout.pusherTopCoinHalfCount; col <= layout.pusherTopCoinHalfCount; ++col) {
            const float x = static_cast<float>(col) * layout.pusherTopCoinSpacing;
            factory.CreateCoin(registry, hlslpp::float3(x, layout.PusherTopY() + coinHalfY + lift, pusherStartZ + layout.pusherTopCoinOffsetZ));
        }

        //--------------------------------------------------------------
        // 最初から台にある果物。今の果樹の段階で出る果物から選び、敷き詰めたコインのすぐ上に置く
        // （高い所から落とすと下のコインを床へめり込ませてしまう）
        //--------------------------------------------------------------
        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        const int           level   = registry.GetContext<GameState>().treeLevel;
        std::mt19937        rng(std::random_device{}());

        for(size_t i = 0; i < layout.initialFruitX.size(); ++i) {
            const int fruitIndex = catalog.PickSpawnable(level, rng);
            if(fruitIndex < 0)
                break;

            const FruitDef&   def    = catalog.Fruits()[fruitIndex];
            const float       y      = coinHalfY * 2.0f + lift + def.HalfHeightOfBounds() + lift;
            const float       z      = static_cast<float>(i % 2) * layout.initialFruitStepZ;
            const VariantDef& normal = registry.GetContext<CollectionConfig>().Variants()[0];
            factory.CreateFruit(registry, def, fruitIndex, 0, normal.ColorOf(def), normal.glow, hlslpp::float3(layout.initialFruitX[i], y, z));
        }
    }

    //----------------------------------------------------------------------------
    //! ライト・空・カメラを生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateEnvironment() {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const StageConfig&      stage    = GetStageConfig(registry);
        const UiConfig&         ui       = GetUiConfig(registry);

        {
            // ディレクショナルライト
            Tsukino::ECS::Entity                              e     = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::DirectionalLightComponent& light = registry.AddComponent<Tsukino::BuiltIn::ECS::DirectionalLightComponent>(e);
            light.direction                                         = stage.sunDirection;
            light.color                                             = stage.sunColor;
            light.intensity                                         = stage.sunIntensity;
            light.castShadow                                        = stage.sunShadow;
        }
        {
            // 筐体の上の点光源
            Tsukino::ECS::Entity                       e         = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            transform.position                                   = stage.lampPosition;
            transform.dirty                                      = true;

            Tsukino::BuiltIn::ECS::PointLightComponent& light = registry.AddComponent<Tsukino::BuiltIn::ECS::PointLightComponent>(e);
            light.color                                      = stage.lampColor;
            light.intensity                                  = stage.lampIntensity;
            light.range                                      = stage.lampRange;
            light.enabled                                    = true;
        }
        {
            // 大気散乱（空）
            Tsukino::ECS::Entity e = m_scene.CreateEntity();
            registry.AddComponent<Tsukino::BuiltIn::ECS::SkyAtmosphereComponent>(e);
        }
        {
            // 台の周りをゆっくり漂う光の粒（屋台の夕暮れの雰囲気）
            Tsukino::ECS::Entity                             e         = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::AmbientParticleComponent& particles = registry.AddComponent<Tsukino::BuiltIn::ECS::AmbientParticleComponent>(e);
            particles.count                                            = stage.particleCount;
            particles.volumeSize                                       = stage.particleVolume;
            particles.color                                            = stage.particleColor;
            particles.minSize                                          = stage.particleMinSize;
            particles.maxSize                                          = stage.particleMaxSize;
            particles.driftVelocity                                    = stage.particleDrift;
            particles.swayAmplitude                                    = stage.particleSway;
            particles.nearFadeDistance                                 = stage.particleNearFade;
        }
        {
            // カメラ（プレイヤーの目線）
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = stage.cameraPosition;
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& cam = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            cam.useLookAt                               = true;
            cam.lookAtTarget                            = stage.cameraLookAt;
            cam.nearZ                                   = stage.cameraNear;
            cam.farZ                                    = stage.cameraFar;
            cam.isPrimary                               = true;
        }

        //--------------------------------------------------------------
        // 画面スプライト用の2Dカメラ。
        // エンジンはメインでない（isPrimary=false）カメラを画面スプライトの描画に使うため、
        // 正射影のカメラを1つ置く（無いと画面スプライトが出ない）。
        // Debug ビルドのデバッグカメラもメインでないカメラで、後から作った方が上書きされるため、
        // デバッグカメラより先に作る
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(0.0f, 0.0f, -10.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& camera = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            camera.projectionType                          = Tsukino::BuiltIn::ECS::CameraComponent::ProjectionType::Orthographic;
            camera.orthoSize                               = ui.screenHeight;
            camera.isPrimary                               = false;
        }

#ifdef _DEBUG
        {
            // デバッグカメラ（Debug ビルドのみ。切り替えは DebugCameraSystem の操作に従う）
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = stage.debugCameraPosition;
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& cam = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            cam.lookAtTarget                            = stage.debugCameraLookAt;
            cam.nearZ                                   = stage.cameraNear;
            cam.farZ                                    = stage.cameraFar;
            cam.isPrimary                               = false;

            Tsukino::BuiltIn::ECS::DebugCameraComponent& debug = registry.AddComponent<Tsukino::BuiltIn::ECS::DebugCameraComponent>(e);
            debug.moveSpeed                                    = stage.debugCameraSpeed;
            debug.sprintSpeed                                  = stage.debugCameraSprint;

            registry.AddComponent<Tsukino::BuiltIn::ECS::DebugCameraTag>(e);
        }
#endif
    }

    //----------------------------------------------------------------------------
    //! コインの投入口と HUD を生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreatePlayerInterface(PrizeFactory& factory) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const TableLayout&      layout   = GetTableLayout(registry);
        const StageConfig&      stage    = GetStageConfig(registry);
        const UiConfig&         ui       = GetUiConfig(registry);
        const Texts&            texts    = GetTexts(registry);

        //--------------------------------------------------------------
        // 投入口の目印（半透明の板。CoinLauncherSystem が投入位置へ動かす）
        //--------------------------------------------------------------
        {
            const hlslpp::float3 coinHalf = factory.CoinHalfExtent();
            Tsukino::ECS::Entity e        = factory.CreateVisualBox(registry, hlslpp::float3(0.0f, layout.LaunchMarkerY(), layout.LaunchZ()),
                                                                    hlslpp::float3(coinHalf.x, stage.launchMarkerHalfThickness, coinHalf.z), stage.launchMarkerOpacity);
            ECS::CoinLauncherComponent& launcher = registry.AddComponent<ECS::CoinLauncherComponent>(e);
            if(registry.HasContext<EconomyConfig>()) {
                launcher.interval  = registry.GetContext<EconomyConfig>().launchInterval;
                launcher.laneSpeed = registry.GetContext<EconomyConfig>().launchLaneSpeed;
            }
        }

        //--------------------------------------------------------------
        // チェッカー（左右に動く穴）の目印。台の手前端のすぐ下、コインが最終的に落ちる払い出し口にあり、
        // CheckerSystem が左右に動かす。判定は落ちたコインの位置で行うので、これは見た目だけ（コライダー無し）
        //--------------------------------------------------------------
        {
            const float          halfWidth = registry.GetContext<TableStats>().checkerHalfWidth;
            Tsukino::ECS::Entity e         = factory.CreateVisualBox(registry, hlslpp::float3(0.0f, stage.checkerMarkerY, layout.fieldFrontZ + stage.checkerMarkerOffsetZ),
                                                                     hlslpp::float3(halfWidth, stage.checkerMarkerHalfThickness, stage.checkerMarkerHalfDepth), 1.0f,
                                                                     stage.checkerMarkerGlow.color);
            AddGlow(registry, e, stage.checkerMarkerGlow);

            // 穴の幅の強化で目印も伸ばせるよう、幅あたりのスケールを覚えておく
            ECS::CheckerComponent& checker = registry.AddComponent<ECS::CheckerComponent>(e);
            checker.scalePerHalfWidth      = float(registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e).scale.x) / halfWidth;
        }

        //--------------------------------------------------------------
        // HUD（座標は画面左上からのピクセル。文字の大きさは scale.x）
        //--------------------------------------------------------------
        for(const HudKindName& kind : kHudKinds) {
            const UiText&                              spec = ui.HudText(kind.name);
            Tsukino::ECS::Entity                       e    = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t    = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                      = hlslpp::float3(spec.position.x, spec.position.y, 0.0f);
            t.scale                                         = hlslpp::float3(spec.scale, spec.scale, 1.0f);
            t.dirty                                         = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.fontHandle                            = GetUiFont(registry, spec.bold);
            font.color                                 = spec.color;
            font.horizontalAlign                       = ToEngineAlign(spec.align);
            // 収める幅。指定が無ければ、揃え方に合わせて画面の端まで（余白を残す）
            if(spec.maxWidth > 0.0f)
                font.maxWidth = spec.maxWidth;
            else if(spec.align == UiAlign::Left)
                font.maxWidth = ui.screenWidth - float(spec.position.x) - ui.textPadding;
            else if(spec.align == UiAlign::Right)
                font.maxWidth = float(spec.position.x) - ui.textPadding;
            else
                font.maxWidth = std::min(float(spec.position.x), ui.screenWidth - float(spec.position.x)) * 2.0f - ui.textPadding * 2.0f;
            font.outlineColor                          = ui.hudOutlineColor;
            font.outlineWidth                          = ui.outlineWidth;

            ECS::HudTextComponent& hud = registry.AddComponent<ECS::HudTextComponent>(e);
            hud.kind                   = kind.kind;
        }

        //--------------------------------------------------------------
        // 画面スプライト（白い小さなテクスチャを色付けして使い回す。位置は中心）
        //--------------------------------------------------------------
        Tsukino::EngineIntegration::EngineContext* context = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        const Tsukino::Asset::AssetHandle          white   = context->assetManager->Load(Tsukino::Core::Path(AssetPaths::kWhiteTexture));

        auto createPanel = [&](const hlslpp::float2& center, const hlslpp::float2& size, const hlslpp::float4& color, int sortOrder) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(center.x, center.y, 0.0f);
            t.scale                                      = hlslpp::float3(size.x / AssetPaths::kWhiteTextureSize, size.y / AssetPaths::kWhiteTextureSize, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e);
            sprite.textureHandle                           = white;
            sprite.tintColor                               = color;
            sprite.sortOrder                               = sortOrder;
            return e;
        };

        //--------------------------------------------------------------
        // 画面全体を一瞬光らせる板（加算。ふだんはスケール 0 で描かない。EffectsSystem が光らせる）。
        // HUD の文字やボタンより奥に置き、文字が読めなくならないようにする
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity flash = createPanel(ui.ScreenCenter(), hlslpp::float2(ui.screenWidth, ui.screenHeight), hlslpp::float4(0.0f, 0.0f, 0.0f, 0.0f), -10);
            registry.GetComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(flash).blendMode = Tsukino::BuiltIn::ECS::SpriteBlendMode::Additive;
            ECS::ScreenFlashComponent& component = registry.AddComponent<ECS::ScreenFlashComponent>(flash);
            component.fullScale                  = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(flash).scale;
            registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(flash).scale = hlslpp::float3(0.0f, 0.0f, 1.0f);
        }

        //--------------------------------------------------------------
        // マナゲージ（背景＋中身。中身の幅は HudSystem がマナに合わせて変える）
        //--------------------------------------------------------------
        createPanel(hlslpp::float2(ui.manaGaugeLeft + ui.manaGaugeWidth * 0.5f, ui.manaGaugeCenterY),
                    hlslpp::float2(ui.manaGaugeWidth + ui.manaGaugePadding, ui.manaGaugeHeight + ui.manaGaugePadding), ui.manaGaugeBackColor, 0);
        {
            Tsukino::ECS::Entity     e     = createPanel(hlslpp::float2(ui.manaGaugeLeft, ui.manaGaugeCenterY), hlslpp::float2(0.0f, ui.manaGaugeHeight),
                                                         ui.manaGaugeFillColor, 1);
            ECS::ManaGaugeComponent& gauge = registry.AddComponent<ECS::ManaGaugeComponent>(e);
            gauge.left                     = ui.manaGaugeLeft;
            gauge.fullWidth                = ui.manaGaugeWidth;
            gauge.height                   = ui.manaGaugeHeight;
            gauge.textureSize              = AssetPaths::kWhiteTextureSize;
        }

        //--------------------------------------------------------------
        // おすそわけ待ちのリング（「おすそわけ待ち」の文字の左。下地＋中身）。
        // 待っている間だけ HudSystem が表示し、中身を真上から時計回りに塗る
        //--------------------------------------------------------------
        {
            const Tsukino::Asset::AssetHandle ring       = context->assetManager->Load(Tsukino::Core::Path(AssetPaths::kRingTexture));
            const float                       shownScale = ui.reliefRingDiameter / AssetPaths::kRingTextureSize;

            auto createRing = [&](ECS::ReliefGaugePart part, const hlslpp::float4& color, int sortOrder) {
                Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
                Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
                t.position                                   = hlslpp::float3(ui.reliefRingCenter.x, ui.reliefRingCenter.y, 0.0f);
                t.scale                                      = hlslpp::float3(0.0f, 0.0f, 1.0f);    // 待っていない間は隠す
                t.dirty                                      = true;

                Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e);
                sprite.textureHandle                           = ring;
                sprite.tintColor                               = color;
                sprite.sortOrder                               = sortOrder;

                ECS::ReliefGaugeComponent& gauge = registry.AddComponent<ECS::ReliefGaugeComponent>(e);
                gauge.part                       = part;
                gauge.shownScale                 = shownScale;
                return e;
            };

            createRing(ECS::ReliefGaugePart::Background, ui.reliefRingBackColor, 0);
            Tsukino::ECS::Entity fill = createRing(ECS::ReliefGaugePart::Fill, ui.reliefRingFillColor, 1);
            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.GetComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(fill);
            sprite.fillMode                                = Tsukino::BuiltIn::ECS::SpriteFillMode::Radial;
            sprite.fillAmount                              = 0.0f;
        }

        //--------------------------------------------------------------
        // 魔法ボタン（画面下に横並び。色と文字は HudSystem、クリックは MagicInputSystem が扱う）
        //--------------------------------------------------------------
        const float totalWidth = ui.magicButtonWidth * kMagicSlotCount + ui.magicButtonGap * (kMagicSlotCount - 1);
        const float firstX     = ui.ScreenCenter().x - totalWidth * 0.5f + ui.magicButtonWidth * 0.5f;
        for(int slot = 1; slot <= kMagicSlotCount; ++slot) {
            const hlslpp::float2 center(firstX + (ui.magicButtonWidth + ui.magicButtonGap) * static_cast<float>(slot - 1), ui.magicButtonY);

            Tsukino::ECS::Entity button = createPanel(center, hlslpp::float2(ui.magicButtonWidth, ui.magicButtonHeight), ui.magicLockedColor, 10);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);

            // ボタンの上の文字（ボタンの中心に揃える）
            Tsukino::ECS::Entity                       label = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t     = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(label);
            t.position                                       = hlslpp::float3(center.x, center.y, 0.0f);
            t.scale                                          = hlslpp::float3(ui.magicLabelScale, ui.magicLabelScale, 1.0f);
            t.dirty                                          = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(label);
            font.fontHandle                            = GetUiFont(registry, true);    // ボタンの文字は太字
            font.maxWidth                              = ui.magicButtonWidth - ui.textPadding * 2.0f;
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = ui.magicLabelOutline;
            font.outlineWidth                          = ui.outlineWidth;
            font.sortOrder                             = 11;

            ECS::MagicButtonComponent& magicButton = registry.AddComponent<ECS::MagicButtonComponent>(button);
            magicButton.slot                       = slot;
            magicButton.label                      = label;
        }

        //--------------------------------------------------------------
        // 文字のエンティティを作る（図鑑とボタンで共通）。
        // maxWidth は収める幅（置く枠から決める。超えたらエンジンが縮めて描く。0 なら制限なし）
        //--------------------------------------------------------------
        auto createText = [&](const hlslpp::float2& position, float scale, Tsukino::BuiltIn::ECS::HorizontalAlign align, const hlslpp::float4& color, int sortOrder,
                              bool bold, float maxWidth) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(position.x, position.y, 0.0f);
            t.scale                                      = hlslpp::float3(scale, scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.fontHandle                            = GetUiFont(registry, bold);
            font.color                                 = color;
            font.horizontalAlign                       = align;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = ui.textOutlineColor;
            font.outlineWidth                          = ui.outlineWidth;
            font.sortOrder                             = sortOrder;
            font.maxWidth                              = std::max(0.0f, maxWidth);
            return e;
        };
        auto createFontText = [&](const hlslpp::float2& position, const UiFont& style, Tsukino::BuiltIn::ECS::HorizontalAlign align, int sortOrder, float maxWidth) {
            return createText(position, style.scale, align, style.color, sortOrder, style.bold, maxWidth);
        };
        // 幅 width の枠の中に、両側に余白を残して収める幅
        auto inside = [&](float width) { return width - ui.textPadding * 2.0f; };
        constexpr auto kLeft   = Tsukino::BuiltIn::ECS::HorizontalAlign::Left;
        constexpr auto kCenter = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
        const hlslpp::float4 kWhite(1.0f, 1.0f, 1.0f, 1.0f);

        //--------------------------------------------------------------
        // 画面（図鑑・強化）の開閉ボタン（右上。文字は MenuSystem が開閉に合わせて書く）
        //--------------------------------------------------------------
        struct MenuButtonSpec {
            MenuKind                menu;
            Tsukino::Input::KeyCode key;
            float                   y;
            hlslpp::float4          color;
            const char*             closedText;
            const char*             openText;
        };
        const MenuButtonSpec menuButtons[] = {
            {MenuKind::Zukan, Tsukino::Input::KeyCode::Tab, ui.zukanButtonY, ui.zukanButtonColor, "menu.zukanButton", "menu.zukanClose"},
            {MenuKind::Upgrade, Tsukino::Input::KeyCode::U, ui.upgradeButtonY, ui.upgradeButtonColor, "menu.upgradeButton", "menu.upgradeClose"},
            // Esc は MenuSystem が別に扱う（開いている画面を閉じる・何も無ければオプション）ので、ボタンにはキーを割り当てない
            {MenuKind::Options, Tsukino::Input::KeyCode::None, ui.optionsButtonY, ui.optionsButtonColor, "menu.optionsButton", "menu.optionsClose"},
        };
        for(const MenuButtonSpec& spec : menuButtons) {
            Tsukino::ECS::Entity button = createPanel(hlslpp::float2(ui.menuButtonX, spec.y), hlslpp::float2(ui.menuButtonWidth, ui.menuButtonHeight), spec.color, 10);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);

            ECS::MenuButtonComponent& menuButton = registry.AddComponent<ECS::MenuButtonComponent>(button);
            menuButton.menu                      = spec.menu;
            menuButton.key                       = spec.key;
            menuButton.label      = createText(hlslpp::float2(ui.menuButtonX, spec.y), ui.menuLabelScale, kCenter, kWhite, 11, true, inside(ui.menuButtonWidth));
            menuButton.closedText = texts.Get(spec.closedText);
            menuButton.openText   = texts.Get(spec.openText);
        }

        //--------------------------------------------------------------
        // 画面の要素にする（開閉に合わせて MenuSystem が表示を切り替える）。
        // text は決まった文字（タイトル・見出し・説明）。空なら画面ごとのシステムが書く
        //--------------------------------------------------------------
        auto addPage = [&](Tsukino::ECS::Entity e, MenuKind menu, const std::wstring& text = L"") {
            ECS::MenuPageComponent& page = registry.AddComponent<ECS::MenuPageComponent>(e);
            page.menu                    = menu;
            page.openScale               = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e).scale;
            page.text                    = text;
            return e;
        };

        // 画面の上下左右の端
        const float menuTop    = ui.menuCenter.y - ui.menuSize.y * 0.5f;
        const float menuBottom = ui.menuCenter.y + ui.menuSize.y * 0.5f;
        const float menuLeft   = ui.menuCenter.x - ui.menuSize.x * 0.5f;
        const float menuRight  = ui.menuCenter.x + ui.menuSize.x * 0.5f;

        // 画面の背景のパネル。画面の上のクリックでコインが投入されないよう、クリックを受け止める
        auto createMenuPanel = [&](MenuKind menu, const std::wstring& title) {
            Tsukino::ECS::Entity panel = createPanel(ui.menuCenter, ui.menuSize, ui.menuColor, 20);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(panel);
            addPage(panel, menu);

            addPage(createFontText(hlslpp::float2(ui.menuCenter.x, menuTop + ui.menuTitleOffsetY), ui.menuTitle, kCenter, 22, inside(ui.menuSize.x)), menu, title);
        };

        //--------------------------------------------------------------
        // 画面の中のスクロールする行の領域。枠（切り取り＋スクロール）・中身・右端のスクロールバーを作る。
        // 行は中身の子にし、位置は枠の左上から見た相対位置で置く（attach が直す）。
        // 枠は MenuPageComponent を持ち、開いている画面の枠だけ MenuSystem がスクロールを受け付けさせる
        //--------------------------------------------------------------
        struct ScrollList {
            Tsukino::ECS::Entity content = entt::null;    // 行の親
            float                left    = 0.0f;          // 枠の左端（画面ピクセル）
            float                top     = 0.0f;          // 枠の上端（画面ピクセル）
        };
        auto createScrollList = [&](MenuKind menu, float left, float top, float right, float bottom, float contentHeight, const hlslpp::float4& thumbColor) {
            const float width  = right - left;
            const float height = bottom - top;

            Tsukino::ECS::Entity                       view          = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& viewTransform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(view);
            viewTransform.position                                   = hlslpp::float3(left + width * 0.5f, top + height * 0.5f, 0.0f);
            viewTransform.dirty                                      = true;
            registry.AddComponent<Tsukino::BuiltIn::ECS::UIClipComponent>(view).size = hlslpp::float2(width, height);
            addPage(view, menu);

            Tsukino::ECS::Entity                       content          = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& contentTransform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(content);
            contentTransform.parent                                     = view;
            contentTransform.position                                   = hlslpp::float3(-width * 0.5f, -height * 0.5f, 0.0f);    // 一番上までスクロールした位置
            contentTransform.dirty                                      = true;

            // スクロールバー（溝とつまみ）。表示・大きさ・位置は ScrollViewSystem が決める（中身が収まるときは出ない）
            const float          barX  = menuRight - ui.scrollBarInset;
            Tsukino::ECS::Entity track = createPanel(hlslpp::float2(barX, top + height * 0.5f), hlslpp::float2(ui.scrollBarWidth, height), ui.scrollTrackColor, 21);
            Tsukino::ECS::Entity thumb = createPanel(hlslpp::float2(barX, top), hlslpp::float2(ui.scrollBarWidth, ui.scrollBarWidth), thumbColor, 22);

            // 画面のパネルと同じく、スクロールバーの上のクリックでコインが投入されないよう受け止める
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(track);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(thumb);

            Tsukino::BuiltIn::ECS::ScrollBarComponent& bar = registry.AddComponent<Tsukino::BuiltIn::ECS::ScrollBarComponent>(track);
            bar.thumb                                      = thumb;
            bar.size                                       = hlslpp::float2(ui.scrollBarWidth, height);

            Tsukino::BuiltIn::ECS::ScrollViewComponent& scroll = registry.AddComponent<Tsukino::BuiltIn::ECS::ScrollViewComponent>(view);
            scroll.content                                     = content;
            scroll.scrollBar                                   = track;
            scroll.contentHeight                               = contentHeight;
            scroll.enabled                                     = false;    // 画面を開いたら MenuSystem が有効にする

            return ScrollList{content, left, top};
        };

        // 画面の座標で作った要素を、スクロールする行の領域の中身の子にする
        auto attach = [&](const ScrollList& list, Tsukino::ECS::Entity e) {
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.parent                                     = list.content;
            t.position                                   = t.position - hlslpp::float3(list.left, list.top, 0.0f);
            t.dirty                                      = true;
            return e;
        };

        //--------------------------------------------------------------
        // 図鑑の画面（行＝果物、列＝バリエーション）。最初は閉じていて、ZukanSystem が開閉する。
        // バリエーションの数は定義データから決まるので、列の間隔もそれに合わせる
        //--------------------------------------------------------------
        {
            const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
            const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
            const int               rows       = static_cast<int>(catalog.Fruits().size());
            const int               columns    = static_cast<int>(collection.Variants().size());

            auto addElement = [&](Tsukino::ECS::Entity e, ECS::ZukanElementKind kind, int fruitIndex = -1, int variantIndex = -1) {
                addPage(e, MenuKind::Zukan);
                ECS::ZukanElementComponent& element = registry.AddComponent<ECS::ZukanElementComponent>(e);
                element.kind                        = kind;
                element.fruitIndex                  = fruitIndex;
                element.variantIndex                = variantIndex;
            };

            createMenuPanel(MenuKind::Zukan, texts.Get("zukan.title"));

            // 列の配置（バリエーションが増えたら間隔を詰める）
            const float columnsLeft  = menuLeft + ui.zukanColumnsLeft;
            const float columnsWidth = ui.menuSize.x - ui.zukanColumnsLeft - ui.zukanColumnsRight;
            const float columnPitch  = columnsWidth / static_cast<float>(std::max(1, columns));
            auto        columnX      = [&](int v) { return columnsLeft + columnPitch * (static_cast<float>(v) + 0.5f); };

            // 列の見出し
            const float headerY = menuTop + ui.zukanHeaderOffsetY;
            for(int v = 0; v < columns; ++v) {
                const std::wstring& name = collection.Variants()[v].name;
                addPage(createFontText(hlslpp::float2(columnX(v), headerY), ui.zukanHeader, kCenter, 22, inside(columnPitch)), MenuKind::Zukan,
                        name.empty() ? texts.Get("zukan.normalVariant") : name);
            }

            // 行（果物が増えて枠に収まらなくなったらスクロールする。行の高さは変えない）
            const float      rowsTop = headerY + ui.zukanRowsGap;
            const float      pitch   = ui.zukanRowPitch;
            const ScrollList list    = createScrollList(MenuKind::Zukan, menuLeft + ui.zukanListLeft, rowsTop, menuRight - ui.zukanListRight, menuBottom - ui.zukanRowsBottom,
                                                        pitch * static_cast<float>(rows), ui.zukanThumbColor);
            // 数（×3 など）は、次の列の色見本の左端まで
            const float countWidth = columnPitch + ui.zukanSwatchOffsetX - ui.zukanSwatchSize * 0.5f - ui.zukanCountOffsetX - ui.textPadding;
            for(int f = 0; f < rows; ++f) {
                const float y = rowsTop + pitch * (static_cast<float>(f) + 0.5f);

                addElement(attach(list, createFontText(hlslpp::float2(menuLeft + ui.zukanNameX, y), ui.zukanName, kLeft, 22, ui.zukanColumnsLeft - ui.zukanNameX - ui.textPadding)), ECS::ZukanElementKind::RowName, f);

                for(int v = 0; v < columns; ++v) {
                    const float x = columnX(v);
                    addElement(attach(list, createPanel(hlslpp::float2(x + ui.zukanSwatchOffsetX, y), hlslpp::float2(ui.zukanSwatchSize, ui.zukanSwatchSize),
                                                        ui.zukanSwatchColor, 21)),
                               ECS::ZukanElementKind::Swatch, f, v);
                    addElement(attach(list, createFontText(hlslpp::float2(x + ui.zukanCountOffsetX, y), ui.zukanCount, kLeft, 22, countWidth)),
                               ECS::ZukanElementKind::Count, f, v);
                }
            }

            // 下部の集計
            addElement(createFontText(hlslpp::float2(ui.menuCenter.x, menuBottom - ui.zukanFooterOffsetY), ui.zukanFooter, kCenter, 22, inside(ui.menuSize.x)), ECS::ZukanElementKind::Footer);
        }

        //--------------------------------------------------------------
        // 強化の画面（行＝強化）。最初は閉じている
        //--------------------------------------------------------------
        {
            const UpgradeCatalog& upgrades = registry.GetContext<UpgradeCatalog>();
            const int             rows     = static_cast<int>(upgrades.Upgrades().size());

            auto addElement = [&](Tsukino::ECS::Entity e, ECS::UpgradeElementKind kind, int upgradeIndex = -1) {
                addPage(e, MenuKind::Upgrade);
                ECS::UpgradeElementComponent& element = registry.AddComponent<ECS::UpgradeElementComponent>(e);
                element.kind                          = kind;
                element.upgradeIndex                  = upgradeIndex;
            };

            createMenuPanel(MenuKind::Upgrade, texts.Get("upgrade.title"));

            // 手持ち（価格と見比べられるよう、タイトルのすぐ下）
            addElement(createFontText(hlslpp::float2(ui.menuCenter.x, menuTop + ui.upgradeWalletOffsetY), ui.upgradeWallet, kCenter, 22, inside(ui.menuSize.x)),
                       ECS::UpgradeElementKind::Wallet);

            // 行（強化が増えて枠に収まらなくなったらスクロールする。行の高さは変えない）
            const float      rowsTop = menuTop + ui.upgradeRowsTop;
            const float      pitch   = ui.upgradeRowPitch;
            const float      buttonX = menuRight - ui.upgradeButtonRight - ui.upgradeButtonWidth * 0.5f;
            const float      nameX   = menuLeft + ui.upgradeNameX;
            const float      effectX = menuLeft + ui.upgradeEffectX;
            const ScrollList list    = createScrollList(MenuKind::Upgrade, menuLeft + ui.upgradeListLeft, rowsTop, menuRight - ui.upgradeListRight,
                                                        menuBottom - ui.upgradeRowsBottom, pitch * static_cast<float>(rows), ui.upgradeThumbColor);
            // 名前と説明は効果の列まで、効果と価格は購入ボタンの左端まで
            const float nameWidth   = effectX - nameX - ui.textPadding;
            const float effectWidth = buttonX - ui.upgradeButtonWidth * 0.5f - effectX - ui.textPadding;
            for(int u = 0; u < rows; ++u) {
                const UpgradeDef& def = upgrades.Upgrades()[u];
                const float       y   = rowsTop + pitch * (static_cast<float>(u) + 0.5f);
                const float       dy  = pitch * ui.upgradeRowSpread;    // 1行の中の上段・下段のずれ

                // 名前とレベル（上段）・説明（下段）
                addElement(attach(list, createFontText(hlslpp::float2(nameX, y - dy), ui.upgradeName, kLeft, 22, nameWidth)), ECS::UpgradeElementKind::Name, u);
                addPage(attach(list, createFontText(hlslpp::float2(nameX, y + dy), ui.upgradeDescription, kLeft, 22, nameWidth)), MenuKind::Upgrade, def.description);

                // 効果（上段）・価格（下段。色は UpgradeSystem が買えるかどうかで変える）
                addElement(attach(list, createFontText(hlslpp::float2(effectX, y - dy), ui.upgradeEffect, kLeft, 22, effectWidth)), ECS::UpgradeElementKind::Effect, u);
                addElement(attach(list, createText(hlslpp::float2(effectX, y + dy), ui.upgradeCostScale, kLeft, ui.upgradeAffordableColor, 22, false, effectWidth)),
                           ECS::UpgradeElementKind::Cost, u);

                // 購入ボタン（色は UpgradeSystem が買えるかどうかで変える）
                Tsukino::ECS::Entity button = attach(list, createPanel(hlslpp::float2(buttonX, y), hlslpp::float2(ui.upgradeButtonWidth, ui.upgradeButtonHeight),
                                                                       ui.upgradeCannotBuyColor, 21));
                registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
                addElement(button, ECS::UpgradeElementKind::BuyButton, u);
                addElement(attach(list, createText(hlslpp::float2(buttonX, y), ui.upgradeLabelScale, kCenter, kWhite, 22, true, inside(ui.upgradeButtonWidth))), ECS::UpgradeElementKind::BuyLabel, u);
            }
        }

        //--------------------------------------------------------------
        // 「おかえり」画面（閉じていた間の報酬）。起動時に開いていれば出し、ボタンで閉じる。
        // 中身は起動時に決まるので、すべて決まった文字として作る。ほかの画面より手前に重ねる
        //--------------------------------------------------------------
        {
            const OfflineReport& report = registry.GetContext<OfflineReport>();
            const TableStats&    stats  = registry.GetContext<TableStats>();
            const float          top    = ui.menuCenter.y - ui.welcomeSize.y * 0.5f;
            const float          bottom = ui.menuCenter.y + ui.welcomeSize.y * 0.5f;

            Tsukino::ECS::Entity panel = createPanel(ui.menuCenter, ui.welcomeSize, ui.welcomeColor, 30);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(panel);
            addPage(panel, MenuKind::Welcome);

            auto addLine = [&](float y, const UiFont& style, const std::wstring& text) {
                addPage(createFontText(hlslpp::float2(ui.menuCenter.x, top + y), style, kCenter, 32, inside(ui.welcomeSize.x)), MenuKind::Welcome, text);
            };

            addLine(ui.welcomeTitleY, ui.welcomeTitle, texts.Get("welcome.title"));
            addLine(ui.welcomeAwayY, ui.welcomeAway, texts.Format("welcome.away", {{"duration", FormatDuration(report.awaySeconds, texts)}}));
            if(report.hasFairy) {
                std::wstring reward = texts.Format("welcome.reward", {{"coins", std::to_wstring(report.coins)}});
                if(report.fruitCount > 0)
                    reward += texts.Format("welcome.rewardFruit", {{"fruit", std::to_wstring(report.fruitPoints)}, {"count", std::to_wstring(report.fruitCount)}});
                addLine(ui.welcomeRewardY, ui.welcomeReward, reward + texts.Get("welcome.rewardTail"));
            } else {
                addLine(ui.welcomeRewardY, ui.welcomeNoFairy, texts.Get("welcome.noFairy"));
            }
            if(report.capped) {
                const long long limitSeconds = static_cast<long long>(stats.offlineMaxHours * 3600.0f);
                addLine(ui.welcomeCappedY, ui.welcomeCapped, texts.Format("welcome.capped", {{"limit", FormatDuration(limitSeconds, texts)}}));
            }

            // 閉じるボタン（Enter でも閉じる。閉じた後は開かない）
            const hlslpp::float2 buttonCenter(ui.menuCenter.x, bottom - ui.welcomeButtonOffsetY);
            Tsukino::ECS::Entity button = createPanel(buttonCenter, ui.welcomeButtonSize, ui.welcomeButtonColor, 31);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
            addPage(button, MenuKind::Welcome);

            ECS::MenuButtonComponent& close = registry.AddComponent<ECS::MenuButtonComponent>(button);
            close.menu                      = MenuKind::Welcome;
            close.key                       = Tsukino::Input::KeyCode::Enter;
            close.label                     = addPage(createText(buttonCenter, ui.welcomeLabelScale, kCenter, kWhite, 32, true, inside(ui.welcomeButtonSize.x)), MenuKind::Welcome);
            close.openText                  = texts.Get("welcome.close");
            close.canOpen                   = false;
        }

        //--------------------------------------------------------------
        // オプション画面。タイトルの下をスクロールする領域にし、上から設定（音量・消音・操作説明の表示）と
        // 「ゲームを終了」を並べる。「データを消して最初から」は最初に見える範囲より下に置き、スクロールしないと見えない。
        // ボタンの文字と色、音量の表示は OptionsSystem が書く
        //--------------------------------------------------------------
        {
            createMenuPanel(MenuKind::Options, texts.Get("options.title"));

            // 中身の高さは「データ」の段の位置で決まるので、先に並べる位置を出す
            const float      listTop     = menuTop + ui.optionsListTop;
            const float      listBottom  = menuBottom - ui.optionsListBottom;
            const float      dataTitleY  = listBottom + ui.optionsDataBelowFold;
            const float      resetY      = dataTitleY + ui.optionsDataButtonGap;
            const float      contentSize = resetY + ui.optionsResetSize.y * 0.5f + ui.optionsBottomMargin - listTop;
            const ScrollList list        = createScrollList(MenuKind::Options, menuLeft + ui.optionsListLeft, listTop, menuRight - ui.optionsListRight, listBottom,
                                                            contentSize, ui.optionsThumbColor);

            // 文字を1つ置く（中身の子にする）
            auto addText = [&](const hlslpp::float2& position, const UiFont& style, Tsukino::BuiltIn::ECS::HorizontalAlign align, float maxWidth,
                               const std::wstring& text = L"") {
                return addPage(attach(list, createFontText(position, style, align, 22, maxWidth)), MenuKind::Options, text);
            };
            // ボタンを1つ置く（中身の子にする。文字は OptionsSystem が書く）
            auto addButton = [&](ECS::OptionsElementKind kind, const hlslpp::float2& center, const hlslpp::float2& size) {
                Tsukino::ECS::Entity button = attach(list, createPanel(center, size, ui.optionsButtonFill, 21));
                registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
                addPage(button, MenuKind::Options);
                ECS::OptionsElementComponent& element = registry.AddComponent<ECS::OptionsElementComponent>(button);
                element.kind                          = kind;
                element.label = addPage(attach(list, createText(center, ui.optionsButtonLabelScale, kCenter, kWhite, 22, true, inside(size.x))), MenuKind::Options);
            };

            //--------------------------------------------------------------
            // 設定の行と「ゲームを終了」
            //--------------------------------------------------------------
            const float labelX = menuLeft + ui.optionsLabelX;
            const float dataWidth = (menuRight - ui.optionsListRight) - labelX - ui.textPadding;    // データの見出しと説明は、スクロールする領域の右端まで
            auto        rowY   = [&](int row) { return listTop + ui.optionsRowsTop + ui.optionsRowPitch * static_cast<float>(row); };
            // 項目名は、右にあるボタン（「−」かオン/オフ）の左端まで
            const float labelWidth = std::min(ui.optionsMinusX - ui.optionsStepSize.x * 0.5f, ui.optionsValueX - ui.optionsToggleSize.x * 0.5f) - ui.optionsLabelX - ui.textPadding;
            auto        label      = [&](int row, const char* key) { addText(hlslpp::float2(labelX, rowY(row)), ui.optionsLabel, kLeft, labelWidth, texts.Get(key)); };
            auto volumeRow = [&](int row, const char* key, ECS::OptionsElementKind down, ECS::OptionsElementKind value, ECS::OptionsElementKind up) {
                label(row, key);
                addButton(down, hlslpp::float2(menuLeft + ui.optionsMinusX, rowY(row)), ui.optionsStepSize);
                registry.AddComponent<ECS::OptionsElementComponent>(addText(hlslpp::float2(menuLeft + ui.optionsValueX, rowY(row)), ui.optionsValue, kCenter,
                                                                            ui.optionsPlusX - ui.optionsMinusX - ui.optionsStepSize.x - ui.textPadding)).kind = value;
                addButton(up, hlslpp::float2(menuLeft + ui.optionsPlusX, rowY(row)), ui.optionsStepSize);
            };
            volumeRow(0, "options.bgm", ECS::OptionsElementKind::BgmDown, ECS::OptionsElementKind::BgmValue, ECS::OptionsElementKind::BgmUp);
            volumeRow(1, "options.se", ECS::OptionsElementKind::SeDown, ECS::OptionsElementKind::SeValue, ECS::OptionsElementKind::SeUp);
            label(2, "options.mute");
            addButton(ECS::OptionsElementKind::Mute, hlslpp::float2(menuLeft + ui.optionsValueX, rowY(2)), ui.optionsToggleSize);
            label(3, "options.hint");
            addButton(ECS::OptionsElementKind::Hint, hlslpp::float2(menuLeft + ui.optionsValueX, rowY(3)), ui.optionsToggleSize);
            addButton(ECS::OptionsElementKind::Quit, hlslpp::float2(labelX + ui.optionsQuitSize.x * 0.5f, rowY(4)), ui.optionsQuitSize);

            //--------------------------------------------------------------
            // データ（スクロールした先）: 見出し・説明・「データを消して最初から」（押すと確認ウィンドウ）
            //--------------------------------------------------------------
            addText(hlslpp::float2(labelX, dataTitleY), ui.optionsDataTitle, kLeft, dataWidth, texts.Get("options.dataTitle"));
            addText(hlslpp::float2(labelX, dataTitleY + ui.optionsDataNoteGap), ui.optionsDataNote, kLeft, dataWidth, texts.Get("options.dataNote"));
            addButton(ECS::OptionsElementKind::Reset, hlslpp::float2(labelX + ui.optionsResetSize.x * 0.5f, resetY), ui.optionsResetSize);
        }

        //--------------------------------------------------------------
        // データ消去の確認ウィンドウ（3回）。ほかのすべての画面より手前に出し、後ろは暗い板で覆ってクリックを受け止める。
        // 出しているかどうか・文字・色は OptionsSystem が OptionsState を見て決める（最初は隠す）
        //--------------------------------------------------------------
        {
            const float top    = ui.menuCenter.y - ui.confirmSize.y * 0.5f;
            const float bottom = ui.menuCenter.y + ui.confirmSize.y * 0.5f;

            // 確認ウィンドウの部品にする（スプライトは出すときのスケールを覚えて隠す）
            auto addPart = [&](Tsukino::ECS::Entity e, ECS::OptionsDialogPart part) {
                ECS::OptionsDialogComponent& component = registry.AddComponent<ECS::OptionsDialogComponent>(e);
                component.part                         = part;
                if(registry.HasComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e)) {
                    Tsukino::BuiltIn::ECS::TransformComponent& t = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
                    component.shownScale                         = t.scale;
                    t.scale                                      = hlslpp::float3(0.0f, 0.0f, 1.0f);
                }
                return e;
            };

            Tsukino::ECS::Entity dimmer = createPanel(ui.ScreenCenter(), hlslpp::float2(ui.screenWidth, ui.screenHeight), ui.confirmDimColor, 40);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(dimmer);
            addPart(dimmer, ECS::OptionsDialogPart::Dimmer);
            Tsukino::ECS::Entity window = createPanel(ui.menuCenter, ui.confirmSize, ui.confirmColor, 41);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(window);
            addPart(window, ECS::OptionsDialogPart::Window);

            addPart(createFontText(hlslpp::float2(ui.menuCenter.x, top + ui.confirmStepY), ui.confirmStep, kCenter, 42, inside(ui.confirmSize.x)), ECS::OptionsDialogPart::Step);
            addPart(createFontText(hlslpp::float2(ui.menuCenter.x, top + ui.confirmMessageY), ui.confirmMessage, kCenter, 42, inside(ui.confirmSize.x)), ECS::OptionsDialogPart::Message);
            addPart(createFontText(hlslpp::float2(ui.menuCenter.x, top + ui.confirmNoteY), ui.confirmNote, kCenter, 42, inside(ui.confirmSize.x)), ECS::OptionsDialogPart::Note);

            auto addButton = [&](ECS::OptionsDialogPart part, float offsetX) {
                const hlslpp::float2 center(ui.menuCenter.x + offsetX, bottom - ui.confirmButtonOffsetY);
                Tsukino::ECS::Entity button = createPanel(center, ui.confirmButtonSize, ui.confirmNoColor, 42);
                registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
                addPart(button, part);
                Tsukino::ECS::Entity label = addPart(createText(center, ui.optionsButtonLabelScale, kCenter, kWhite, 43, true, inside(ui.confirmButtonSize.x)), ECS::OptionsDialogPart::ButtonLabel);
                registry.GetComponent<ECS::OptionsDialogComponent>(button).label = label;
            };
            addButton(ECS::OptionsDialogPart::Yes, ui.confirmYesOffsetX);
            addButton(ECS::OptionsDialogPart::No, ui.confirmNoOffsetX);
        }
    }

}    // namespace FruitMagic

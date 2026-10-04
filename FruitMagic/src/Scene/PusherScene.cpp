//----------------------------------------------------------------------------
//! @file   PusherScene.cpp
//! @brief  コインプッシャー台のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/PusherScene.hpp>

#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/ManaConfig.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/OfflineReward.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>
#include <FruitMagic/Game/SaveData.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>
#include <FruitMagic/ECS/Component/CheckerComponent.hpp>
#include <FruitMagic/ECS/Component/CoinLauncherComponent.hpp>
#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Component/ManaGaugeComponent.hpp>
#include <FruitMagic/ECS/Component/MenuComponent.hpp>
#include <FruitMagic/ECS/Component/UpgradeElementComponent.hpp>
#include <FruitMagic/ECS/Component/ZukanElementComponent.hpp>
#include <FruitMagic/ECS/System/CheckerSystem.hpp>
#include <FruitMagic/ECS/System/CoinLauncherSystem.hpp>
#include <FruitMagic/ECS/System/FairySystem.hpp>
#include <FruitMagic/ECS/System/HarvestSystem.hpp>
#include <FruitMagic/ECS/System/HudSystem.hpp>
#include <FruitMagic/ECS/System/MagicInputSystem.hpp>
#include <FruitMagic/ECS/System/ManaSystem.hpp>
#include <FruitMagic/ECS/System/MenuSystem.hpp>
#include <FruitMagic/ECS/System/ShakeMagicSystem.hpp>
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
#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/PhysicsSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/LightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/InteractionSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>
#ifdef _DEBUG
#include <Tsukino/EngineIntegration/ECS/System/DebugCameraSystem.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraTag.hpp>
#endif

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DirectionalLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SkyAtmosphereComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Core/Log.hpp>

#include <entt/entt.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //! @brief 物理に渡す1フレームの経過時間の上限（秒）。重いフレームで一気に進めてすり抜けるのを防ぐ
        constexpr float kMaxSimulationStep = 1.0f / 30.0f;

        //--------------------------------------------------------------
        // 画面の UI の配置（画面ピクセル。画面は 1280 x 720）
        //--------------------------------------------------------------
        constexpr float kWhiteTextureSize  = 8.0f;      // Assets/Textures/White.png の1辺のピクセル数
        constexpr float kManaGaugeLeft     = 28.0f;     // マナゲージの左端
        constexpr float kManaGaugeCenterY  = 100.0f;    // マナゲージの中心の高さ
        constexpr float kManaGaugeWidth    = 220.0f;    // マナゲージの全幅
        constexpr float kManaGaugeHeight   = 14.0f;     // マナゲージの高さ
        constexpr float kMagicButtonWidth  = 200.0f;    // 魔法ボタンの幅
        constexpr float kMagicButtonHeight = 46.0f;     // 魔法ボタンの高さ
        constexpr float kMagicButtonGap    = 16.0f;     // 魔法ボタンの間隔
        constexpr float kMagicButtonY      = 640.0f;    // 魔法ボタンの中心の高さ
        constexpr float kMenuButtonX       = 1170.0f;   // 画面の開閉ボタン（右上）の中心X
        constexpr float kZukanButtonY      = 40.0f;     // 図鑑ボタンの中心Y
        constexpr float kUpgradeButtonY    = 90.0f;     // 強化ボタンの中心Y
        constexpr float kMenuButtonWidth   = 170.0f;    // 画面の開閉ボタンの幅
        constexpr float kMenuButtonHeight  = 42.0f;     // 画面の開閉ボタンの高さ
        constexpr float kMenuCenterX       = 640.0f;    // 画面（図鑑・強化）の中心X
        constexpr float kMenuCenterY       = 340.0f;    // 画面の中心Y
        constexpr float kMenuWidth         = 900.0f;    // 画面の幅
        constexpr float kMenuHeight        = 540.0f;    // 画面の高さ
        constexpr float kZukanMaxRowPitch  = 44.0f;     // 図鑑の行の高さの上限（果物が少ないとき）
        constexpr float kUpgradeMaxRowPitch = 100.0f;   // 強化画面の行の高さの上限（強化が少ないとき）
        constexpr float kBuyButtonWidth    = 150.0f;    // 購入ボタンの幅
        constexpr float kBuyButtonHeight   = 46.0f;     // 購入ボタンの高さ
        constexpr float kWelcomeWidth      = 720.0f;    // 「おかえり」画面の幅
        constexpr float kWelcomeHeight     = 300.0f;    // 「おかえり」画面の高さ

        //! @brief プッシャーの振幅を強化の値へ近づける速さ（cm/秒）。一気に変えると速度が跳ねて景品を弾き飛ばすため
        constexpr float kPusherAmplitudeChangeSpeed = 4.0f;

        //! @brief プッシャーの半サイズ
        hlslpp::float3 PusherHalfExtent() {
            return hlslpp::float3(Layout::kPusherHalfWidth, Layout::kPusherHalfHeight, Layout::kPusherHalfDepth);
        }

        //! @brief 押していないときのプッシャーの中心Z（起動時の位置）
        constexpr float kPusherStartCenterZ = Layout::PusherCenterZ(Layout::kPusherAmplitude);
    }    // namespace

    //----------------------------------------------------------------------------
    //! シーン固有の初期化処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) {
        Tsukino::EngineIntegration::EngineContext* context  = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();
        Tsukino::ECS::EventBus&                    eventBus = m_scene.GetEventBus();
        Tsukino::ECS::Registry&                    registry = m_scene.GetRegistry();

        //--------------------------------------------------------------
        // システムの生成と追加
        //--------------------------------------------------------------
        enum class SystemPriority : int {
#ifdef _DEBUG
            DebugResource = -1,
#endif
            Interaction = 0,     // マウスの下の UI（魔法ボタン・画面）を先に決め、同じフレームの入力処理が読めるようにする
            Menu,                // 画面の開閉と表示切替。中身は続く Zukan・Upgrade が書く
            Zukan,
            Upgrade,
            MagicInput,
            CoinLauncher,        // 投入したコインを今フレームの Transform・物理に乗せる
            Fairy,
            Transform,
            Physics,             // Transform 確定後に剛体を進め、結果を Transform へ書き戻す
            PrizeDrop,           // 物理の結果で落下を判定する
            Checker,             // 払い出し口に落ちたコインがチェッカーに入ったかを、同じフレームの落下イベントで判定する
            Wallet,
            Harvest,
            Mana,
            Roulette,            // 当たりで果物を生成する（次のフレームの Transform・物理に乗る）
            ShakeMagic,          // 衝撃の要求を付ける（次のフレームの Physics で反映）。カメラも揺らす
            Hud,
            Save,
            Light,
            SkyAtmosphere,
#ifdef _DEBUG
            DebugCamera,
#endif
            Camera,
            Font,
            Sprite,
            Render,
        };

#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<ECS::DebugResourceSystem>(), (int)SystemPriority::DebugResource);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::InteractionSystem>(), (int)SystemPriority::Interaction);
        m_scene.AddSystem(std::make_shared<ECS::MenuSystem>(), (int)SystemPriority::Menu);
        m_scene.AddSystem(std::make_shared<ECS::ZukanSystem>(), (int)SystemPriority::Zukan);
        m_scene.AddSystem(std::make_shared<ECS::UpgradeSystem>(), (int)SystemPriority::Upgrade);
        m_scene.AddSystem(std::make_shared<ECS::MagicInputSystem>(eventBus), (int)SystemPriority::MagicInput);
        m_scene.AddSystem(std::make_shared<ECS::CoinLauncherSystem>(), (int)SystemPriority::CoinLauncher);
        m_scene.AddSystem(std::make_shared<ECS::FairySystem>(), (int)SystemPriority::Fairy);
        m_scene.AddSystem(std::make_shared<ECS::CheckerSystem>(eventBus), (int)SystemPriority::Checker);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), (int)SystemPriority::Transform);
        auto physicsSystem = std::make_shared<Tsukino::BuiltIn::ECS::PhysicsSystem>(eventBus);
        // このゲームは 1unit=1cm。物理エンジンの重力・接触の許容値をcmに合わせる
        physicsSystem->SetUnitsPerMeter(100.0f);
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
        m_scene.AddSystem(std::make_shared<ECS::HudSystem>(eventBus), (int)SystemPriority::Hud);
        m_scene.AddSystem(std::make_shared<ECS::SaveSystem>(), (int)SystemPriority::Save);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::LightSystem>(), (int)SystemPriority::Light);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SkyAtmosphereSystem>(), (int)SystemPriority::SkyAtmosphere);
#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DebugCameraSystem>(), (int)SystemPriority::DebugCamera);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), (int)SystemPriority::Camera);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), (int)SystemPriority::Font);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), (int)SystemPriority::Sprite);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ModelSystem>(), (int)SystemPriority::Render);

        //--------------------------------------------------------------
        // ゲーム全体で共有するデータをレジストリのコンテキストへ置く
        //--------------------------------------------------------------
        // 果物とルーレットの定義データ（Debug は作業ディレクトリ、Release は exe の隣が基準）
        const std::string dataRoot = (Tsukino::IO::FileSystem::GetAssetRootPath() / "Assets/Data").string();

        FruitCatalog& catalog = registry.SetContext<FruitCatalog>();
        if(!catalog.Load(dataRoot)) {
            Tsukino::Core::Log::Error("PusherScene: no fruit data could be loaded from " + dataRoot + ". Fruits will not appear.");
        }
        registry.SetContext<RouletteConfig>().Load(dataRoot + "/Roulette.json");
        registry.SetContext<RouletteState>();
        ManaConfig& manaConfig = registry.SetContext<ManaConfig>();
        manaConfig.Load(dataRoot + "/Mana.json");
        registry.SetContext<MagicCatalog>().Load(dataRoot + "/Magic.json");
        registry.SetContext<MagicState>();
        CollectionConfig& collection = registry.SetContext<CollectionConfig>();
        collection.Load(dataRoot + "/Collection.json");
        MenuState& menu = registry.SetContext<MenuState>();
        registry.SetContext<UpgradeCatalog>().Load(dataRoot + "/Upgrades.json");
        registry.SetContext<TableStats>();
        OfflineConfig& offline = registry.SetContext<OfflineConfig>();
        offline.Load(dataRoot + "/Offline.json");
        offline.savePath = SaveData::DefaultPath();

        GameState& state = registry.SetContext<GameState>();
        state.maxMana    = manaConfig.maxMana;
        state.harvestCounts.assign(catalog.Fruits().size(), std::vector<int>(collection.Variants().size(), 0));

        //--------------------------------------------------------------
        // セーブを読み、強化を台の性能と果樹の段階へ反映してから、閉じていた間の報酬を受け取る
        // （報酬の計算に強化の値を使うため、この順番）
        //--------------------------------------------------------------
        long long  savedAt = 0;
        const bool loaded  = SaveData::Load(registry, offline.savePath, savedAt);
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
        factory.Initialize(*context->assetManager);

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
        // PhysicsSystem は1フレーム1ステップなので、起動直後のアセット読み込み等で
        // フレームが重くなったときに大きく進めないよう上限を設ける（その分ゲーム内時間は遅れる）。
        // プッシャーの移動にも同じ時間を使い、物理の1ステップで動く距離と食い違わないようにする
        const float simulationStep = std::min(deltaTime, kMaxSimulationStep);

        //--------------------------------------------------------------
        // プッシャーを前後に往復させる
        // m_scene.Update() より前に書き込むことで、同じフレームの PhysicsSystem が
        // 位置の差分から速度を求め、乗っている景品を押す
        //--------------------------------------------------------------
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        if(m_pusherEntity != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity)) {
            m_pusherTime = std::fmod(m_pusherTime + simulationStep, Layout::kPusherPeriod);

            // 押し幅の強化は、振幅を少しずつ近づけて反映する
            if(registry.HasContext<TableStats>()) {
                const float target = registry.GetContext<TableStats>().pusherAmplitude;
                const float step   = kPusherAmplitudeChangeSpeed * simulationStep;
                m_pusherAmplitude  = std::clamp(target, m_pusherAmplitude - step, m_pusherAmplitude + step);
            }

            // 最も引っ込んだ位置は振幅によらず同じで、振幅が増えた分だけ前に出る
            const float phase = 2.0f * kPi * m_pusherTime / Layout::kPusherPeriod;

            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity);
            t.position = hlslpp::float3(0.0f, Layout::kPusherCenterY, Layout::PusherCenterZ(m_pusherAmplitude) + std::sin(phase) * m_pusherAmplitude);
            t.dirty    = true;
        }

        m_scene.Update(simulationStep);
    }

    //----------------------------------------------------------------------------
    //! シーンの終了処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnExit() {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        if(registry.HasContext<OfflineConfig>() && SaveData::Save(registry, registry.GetContext<OfflineConfig>().savePath))
            Tsukino::Core::Log::Info("PusherScene: saved.");
    }

    //----------------------------------------------------------------------------
    //! 筐体（床・壁・背面パネル・景品受け）とプッシャーを生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateCabinet(PrizeFactory& factory) {
        using Tsukino::BuiltIn::ECS::RigidbodyType;
        using namespace Layout;

        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();

        // プレイフィールドの床（上面が y = 0）。
        // 物理は離散ステップなので、フレームが重いと薄い床は景品がすり抜ける。
        // 見た目より厚めにして、1ステップで突き抜けないようにする
        const float fieldHalfDepth = (kFieldFrontZ - kFieldBackZ) * 0.5f;
        const float fieldCenterZ   = kFieldBackZ + fieldHalfDepth;
        factory.CreateBox(registry, hlslpp::float3(0.0f, -kStaticThickness, fieldCenterZ),
                          hlslpp::float3(kFieldHalfWidth, kStaticThickness, fieldHalfDepth), RigidbodyType::Static);

        // 左右の側壁（景品が横からこぼれないように、床から少し上まで）
        for(float side : {-1.0f, 1.0f}) {
            factory.CreateBox(registry, hlslpp::float3(side * (kFieldHalfWidth + 1.0f), 12.0f - kStaticThickness, fieldCenterZ),
                              hlslpp::float3(1.0f, 12.0f + kStaticThickness, fieldHalfDepth), RigidbodyType::Static);
        }

        // 背面パネル。プッシャー上面のすぐ上に置き、プッシャーが引っ込むときに
        // 上面に乗った景品を手前へ掻き落とす（実機のプッシャーと同じ仕組み）。
        // 隙間は作らず、パネルの下端をプッシャーの中へ少し食い込ませる（静的な物とKinematicは衝突しない）。
        // 隙間があると、引っ込むプッシャーに挟まれたコインが押し込まれて下をくぐり、奥へ運ばれてしまう
        const float pusherTop = kPusherTopY;
        const float panelGap  = -3.0f;    // 積み重なったコインに押されて少しめり込んだ物も止められる深さ
        factory.CreateBox(registry, hlslpp::float3(0.0f, pusherTop + panelGap + 15.0f, kBackPanelZ),
                          hlslpp::float3(kFieldHalfWidth, 15.0f, kBackPanelHalfThickness), RigidbodyType::Static);

        //--------------------------------------------------------------
        // 景品受け。中央が払い出し口、その左右が溝（一段低くして区別する）
        // 判定は PrizeDropSystem が落ちた位置で行うので、ここは見た目と受け止め用
        //--------------------------------------------------------------
        const float trayCenterZ   = kFieldFrontZ + 15.0f;
        const float trayHalfDepth = 18.0f;
        factory.CreateBox(registry, hlslpp::float3(0.0f, kTrayTopY - kStaticThickness, trayCenterZ),
                          hlslpp::float3(kPayoutHalfWidth, kStaticThickness, trayHalfDepth), RigidbodyType::Static);

        const float gutterHalfWidth = (kFieldHalfWidth + 5.0f - kPayoutHalfWidth) * 0.5f;
        for(float side : {-1.0f, 1.0f}) {
            // 溝の底（払い出し口より 6cm 低い）
            factory.CreateBox(registry, hlslpp::float3(side * (kPayoutHalfWidth + gutterHalfWidth), kTrayTopY - 6.0f - kStaticThickness, trayCenterZ),
                              hlslpp::float3(gutterHalfWidth, kStaticThickness, trayHalfDepth), RigidbodyType::Static);

            // 払い出し口と溝の仕切り
            factory.CreateBox(registry, hlslpp::float3(side * kPayoutHalfWidth, kTrayTopY + 2.0f, trayCenterZ),
                              hlslpp::float3(0.5f, 2.0f, trayHalfDepth), RigidbodyType::Static);
        }

        //--------------------------------------------------------------
        // プッシャー（Kinematic。OnUpdate で位置を直接動かす）
        //--------------------------------------------------------------
        m_pusherEntity = factory.CreateBox(registry, hlslpp::float3(0.0f, kPusherCenterY, kPusherStartCenterZ), PusherHalfExtent(),
                                           RigidbodyType::Kinematic);
    }

    //----------------------------------------------------------------------------
    //! 起動時に台に置いておく景品を生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateInitialPrizes(PrizeFactory& factory) {
        using namespace Layout;

        Tsukino::ECS::Registry& registry   = m_scene.GetRegistry();
        const hlslpp::float3    coinHalf   = PrizeFactory::CoinHalfExtent();
        const float             coinHalfX  = coinHalf.x;
        const float             coinHalfY  = coinHalf.y;

        // プッシャー前の床にコインを手前端まで敷き詰める（実機と同じく、押せばすぐ縁から落ちる状態にしておく）。
        // 最前列はプッシャーが届く位置から
        const float pusherFrontMax = kPusherMinFrontZ + kPusherAmplitude * 2.0f;
        // 隙間があると押した分が隙間に吸われて縁まで伝わらないので、ほぼ接するくらいに詰める
        const float firstRowZ      = pusherFrontMax - 5.0f;
        const float pitch          = coinHalfX * 2.0f + 0.3f;
        const int   rowCount       = static_cast<int>((kFieldFrontZ - coinHalfX - firstRowZ) / pitch) + 1;
        const int   halfColCount   = static_cast<int>((kFieldHalfWidth - coinHalfX - 0.5f) / pitch);
        for(int row = 0; row < rowCount; ++row) {
            for(int col = -halfColCount; col <= halfColCount; ++col) {
                const float x = static_cast<float>(col) * pitch;
                const float z = firstRowZ + static_cast<float>(row) * pitch;
                factory.CreateCoin(registry, hlslpp::float3(x, coinHalfY + 0.5f, z));
            }
        }

        // プッシャーの上にもコインを数枚
        for(int col = -3; col <= 3; ++col) {
            const float x = static_cast<float>(col) * 7.0f;
            factory.CreateCoin(registry, hlslpp::float3(x, kPusherTopY + coinHalfY + 0.5f, kPusherStartCenterZ + 8.0f));
        }

        //--------------------------------------------------------------
        // 最初から台にある果物。今の果樹の段階で出る果物から選び、敷き詰めたコインのすぐ上に置く
        // （高い所から落とすと下のコインを床へめり込ませてしまう）
        //--------------------------------------------------------------
        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        const int           level   = registry.GetContext<GameState>().treeLevel;
        std::mt19937        rng(std::random_device{}());

        const float fruitXs[] = {-16.0f, -6.0f, 6.0f, 16.0f};
        for(int i = 0; i < 4; ++i) {
            const int fruitIndex = catalog.PickSpawnable(level, rng);
            if(fruitIndex < 0)
                break;

            const FruitDef& def = catalog.Fruits()[fruitIndex];
            const float     y   = coinHalfY * 2.0f + 0.5f + def.HalfHeightOfBounds() + 0.5f;
            const VariantDef& normal = registry.GetContext<CollectionConfig>().Variants()[0];
            factory.CreateFruit(registry, def, fruitIndex, 0, normal.ColorOf(def), normal.glow, hlslpp::float3(fruitXs[i], y, static_cast<float>(i % 2) * 10.0f));
        }
    }

    //----------------------------------------------------------------------------
    //! ライト・空・カメラを生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateEnvironment() {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();

        {
            // ディレクショナルライト（影付き）
            Tsukino::ECS::Entity                              e     = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::DirectionalLightComponent& light = registry.AddComponent<Tsukino::BuiltIn::ECS::DirectionalLightComponent>(e);
            light.direction                                         = hlslpp::float3(-0.3f, -1.0f, -0.4f);
            light.color                                             = hlslpp::float3(1.0f, 0.97f, 0.9f);
            light.intensity                                         = 1.5f;
            light.castShadow                                        = true;
        }
        {
            // 筐体の上の点光源（減衰は intensity / (d^2 + 1) なので、距離60前後で効くように d^2 のオーダーにする）
            Tsukino::ECS::Entity                       e         = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            transform.position                                   = hlslpp::float3(0.0f, 60.0f, 0.0f);
            transform.dirty                                      = true;

            Tsukino::BuiltIn::ECS::PointLightComponent& light = registry.AddComponent<Tsukino::BuiltIn::ECS::PointLightComponent>(e);
            light.color                                      = hlslpp::float3(1.0f, 0.85f, 0.6f);
            light.intensity                                  = 6000.0f;
            light.range                                      = 200.0f;
            light.enabled                                    = true;
        }
        {
            // 大気散乱（空）
            Tsukino::ECS::Entity e = m_scene.CreateEntity();
            registry.AddComponent<Tsukino::BuiltIn::ECS::SkyAtmosphereComponent>(e);
        }
        {
            // カメラ（プレイヤーの目線：手前斜め上から台を見下ろす）
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(0.0f, 75.0f, 105.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& cam = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            cam.useLookAt                               = true;
            cam.lookAtTarget                            = hlslpp::float3(0.0f, 0.0f, -5.0f);
            cam.nearZ                                   = 1.0f;
            cam.farZ                                    = 5000.0f;
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
            camera.orthoSize                               = 720.0f;
            camera.isPrimary                               = false;
        }

#ifdef _DEBUG
        {
            // デバッグカメラ（Debug ビルドのみ。切り替えは DebugCameraSystem の操作に従う）
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(60.0f, 60.0f, 80.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& cam = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            cam.lookAtTarget                            = hlslpp::float3(0.0f, 0.0f, 0.0f);
            cam.nearZ                                   = 1.0f;
            cam.farZ                                    = 5000.0f;
            cam.isPrimary                               = false;

            Tsukino::BuiltIn::ECS::DebugCameraComponent& debug = registry.AddComponent<Tsukino::BuiltIn::ECS::DebugCameraComponent>(e);
            debug.moveSpeed                                    = 100.0f;
            debug.sprintSpeed                                  = 300.0f;

            registry.AddComponent<Tsukino::BuiltIn::ECS::DebugCameraTag>(e);
        }
#endif
    }

    //----------------------------------------------------------------------------
    //! コインの投入口と HUD を生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreatePlayerInterface(PrizeFactory& factory) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();

        //--------------------------------------------------------------
        // 投入口の目印（半透明の板。CoinLauncherSystem が投入位置へ動かす）
        //--------------------------------------------------------------
        {
            const hlslpp::float3 coinHalf = PrizeFactory::CoinHalfExtent();
            Tsukino::ECS::Entity e        = factory.CreateVisualBox(registry, hlslpp::float3(0.0f, Layout::kLaunchMarkerY, Layout::kLaunchZ),
                                                                    hlslpp::float3(coinHalf.x, 0.2f, coinHalf.z), 0.5f);
            registry.AddComponent<ECS::CoinLauncherComponent>(e);
        }

        //--------------------------------------------------------------
        // チェッカー（左右に動く穴）の目印。台の手前端のすぐ下、コインが最終的に落ちる払い出し口にあり、
        // CheckerSystem が左右に動かす。判定は落ちたコインの位置で行うので、これは見た目だけ（コライダー無し）
        //--------------------------------------------------------------
        {
            const float          halfWidth = registry.GetContext<TableStats>().checkerHalfWidth;
            const hlslpp::float3 gold      = hlslpp::float3(1.0f, 0.7f, 0.1f);
            Tsukino::ECS::Entity e         = factory.CreateVisualBox(registry, hlslpp::float3(0.0f, -3.0f, Layout::kFieldFrontZ + 3.0f),
                                                                     hlslpp::float3(halfWidth, 0.3f, 2.5f), 1.0f, gold);

            // 台の上で見失わないよう、金色に少し光らせる
            Tsukino::BuiltIn::ECS::RimGlowComponent& glow = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(e);
            glow.active                                   = true;
            glow.rimColor                                 = gold;
            glow.rimIntensity                             = 1.0f;
            glow.glow                                     = 0.4f;

            // 穴の幅の強化で目印も伸ばせるよう、幅あたりのスケールを覚えておく
            ECS::CheckerComponent& checker = registry.AddComponent<ECS::CheckerComponent>(e);
            checker.scalePerHalfWidth      = float(registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e).scale.x) / halfWidth;
        }

        //--------------------------------------------------------------
        // HUD（座標は画面左上からのピクセル。文字の大きさは scale.x）
        //--------------------------------------------------------------
        struct HudSpec {
            ECS::HudTextKind                       kind;
            hlslpp::float2                         position;
            float                                  scale;
            hlslpp::float4                         color;
            Tsukino::BuiltIn::ECS::HorizontalAlign align = Tsukino::BuiltIn::ECS::HorizontalAlign::Left;
        };
        const HudSpec specs[] = {
            {ECS::HudTextKind::Coins, hlslpp::float2(24.0f, 20.0f), 1.6f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f)},
            {ECS::HudTextKind::Mana, hlslpp::float2(kManaGaugeLeft + kManaGaugeWidth + 12.0f, kManaGaugeCenterY - 14.0f), 0.9f,
             hlslpp::float4(0.85f, 0.7f, 1.0f, 1.0f)},
            {ECS::HudTextKind::DropPopup, hlslpp::float2(28.0f, 122.0f), 1.2f, hlslpp::float4(0.6f, 1.0f, 0.6f, 1.0f)},
            {ECS::HudTextKind::HarvestTotal, hlslpp::float2(24.0f, 162.0f), 1.2f, hlslpp::float4(1.0f, 0.6f, 0.7f, 1.0f)},
            {ECS::HudTextKind::HarvestPopup, hlslpp::float2(28.0f, 204.0f), 1.2f, hlslpp::float4(1.0f, 0.85f, 0.9f, 1.0f)},
            {ECS::HudTextKind::Roulette, hlslpp::float2(640.0f, 24.0f), 1.3f, hlslpp::float4(1.0f, 0.95f, 0.6f, 1.0f),
             Tsukino::BuiltIn::ECS::HorizontalAlign::Center},
            {ECS::HudTextKind::ControlsHint, hlslpp::float2(24.0f, 690.0f), 0.65f, hlslpp::float4(1.0f, 1.0f, 1.0f, 0.85f)},
        };

        for(const HudSpec& spec : specs) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(spec.position.x, spec.position.y, 0.0f);
            t.scale                                      = hlslpp::float3(spec.scale, spec.scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.color                                 = spec.color;
            font.horizontalAlign                       = spec.align;
            font.outlineColor                          = hlslpp::float4(0.15f, 0.08f, 0.05f, 1.0f);
            font.outlineWidth                          = 2.0f;

            ECS::HudTextComponent& hud = registry.AddComponent<ECS::HudTextComponent>(e);
            hud.kind                   = spec.kind;
        }

        //--------------------------------------------------------------
        // 画面スプライト（白い小さなテクスチャを色付けして使い回す。位置は中心）
        //--------------------------------------------------------------
        Tsukino::EngineIntegration::EngineContext* context = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        const Tsukino::Asset::AssetHandle          white   = context->assetManager->Load(Tsukino::Core::Path("Assets/Textures/White.png"));

        auto createPanel = [&](const hlslpp::float2& center, const hlslpp::float2& size, const hlslpp::float4& color, int sortOrder) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(center.x, center.y, 0.0f);
            t.scale                                      = hlslpp::float3(size.x / kWhiteTextureSize, size.y / kWhiteTextureSize, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e);
            sprite.textureHandle                           = white;
            sprite.tintColor                               = color;
            sprite.sortOrder                               = sortOrder;
            return e;
        };

        //--------------------------------------------------------------
        // マナゲージ（背景＋中身。中身の幅は HudSystem がマナに合わせて変える）
        //--------------------------------------------------------------
        createPanel(hlslpp::float2(kManaGaugeLeft + kManaGaugeWidth * 0.5f, kManaGaugeCenterY), hlslpp::float2(kManaGaugeWidth + 4.0f, kManaGaugeHeight + 4.0f),
                    hlslpp::float4(0.1f, 0.05f, 0.15f, 0.8f), 0);
        {
            Tsukino::ECS::Entity     e     = createPanel(hlslpp::float2(kManaGaugeLeft, kManaGaugeCenterY), hlslpp::float2(0.0f, kManaGaugeHeight),
                                                         hlslpp::float4(0.75f, 0.45f, 1.0f, 1.0f), 1);
            ECS::ManaGaugeComponent& gauge = registry.AddComponent<ECS::ManaGaugeComponent>(e);
            gauge.left                     = kManaGaugeLeft;
            gauge.fullWidth                = kManaGaugeWidth;
            gauge.height                   = kManaGaugeHeight;
            gauge.textureSize              = kWhiteTextureSize;
        }

        //--------------------------------------------------------------
        // 魔法ボタン（画面下に5つ。色と文字は HudSystem、クリックは MagicInputSystem が扱う）
        //--------------------------------------------------------------
        const float totalWidth = kMagicButtonWidth * kMagicSlotCount + kMagicButtonGap * (kMagicSlotCount - 1);
        const float firstX     = 640.0f - totalWidth * 0.5f + kMagicButtonWidth * 0.5f;
        for(int slot = 1; slot <= kMagicSlotCount; ++slot) {
            const hlslpp::float2 center(firstX + (kMagicButtonWidth + kMagicButtonGap) * static_cast<float>(slot - 1), kMagicButtonY);

            Tsukino::ECS::Entity button = createPanel(center, hlslpp::float2(kMagicButtonWidth, kMagicButtonHeight), hlslpp::float4(0.4f, 0.4f, 0.4f, 1.0f), 10);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);

            // ボタンの上の文字（ボタンの中心に揃える）
            Tsukino::ECS::Entity                       label = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t     = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(label);
            t.position                                       = hlslpp::float3(center.x, center.y, 0.0f);
            t.scale                                          = hlslpp::float3(0.85f, 0.85f, 1.0f);
            t.dirty                                          = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(label);
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = hlslpp::float4(0.15f, 0.05f, 0.2f, 1.0f);
            font.outlineWidth                          = 2.0f;
            font.sortOrder                             = 11;

            ECS::MagicButtonComponent& magicButton = registry.AddComponent<ECS::MagicButtonComponent>(button);
            magicButton.slot                       = slot;
            magicButton.label                      = label;
        }

        //--------------------------------------------------------------
        // 文字のエンティティを作る（図鑑とボタンで共通）
        //--------------------------------------------------------------
        auto createText = [&](const hlslpp::float2& position, float scale, Tsukino::BuiltIn::ECS::HorizontalAlign align, const hlslpp::float4& color, int sortOrder) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(position.x, position.y, 0.0f);
            t.scale                                      = hlslpp::float3(scale, scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.color                                 = color;
            font.horizontalAlign                       = align;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = hlslpp::float4(0.1f, 0.03f, 0.15f, 1.0f);
            font.outlineWidth                          = 2.0f;
            font.sortOrder                             = sortOrder;
            return e;
        };

        //--------------------------------------------------------------
        // 画面（図鑑・強化）の開閉ボタン（右上。文字は MenuSystem が開閉に合わせて書く）
        //--------------------------------------------------------------
        struct MenuButtonSpec {
            MenuKind                menu;
            Tsukino::Input::KeyCode key;
            float                   y;
            hlslpp::float4          color;
            const wchar_t*          closedText;
            const wchar_t*          openText;
        };
        const MenuButtonSpec menuButtons[] = {
            {MenuKind::Zukan, Tsukino::Input::KeyCode::Tab, kZukanButtonY, hlslpp::float4(0.95f, 0.55f, 0.65f, 1.0f), L"図鑑 (Tab)", L"閉じる (Tab)"},
            {MenuKind::Upgrade, Tsukino::Input::KeyCode::U, kUpgradeButtonY, hlslpp::float4(0.45f, 0.75f, 0.4f, 1.0f), L"強化 (U)", L"閉じる (U)"},
        };
        for(const MenuButtonSpec& spec : menuButtons) {
            Tsukino::ECS::Entity button = createPanel(hlslpp::float2(kMenuButtonX, spec.y), hlslpp::float2(kMenuButtonWidth, kMenuButtonHeight), spec.color, 10);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);

            ECS::MenuButtonComponent& menuButton = registry.AddComponent<ECS::MenuButtonComponent>(button);
            menuButton.menu                      = spec.menu;
            menuButton.key                       = spec.key;
            menuButton.label      = createText(hlslpp::float2(kMenuButtonX, spec.y), 0.85f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                                               hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), 11);
            menuButton.closedText = spec.closedText;
            menuButton.openText   = spec.openText;
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

        // 画面の背景のパネル。画面の上のクリックでコインが投入されないよう、クリックを受け止める
        auto createMenuPanel = [&](MenuKind menu, const std::wstring& title) {
            Tsukino::ECS::Entity panel = createPanel(hlslpp::float2(kMenuCenterX, kMenuCenterY), hlslpp::float2(kMenuWidth, kMenuHeight),
                                                     hlslpp::float4(0.08f, 0.04f, 0.12f, 0.9f), 20);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(panel);
            addPage(panel, menu);

            addPage(createText(hlslpp::float2(kMenuCenterX, kMenuCenterY - kMenuHeight * 0.5f + 32.0f), 1.3f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                               hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f), 22),
                    menu, title);
        };

        //--------------------------------------------------------------
        // 図鑑の画面（行＝果物、列＝バリエーション）。最初は閉じていて、ZukanSystem が開閉する。
        // 果物・バリエーションの数は定義データから決まるので、行の高さ・列の間隔もそれに合わせる
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

            createMenuPanel(MenuKind::Zukan, L"図鑑");

            const float top    = kMenuCenterY - kMenuHeight * 0.5f;
            const float left   = kMenuCenterX - kMenuWidth * 0.5f;
            const float bottom = kMenuCenterY + kMenuHeight * 0.5f;

            // 列の配置（バリエーションが増えたら間隔を詰める）
            const float columnsLeft  = left + 330.0f;
            const float columnsWidth = kMenuWidth - 330.0f - 40.0f;
            const float columnPitch  = columnsWidth / static_cast<float>(std::max(1, columns));
            auto        columnX      = [&](int v) { return columnsLeft + columnPitch * (static_cast<float>(v) + 0.5f); };

            // 列の見出し
            const float headerY = top + 78.0f;
            for(int v = 0; v < columns; ++v) {
                const std::wstring& name = collection.Variants()[v].name;
                addPage(createText(hlslpp::float2(columnX(v), headerY), 0.85f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                                   hlslpp::float4(0.9f, 0.8f, 1.0f, 1.0f), 22),
                        MenuKind::Zukan, name.empty() ? std::wstring(L"通常") : name);
            }

            // 行（果物が増えたら行の高さを詰める）
            const float rowsTop    = headerY + 30.0f;
            const float rowsHeight = (bottom - 56.0f) - rowsTop;
            const float rowPitch   = std::min(kZukanMaxRowPitch, rowsHeight / static_cast<float>(std::max(1, rows)));
            const float swatchSize = std::min(26.0f, rowPitch - 6.0f);
            const float textScale  = std::min(0.85f, rowPitch / 46.0f);
            for(int f = 0; f < rows; ++f) {
                const float y = rowsTop + rowPitch * (static_cast<float>(f) + 0.5f);

                addElement(createText(hlslpp::float2(left + 36.0f, y), textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                      hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), 22),
                           ECS::ZukanElementKind::RowName, f);

                for(int v = 0; v < columns; ++v) {
                    const float x = columnX(v);
                    addElement(createPanel(hlslpp::float2(x - 30.0f, y), hlslpp::float2(swatchSize, swatchSize), hlslpp::float4(0.2f, 0.2f, 0.25f, 1.0f), 21),
                               ECS::ZukanElementKind::Swatch, f, v);
                    addElement(createText(hlslpp::float2(x - 10.0f, y), textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                          hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f), 22),
                               ECS::ZukanElementKind::Count, f, v);
                }
            }

            // 下部の集計
            addElement(createText(hlslpp::float2(kMenuCenterX, bottom - 28.0f), 0.9f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                                  hlslpp::float4(0.85f, 1.0f, 0.85f, 1.0f), 22),
                       ECS::ZukanElementKind::Footer);
        }

        //--------------------------------------------------------------
        // 強化の画面（行＝強化）。最初は閉じている。
        // 強化の数は定義データから決まるので、行の高さもそれに合わせる
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

            createMenuPanel(MenuKind::Upgrade, L"台の強化");

            const float top    = kMenuCenterY - kMenuHeight * 0.5f;
            const float left   = kMenuCenterX - kMenuWidth * 0.5f;
            const float right  = kMenuCenterX + kMenuWidth * 0.5f;
            const float bottom = kMenuCenterY + kMenuHeight * 0.5f;

            // 手持ち（価格と見比べられるよう、タイトルのすぐ下）
            addElement(createText(hlslpp::float2(kMenuCenterX, top + 74.0f), 0.9f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                                  hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), 22),
                       ECS::UpgradeElementKind::Wallet);

            // 行（強化が増えたら行の高さを詰める）
            const float rowsTop    = top + 104.0f;
            const float rowsHeight = (bottom - 24.0f) - rowsTop;
            const float rowPitch   = std::min(kUpgradeMaxRowPitch, rowsHeight / static_cast<float>(std::max(1, rows)));
            const float textScale  = std::min(1.0f, rowPitch / 90.0f);
            const float buttonX    = right - 40.0f - kBuyButtonWidth * 0.5f;
            const float effectX    = left + 445.0f;
            for(int u = 0; u < rows; ++u) {
                const UpgradeDef& def = upgrades.Upgrades()[u];
                const float       y   = rowsTop + rowPitch * (static_cast<float>(u) + 0.5f);
                const float       dy  = rowPitch * 0.2f;    // 1行の中の上段・下段のずれ

                // 名前とレベル（上段）・説明（下段）
                addElement(createText(hlslpp::float2(left + 40.0f, y - dy), 0.9f * textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                      hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), 22),
                           ECS::UpgradeElementKind::Name, u);
                addPage(createText(hlslpp::float2(left + 40.0f, y + dy), 0.62f * textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                   hlslpp::float4(0.8f, 0.75f, 0.9f, 1.0f), 22),
                        MenuKind::Upgrade, def.description);

                // 効果（上段）・価格（下段）
                addElement(createText(hlslpp::float2(effectX, y - dy), 0.75f * textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                      hlslpp::float4(0.7f, 1.0f, 0.75f, 1.0f), 22),
                           ECS::UpgradeElementKind::Effect, u);
                addElement(createText(hlslpp::float2(effectX, y + dy), 0.7f * textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Left,
                                      hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f), 22),
                           ECS::UpgradeElementKind::Cost, u);

                // 購入ボタン（色は UpgradeSystem が買えるかどうかで変える）
                Tsukino::ECS::Entity button = createPanel(hlslpp::float2(buttonX, y), hlslpp::float2(kBuyButtonWidth, std::min(kBuyButtonHeight, rowPitch - 8.0f)),
                                                          hlslpp::float4(0.3f, 0.28f, 0.34f, 1.0f), 21);
                registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
                addElement(button, ECS::UpgradeElementKind::BuyButton, u);
                addElement(createText(hlslpp::float2(buttonX, y), 0.85f * textScale, Tsukino::BuiltIn::ECS::HorizontalAlign::Center,
                                      hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), 22),
                           ECS::UpgradeElementKind::BuyLabel, u);
            }
        }

        //--------------------------------------------------------------
        // 「おかえり」画面（閉じていた間の報酬）。起動時に開いていれば出し、ボタンで閉じる。
        // 中身は起動時に決まるので、すべて決まった文字として作る。ほかの画面より手前に重ねる
        //--------------------------------------------------------------
        {
            const OfflineReport& report = registry.GetContext<OfflineReport>();
            const TableStats&    stats  = registry.GetContext<TableStats>();
            const float          top    = kMenuCenterY - kWelcomeHeight * 0.5f;
            const float          bottom = kMenuCenterY + kWelcomeHeight * 0.5f;

            Tsukino::ECS::Entity panel = createPanel(hlslpp::float2(kMenuCenterX, kMenuCenterY), hlslpp::float2(kWelcomeWidth, kWelcomeHeight),
                                                     hlslpp::float4(0.12f, 0.06f, 0.18f, 0.96f), 30);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(panel);
            addPage(panel, MenuKind::Welcome);

            auto addLine = [&](float y, float scale, const hlslpp::float4& color, const std::wstring& text) {
                addPage(createText(hlslpp::float2(kMenuCenterX, y), scale, Tsukino::BuiltIn::ECS::HorizontalAlign::Center, color, 32), MenuKind::Welcome, text);
            };

            addLine(top + 40.0f, 1.3f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f), L"おかえりなさい！");
            addLine(top + 92.0f, 0.85f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), FormatDuration(report.awaySeconds) + L" のあいだに");
            if(report.hasFairy) {
                std::wstring reward = L"妖精が コイン +" + std::to_wstring(report.coins);
                if(report.fruitCount > 0)
                    reward += L" と 果実 +" + std::to_wstring(report.fruitPoints) + L"（果物 " + std::to_wstring(report.fruitCount) + L"個）";
                addLine(top + 132.0f, 0.85f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), reward + L" を集めてくれました");
            } else {
                addLine(top + 132.0f, 0.75f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f), L"「妖精の自動投入」を強化すると、閉じている間もコインを集めてくれます");
            }
            if(report.capped) {
                const long long limitSeconds = static_cast<long long>(stats.offlineMaxHours * 3600.0f);
                addLine(top + 170.0f, 0.65f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f),
                        L"（おるすばんは " + FormatDuration(limitSeconds) + L" までです。「おるすばん時間」の強化で延ばせます）");
            }

            // 閉じるボタン（Enter でも閉じる。閉じた後は開かない）
            const hlslpp::float2 buttonCenter(kMenuCenterX, bottom - 48.0f);
            Tsukino::ECS::Entity button = createPanel(buttonCenter, hlslpp::float2(200.0f, 50.0f), hlslpp::float4(0.35f, 0.78f, 0.45f, 1.0f), 31);
            registry.AddComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>(button);
            addPage(button, MenuKind::Welcome);

            ECS::MenuButtonComponent& close = registry.AddComponent<ECS::MenuButtonComponent>(button);
            close.menu                      = MenuKind::Welcome;
            close.key                       = Tsukino::Input::KeyCode::Enter;
            close.label      = addPage(createText(buttonCenter, 0.9f, Tsukino::BuiltIn::ECS::HorizontalAlign::Center, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), 32),
                                       MenuKind::Welcome);
            close.openText   = L"受け取る (Enter)";
            close.canOpen    = false;
        }
    }

}    // namespace FruitMagic

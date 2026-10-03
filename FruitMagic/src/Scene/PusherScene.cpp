//----------------------------------------------------------------------------
//! @file   PusherScene.cpp
//! @brief  コインプッシャー台のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/PusherScene.hpp>

#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/ECS/Component/CoinLauncherComponent.hpp>
#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/System/CoinLauncherSystem.hpp>
#include <FruitMagic/ECS/System/PrizeDropSystem.hpp>
#include <FruitMagic/ECS/System/WalletSystem.hpp>
#include <FruitMagic/ECS/System/HudSystem.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/PhysicsSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/LightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#ifdef _DEBUG
#include <Tsukino/EngineIntegration/ECS/System/DebugCameraSystem.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraTag.hpp>
#endif

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DirectionalLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SkyAtmosphereComponent.hpp>

#include <Tsukino/Core/Log.hpp>

#include <entt/entt.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //! @brief 物理に渡す1フレームの経過時間の上限（秒）。重いフレームで一気に進めてすり抜けるのを防ぐ
        constexpr float kMaxSimulationStep = 1.0f / 30.0f;

        //! @brief 果物（仮景品）の半径
        constexpr float kFruitRadius = 4.0f;

        //! @brief プッシャーの半サイズ
        hlslpp::float3 PusherHalfExtent() {
            return hlslpp::float3(Layout::kPusherHalfWidth, Layout::kPusherHalfHeight, Layout::kPusherHalfDepth);
        }
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
            CoinLauncher = 0,    // 投入したコインを今フレームの Transform・物理に乗せるため最初
            Transform,
            Physics,             // Transform 確定後に剛体を進め、結果を Transform へ書き戻す
            PrizeDrop,           // 物理の結果で落下を判定する
            Wallet,
            Hud,
            Light,
            SkyAtmosphere,
#ifdef _DEBUG
            DebugCamera,
#endif
            Camera,
            Font,
            Render,
        };

        m_scene.AddSystem(std::make_shared<ECS::CoinLauncherSystem>(), (int)SystemPriority::CoinLauncher);
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
        m_scene.AddSystem(std::make_shared<ECS::HudSystem>(eventBus), (int)SystemPriority::Hud);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::LightSystem>(), (int)SystemPriority::Light);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SkyAtmosphereSystem>(), (int)SystemPriority::SkyAtmosphere);
#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DebugCameraSystem>(), (int)SystemPriority::DebugCamera);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), (int)SystemPriority::Camera);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), (int)SystemPriority::Font);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ModelSystem>(), (int)SystemPriority::Render);

        //--------------------------------------------------------------
        // ゲーム全体で共有するデータをレジストリのコンテキストへ置く
        //--------------------------------------------------------------
        registry.SetContext<GameState>();
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

            const float phase = 2.0f * kPi * m_pusherTime / Layout::kPusherPeriod;

            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity);
            t.position = hlslpp::float3(0.0f, Layout::kPusherCenterY, Layout::kPusherCenterZ + std::sin(phase) * Layout::kPusherAmplitude);
            t.dirty    = true;
        }

        m_scene.Update(simulationStep);
    }

    //----------------------------------------------------------------------------
    //! シーンの終了処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnExit() {
    }

    //----------------------------------------------------------------------------
    //! 筐体（床・壁・背面パネル・景品受け）とプッシャーを生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateCabinet(const PrizeFactory& factory) {
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
        m_pusherEntity = factory.CreateBox(registry, hlslpp::float3(0.0f, kPusherCenterY, kPusherCenterZ), PusherHalfExtent(),
                                           RigidbodyType::Kinematic);
    }

    //----------------------------------------------------------------------------
    //! 起動時に台に置いておく景品を生成します。
    //----------------------------------------------------------------------------
    void PusherScene::CreateInitialPrizes(const PrizeFactory& factory) {
        using namespace Layout;

        Tsukino::ECS::Registry& registry   = m_scene.GetRegistry();
        const hlslpp::float3    coinHalf   = PrizeFactory::CoinHalfExtent();
        const float             coinHalfX  = coinHalf.x;
        const float             coinHalfY  = coinHalf.y;

        // プッシャー前の床にコインを手前端まで敷き詰める（実機と同じく、押せばすぐ縁から落ちる状態にしておく）。
        // 最前列はプッシャーが届く位置から
        const float pusherFrontMax = kPusherCenterZ + kPusherHalfDepth + kPusherAmplitude;
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
            factory.CreateCoin(registry, hlslpp::float3(x, kPusherTopY + coinHalfY + 0.5f, kPusherCenterZ + 8.0f));
        }

        // 果物の代わりの球を、敷き詰めたコインのすぐ上に置く
        // （高い所から落とすと下のコインを床へめり込ませてしまう）
        const float fruitXs[] = {-16.0f, -6.0f, 6.0f, 16.0f};
        const float fruitY    = coinHalfY * 2.0f + 0.5f + kFruitRadius + 0.5f;
        for(int i = 0; i < 4; ++i) {
            factory.CreateFruit(registry, hlslpp::float3(fruitXs[i], fruitY, static_cast<float>(i % 2) * 10.0f), kFruitRadius);
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
    void PusherScene::CreatePlayerInterface(const PrizeFactory& factory) {
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
        // HUD（座標は画面左上からのピクセル。文字の大きさは scale.x）
        //--------------------------------------------------------------
        struct HudSpec {
            ECS::HudTextKind kind;
            hlslpp::float2   position;
            float            scale;
            hlslpp::float4   color;
        };
        const HudSpec specs[] = {
            {ECS::HudTextKind::Coins, hlslpp::float2(24.0f, 20.0f), 1.6f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f)},
            {ECS::HudTextKind::DropPopup, hlslpp::float2(28.0f, 84.0f), 1.2f, hlslpp::float4(0.6f, 1.0f, 0.6f, 1.0f)},
            {ECS::HudTextKind::ControlsHint, hlslpp::float2(24.0f, 670.0f), 0.8f, hlslpp::float4(1.0f, 1.0f, 1.0f, 0.85f)},
        };

        for(const HudSpec& spec : specs) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(spec.position.x, spec.position.y, 0.0f);
            t.scale                                      = hlslpp::float3(spec.scale, spec.scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.color                                 = spec.color;
            font.outlineColor                          = hlslpp::float4(0.15f, 0.08f, 0.05f, 1.0f);
            font.outlineWidth                          = 2.0f;

            ECS::HudTextComponent& hud = registry.AddComponent<ECS::HudTextComponent>(e);
            hud.kind                   = spec.kind;
        }
    }

}    // namespace FruitMagic

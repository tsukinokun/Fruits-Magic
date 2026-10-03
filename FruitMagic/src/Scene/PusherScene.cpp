//----------------------------------------------------------------------------
//! @file   PusherScene.cpp
//! @brief  コインプッシャー台のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/PusherScene.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/Asset/Model/ModelAsset.hpp>

#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/PhysicsSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/LightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#ifdef _DEBUG
#include <Tsukino/EngineIntegration/ECS/System/DebugCameraSystem.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DebugCameraTag.hpp>
#endif

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CollisionComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DirectionalLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointLightComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SkyAtmosphereComponent.hpp>

#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <entt/entt.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //--------------------------------------------------------------
        // 単位はエンジン規約どおり 1unit ≒ 1cm。
        // 奥が -Z、手前（プレイヤー側・落下口）が +Z
        //--------------------------------------------------------------

        //! @brief 箱モデル（Block.fbx）のスケール1での寸法。中心原点の 100 x 20 x 100
        const hlslpp::float3 kBlockModelSize = hlslpp::float3(100.0f, 20.0f, 100.0f);

        //! @brief プレイフィールドの半幅（X）
        constexpr float kFieldHalfWidth = 30.0f;
        //! @brief プレイフィールド手前端（ここから先へ押し出された景品が落ちる）のZ
        constexpr float kFieldFrontZ = 40.0f;
        //! @brief プレイフィールド奥端のZ（プッシャーが引っ込んでも床が切れない位置）
        constexpr float kFieldBackZ = -80.0f;

        //! @brief プッシャーの半サイズ
        const hlslpp::float3 kPusherHalfExtent = hlslpp::float3(kFieldHalfWidth - 0.5f, 4.0f, 15.0f);
        //! @brief プッシャー中心の往復の中心Z
        constexpr float kPusherCenterZ = -45.0f;
        //! @brief プッシャーの往復の振幅
        constexpr float kPusherAmplitude = 12.0f;
        //! @brief プッシャーの往復の周期（秒）
        constexpr float kPusherPeriod = 3.0f;

        //! @brief 静的な床・トレイのコライダーの半分の厚み（すり抜け対策で厚めにする）
        constexpr float kStaticThickness = 10.0f;

        //! @brief 物理に渡す1フレームの経過時間の上限（秒）。重いフレームで一気に進めてすり抜けるのを防ぐ
        constexpr float kMaxSimulationStep = 1.0f / 30.0f;

        //! @brief エンジン既定の重力は -9.81 unit/s^2 なので、1unit=1cm で実重力にするための倍率
        constexpr float kGravityFactor = 100.0f;

        //! @brief コイン（仮景品）の半サイズ
        const hlslpp::float3 kCoinHalfExtent = hlslpp::float3(2.5f, 0.4f, 2.5f);
        //! @brief 果物（仮景品）の半径
        constexpr float kFruitRadius = 4.0f;

        //--------------------------------------------------------------
        //! 球モデルのスケール1での半径をメッシュのバウンディングボックスから求めます。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] handle       球モデルのハンドル
        //! @return 半径。取得できなかった場合は 1.0f
        //--------------------------------------------------------------
        float MeasureModelRadius(Tsukino::Asset::AssetManager& assetManager, Tsukino::Asset::AssetHandle handle) {
            auto model = std::dynamic_pointer_cast<Tsukino::Asset::ModelAsset>(assetManager.Get(handle));
            if(!model || model->modelData.meshes.empty()) {
                Tsukino::Core::Log::Warn("PusherScene: failed to measure ball model size. Falling back to radius 1.");
                return 1.0f;
            }

            // 全メッシュの AABB を合成
            hlslpp::interop::float3 minPos = model->modelData.meshes[0].bounds.min;
            hlslpp::interop::float3 maxPos = model->modelData.meshes[0].bounds.max;
            for(const auto& mesh : model->modelData.meshes) {
                minPos.x = std::min(minPos.x, mesh.bounds.min.x);
                minPos.y = std::min(minPos.y, mesh.bounds.min.y);
                minPos.z = std::min(minPos.z, mesh.bounds.min.z);
                maxPos.x = std::max(maxPos.x, mesh.bounds.max.x);
                maxPos.y = std::max(maxPos.y, mesh.bounds.max.y);
                maxPos.z = std::max(maxPos.z, mesh.bounds.max.z);
            }

            // 3軸の最大幅の半分を半径とみなす
            const float radius = 0.5f * std::max({maxPos.x - minPos.x, maxPos.y - minPos.y, maxPos.z - minPos.z});

            Tsukino::Core::Log::Info("PusherScene: ball model radius = " + std::to_string(radius));
            return (radius > 0.0f) ? radius : 1.0f;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! シーン固有の初期化処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) {
        Tsukino::EngineIntegration::EngineContext* context = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();
        Tsukino::ECS::EventBus&                    eventBus = m_scene.GetEventBus();
        Tsukino::ECS::Registry&                    registry = m_scene.GetRegistry();

        //--------------------------------------------------------------
        // システムの生成と追加
        //--------------------------------------------------------------
        enum class SystemPriority : int {
            Transform = 0,
            Physics,    // Transform 確定後に剛体を進め、結果を Transform へ書き戻す
            Light,
            SkyAtmosphere,
#ifdef _DEBUG
            DebugCamera,
#endif
            Camera,
            Render,
        };

        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), (int)SystemPriority::Transform);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::PhysicsSystem>(eventBus), (int)SystemPriority::Physics);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::LightSystem>(), (int)SystemPriority::Light);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SkyAtmosphereSystem>(), (int)SystemPriority::SkyAtmosphere);
#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DebugCameraSystem>(), (int)SystemPriority::DebugCamera);
#endif
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), (int)SystemPriority::Camera);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ModelSystem>(), (int)SystemPriority::Render);

        //--------------------------------------------------------------
        // アセットのロード（パスはリポジトリルート相対）
        //--------------------------------------------------------------
        m_blockModel      = context->assetManager->Load(Tsukino::Core::Path("Assets/Models/Block.fbx"));
        m_ballModel       = context->assetManager->Load(Tsukino::Core::Path("Assets/Models/Ball.fbx"));
        m_ballModelRadius = MeasureModelRadius(*context->assetManager, m_ballModel);

        using Tsukino::BuiltIn::ECS::RigidbodyType;

        //--------------------------------------------------------------
        // 筐体
        //--------------------------------------------------------------
        {
            // プレイフィールドの床（上面が y = 0）。
            // 物理は離散ステップなので、フレームが重いと薄い床は景品がすり抜ける。
            // 見た目より厚めにして、1ステップで突き抜けないようにする
            const float fieldHalfDepth = (kFieldFrontZ - kFieldBackZ) * 0.5f;
            CreateBox(hlslpp::float3(0.0f, -kStaticThickness, kFieldBackZ + fieldHalfDepth),
                      hlslpp::float3(kFieldHalfWidth, kStaticThickness, fieldHalfDepth),
                      RigidbodyType::Static);

            // 左右の側壁（景品が横からこぼれないように、床から少し上まで）
            for(float side : {-1.0f, 1.0f}) {
                CreateBox(hlslpp::float3(side * (kFieldHalfWidth + 1.0f), 12.0f - kStaticThickness, kFieldBackZ + fieldHalfDepth),
                          hlslpp::float3(1.0f, 12.0f + kStaticThickness, fieldHalfDepth),
                          RigidbodyType::Static);
            }

            // 背面パネル。プッシャー上面のすぐ上に置き、プッシャーが引っ込むときに
            // 上面に乗った景品を手前へ掻き落とす（実機のプッシャーと同じ仕組み）
            const float pusherTop = kPusherHalfExtent.y * 2.0f;
            CreateBox(hlslpp::float3(0.0f, pusherTop + 1.0f + 15.0f, kPusherCenterZ),
                      hlslpp::float3(kFieldHalfWidth, 15.0f, 1.0f),
                      RigidbodyType::Static);

            // 落下口の下の景品受けトレイ（落ちた景品を受け止める）
            CreateBox(hlslpp::float3(0.0f, -30.0f - kStaticThickness, kFieldFrontZ + 15.0f),
                      hlslpp::float3(kFieldHalfWidth + 5.0f, kStaticThickness, 18.0f),
                      RigidbodyType::Static);
        }

        //--------------------------------------------------------------
        // プッシャー（Kinematic。OnUpdate で位置を直接動かす）
        //--------------------------------------------------------------
        m_pusherEntity = CreateBox(hlslpp::float3(0.0f, kPusherHalfExtent.y, kPusherCenterZ), kPusherHalfExtent, RigidbodyType::Kinematic);

        //--------------------------------------------------------------
        // 仮の景品
        //--------------------------------------------------------------
        {
            // プッシャー前の床にコインを敷き詰める（最前列はプッシャーが届く位置から）
            const float pusherFrontMax = kPusherCenterZ + kPusherHalfExtent.z + kPusherAmplitude;
            for(int row = 0; row < 8; ++row) {
                for(int col = -4; col <= 4; ++col) {
                    const float x = static_cast<float>(col) * 6.0f + ((row % 2) ? 3.0f : 0.0f);
                    const float z = pusherFrontMax - 5.0f + static_cast<float>(row) * 6.5f;
                    if(std::abs(x) > kFieldHalfWidth - static_cast<float>(kCoinHalfExtent.x))
                        continue;
                    CreateBox(hlslpp::float3(x, kCoinHalfExtent.y + 0.5f, z), kCoinHalfExtent, RigidbodyType::Dynamic);
                }
            }

            // プッシャーの上にもコインを数枚
            for(int col = -3; col <= 3; ++col) {
                const float x = static_cast<float>(col) * 7.0f;
                CreateBox(hlslpp::float3(x, kPusherHalfExtent.y * 2.0f + kCoinHalfExtent.y + 0.5f, kPusherCenterZ + 8.0f),
                          kCoinHalfExtent, RigidbodyType::Dynamic);
            }

            // 果物の代わりの球を上から落とす
            const float fruitXs[] = {-16.0f, -6.0f, 6.0f, 16.0f};
            for(int i = 0; i < 4; ++i) {
                CreateSphere(hlslpp::float3(fruitXs[i], 30.0f + static_cast<float>(i) * 6.0f, 0.0f + static_cast<float>(i % 2) * 10.0f), kFruitRadius);
            }
        }

        //--------------------------------------------------------------
        // ライト
        //--------------------------------------------------------------
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

        //--------------------------------------------------------------
        // 大気散乱（空）
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity e = m_scene.CreateEntity();
            registry.AddComponent<Tsukino::BuiltIn::ECS::SkyAtmosphereComponent>(e);
        }

        //--------------------------------------------------------------
        // カメラ（プレイヤーの目線：手前斜め上から台を見下ろす）
        //--------------------------------------------------------------
        {
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
        //--------------------------------------------------------------
        // デバッグカメラ（Debug ビルドのみ。切り替えは DebugCameraSystem の操作に従う）
        //--------------------------------------------------------------
        {
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

        Tsukino::Core::Log::Info("PusherScene: initialized.");
    }

    //----------------------------------------------------------------------------
    //! シーンを更新します。
    //----------------------------------------------------------------------------
    void PusherScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        //--------------------------------------------------------------
        // プッシャーを前後に往復させる
        // m_scene.Update() より前に書き込むことで、同じフレームの PhysicsSystem が
        // 位置の差分から速度を求め、乗っている景品を押す
        //--------------------------------------------------------------
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        if(m_pusherEntity != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity)) {
            m_pusherTime = std::fmod(m_pusherTime + deltaTime, kPusherPeriod);

            const float phase = 2.0f * kPi * m_pusherTime / kPusherPeriod;

            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_pusherEntity);
            t.position = hlslpp::float3(0.0f, kPusherHalfExtent.y, kPusherCenterZ + std::sin(phase) * kPusherAmplitude);
            t.dirty    = true;
        }

        // PhysicsSystem は1フレーム1ステップなので、起動直後のアセット読み込み等で
        // フレームが重くなったときに大きく進めないよう上限を設ける（その分ゲーム内時間は遅れる）
        m_scene.Update(std::min(deltaTime, kMaxSimulationStep));
    }

    //----------------------------------------------------------------------------
    //! シーンの終了処理を行います。
    //----------------------------------------------------------------------------
    void PusherScene::OnExit() {
    }

    //----------------------------------------------------------------------------
    //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PusherScene::CreateBox(const hlslpp::float3& position, const hlslpp::float3& halfExtent, Tsukino::BuiltIn::ECS::RigidbodyType type, bool visible) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        Tsukino::ECS::Entity    e        = m_scene.CreateEntity();

        // 見た目は箱モデルを伸縮させて合わせる（コライダーはスケールの影響を受けないので別途指定）
        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.scale                                      = (halfExtent * 2.0f) / kBlockModelSize;
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = m_blockModel;
        model.visible                                = visible;

        Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
        collision.type                                       = Tsukino::BuiltIn::ECS::ColliderType::Box;
        collision.extent                                     = halfExtent;
        collision.isSensor                                   = false;

        Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
        rb.type                                       = type;
        if(type == Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic) {
            rb.gravityFactor   = kGravityFactor;
            rb.friction        = 0.4f;
            rb.freezeRotationX = false;
            rb.freezeRotationY = false;
            rb.freezeRotationZ = false;
        }

        return e;
    }

    //----------------------------------------------------------------------------
    //! 球型の物体（見た目＋Sphereコライダー＋Dynamic剛体）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PusherScene::CreateSphere(const hlslpp::float3& position, float radius) {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        Tsukino::ECS::Entity    e        = m_scene.CreateEntity();

        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f) * (radius / m_ballModelRadius);
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = m_ballModel;
        model.visible                                = true;

        Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
        collision.type                                       = Tsukino::BuiltIn::ECS::ColliderType::Sphere;
        collision.extent.x                                   = radius;    // Sphere は x が半径
        collision.isSensor                                   = false;

        Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
        rb.type                                       = Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic;
        rb.gravityFactor                              = kGravityFactor;
        rb.friction                                   = 0.5f;
        rb.restitution                                = 0.2f;
        rb.freezeRotationX                            = false;
        rb.freezeRotationY                            = false;
        rb.freezeRotationZ                            = false;

        return e;
    }

}    // namespace FruitMagic

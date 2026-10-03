//----------------------------------------------------------------------------
//! @file   PrizeFactory.cpp
//! @brief  台の部品と景品のエンティティ生成の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/PrizeFactory.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>

#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/Asset/Model/ModelAsset.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CollisionComponent.hpp>

#include <algorithm>
#include <memory>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //! @brief 箱モデル（Block.fbx）のスケール1での寸法。中心原点の 100 x 20 x 100
        const hlslpp::float3 kBlockModelSize = hlslpp::float3(100.0f, 20.0f, 100.0f);

        //--------------------------------------------------------------
        //! 球モデルのスケール1での半径をメッシュのバウンディングボックスから求めます。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] handle       球モデルのハンドル
        //! @return 半径。取得できなかった場合は 1.0f
        //--------------------------------------------------------------
        float MeasureModelRadius(Tsukino::Asset::AssetManager& assetManager, Tsukino::Asset::AssetHandle handle) {
            auto model = std::dynamic_pointer_cast<Tsukino::Asset::ModelAsset>(assetManager.Get(handle));
            if(!model || model->modelData.meshes.empty()) {
                Tsukino::Core::Log::Warn("PrizeFactory: failed to measure ball model size. Falling back to radius 1.");
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
            return (radius > 0.0f) ? radius : 1.0f;
        }

        //--------------------------------------------------------------
        //! Dynamic 剛体の共通設定を行います。
        //! @param  [in,out] rb       設定する剛体
        //! @param  [in]     friction 摩擦係数
        //--------------------------------------------------------------
        void SetupDynamicBody(Tsukino::BuiltIn::ECS::RigidbodyComponent& rb, float friction) {
            rb.type            = Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic;
            rb.friction        = friction;
            rb.freezeRotationX = false;
            rb.freezeRotationY = false;
            rb.freezeRotationZ = false;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 生成に使うモデルを読み込みます。
    //----------------------------------------------------------------------------
    void PrizeFactory::Initialize(Tsukino::Asset::AssetManager& assetManager) {
        m_blockModel      = assetManager.Load(Tsukino::Core::Path("Assets/Models/Block.fbx"));
        m_ballModel       = assetManager.Load(Tsukino::Core::Path("Assets/Models/Ball.fbx"));
        m_ballModelRadius = MeasureModelRadius(assetManager, m_ballModel);
    }

    //----------------------------------------------------------------------------
    //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                                 Tsukino::BuiltIn::ECS::RigidbodyType type) const {
        Tsukino::ECS::Entity e = CreateVisualBox(registry, position, halfExtent);

        // コライダーはスケールの影響を受けないので、大きさを直接指定する
        Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
        collision.type                                       = Tsukino::BuiltIn::ECS::ColliderType::Box;
        collision.extent                                     = halfExtent;
        collision.isSensor                                   = false;

        Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
        rb.type                                       = type;
        if(type == Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic) {
            SetupDynamicBody(rb, 0.3f);
        }

        return e;
    }

    //----------------------------------------------------------------------------
    //! 見た目だけの箱（コライダー無し）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateVisualBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                                       float opacity) const {
        Tsukino::ECS::Entity e = registry.CreateEntity();

        // 見た目は箱モデルを伸縮させて合わせる
        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.scale                                      = BoxScale(halfExtent);
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = m_blockModel;
        model.visible                                = true;
        model.opacity                                = opacity;

        return e;
    }

    //----------------------------------------------------------------------------
    //! 景品のコインを生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateCoin(Tsukino::ECS::Registry& registry, const hlslpp::float3& position) const {
        Tsukino::ECS::Entity e = CreateBox(registry, position, CoinHalfExtent(), Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic);

        ECS::PrizeComponent& prize = registry.AddComponent<ECS::PrizeComponent>(e);
        prize.kind                 = ECS::PrizeKind::Coin;
        prize.value                = 1;

        return e;
    }

    //----------------------------------------------------------------------------
    //! 果物の代わりの球を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateFruit(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, float radius) const {
        Tsukino::ECS::Entity e = registry.CreateEntity();

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
        SetupDynamicBody(rb, 0.5f);
        rb.restitution = 0.2f;

        // M2 で果物定義データに置き換えるまでの仮の価値
        ECS::PrizeComponent& prize = registry.AddComponent<ECS::PrizeComponent>(e);
        prize.kind                 = ECS::PrizeKind::Fruit;
        prize.value                = 5;

        return e;
    }

    //----------------------------------------------------------------------------
    //! 箱モデルの見た目を、指定した大きさに合わせるスケールを求めます。
    //----------------------------------------------------------------------------
    hlslpp::float3 PrizeFactory::BoxScale(const hlslpp::float3& halfExtent) {
        return (halfExtent * 2.0f) / kBlockModelSize;
    }
}    // namespace FruitMagic

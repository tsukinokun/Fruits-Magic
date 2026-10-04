//----------------------------------------------------------------------------
//! @file   ShakeMagicSystem.cpp
//! @brief  魔法「ゆらゆら」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ShakeMagicSystem.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>

#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ImpulseRequestComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief この魔法の id（Magic.json の "id"）
        constexpr const char* kMagicId = "shake";

        //! @brief これより下にある景品は台から落ちている途中なので揺らさない
        constexpr float kOnTableMinY = -2.0f;

        //! @brief カメラの揺れの速さ（ラジアン/秒）
        constexpr float kCameraShakeSpeed = 45.0f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    ShakeMagicSystem::ShakeMagicSystem(Tsukino::ECS::EventBus& eventBus)
        : m_rng(std::random_device{}()) {
        // ハンドラでは覚えるだけにして、効果は Update で始める
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) { m_pendingCast = e.magicIndex; });
    }

    //----------------------------------------------------------------------------
    //! 効果を進めます。
    //----------------------------------------------------------------------------
    void ShakeMagicSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MagicCatalog>() || !registry.HasContext<MagicState>())
            return;

        const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();
        MagicState&         state   = registry.GetContext<MagicState>();

        //--------------------------------------------------------------
        // 自分（"shake"）の魔法が撃たれたら効果を始める
        //--------------------------------------------------------------
        if(m_pendingCast >= 0 && m_activeMagic < 0 && m_pendingCast < static_cast<int>(catalog.Magics().size()) &&
           catalog.Magics()[m_pendingCast].id == kMagicId) {
            m_activeMagic     = m_pendingCast;
            m_timer           = catalog.Magics()[m_activeMagic].Param("duration", 1.2f);
            m_pulseTimer      = 0.0f;
            m_elapsed         = 0.0f;
            state.activeMagic = m_activeMagic;

            // 揺らすカメラ（メインカメラ）と、その元の位置を覚えておく
            m_camera = entt::null;
            registry.View<Tsukino::BuiltIn::ECS::CameraComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
                [&](Tsukino::ECS::Entity entity, Tsukino::BuiltIn::ECS::CameraComponent& camera, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                    if(camera.isPrimary && m_camera == entt::null) {
                        m_camera     = entity;
                        m_cameraBase = transform.position;
                    }
                });
        }
        m_pendingCast = -1;

        if(m_activeMagic < 0)
            return;

        const MagicDef& def = catalog.Magics()[m_activeMagic];

        //--------------------------------------------------------------
        // 一定間隔で衝撃を与える
        //--------------------------------------------------------------
        m_pulseTimer -= deltaTime;
        if(m_pulseTimer <= 0.0f) {
            m_pulseTimer += def.Param("interval", 0.2f);
            Pulse(registry);
        }

        //--------------------------------------------------------------
        // カメラを小刻みに揺らす（だんだん弱く）
        //--------------------------------------------------------------
        m_elapsed += deltaTime;
        if(m_camera != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_camera)) {
            const float duration  = def.Param("duration", 1.2f);
            const float amplitude = def.Param("cameraShake", 0.6f) * std::max(0.0f, 1.0f - m_elapsed / duration);

            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_camera);
            t.position = m_cameraBase + hlslpp::float3(std::sin(m_elapsed * kCameraShakeSpeed) * amplitude, std::sin(m_elapsed * kCameraShakeSpeed * 1.3f) * amplitude * 0.5f, 0.0f);
            t.dirty    = true;
        }

        m_timer -= deltaTime;
        if(m_timer <= 0.0f) {
            Finish(registry);
        }
    }

    //----------------------------------------------------------------------------
    //! 台の上の景品すべてに衝撃を1回与えます。
    //----------------------------------------------------------------------------
    void ShakeMagicSystem::Pulse(Tsukino::ECS::Registry& registry) {
        const MagicDef& def     = registry.GetContext<MagicCatalog>().Magics()[m_activeMagic];
        const float     forward = def.Param("forward", 18.0f);
        const float     side    = def.Param("side", 6.0f);
        const float     up      = def.Param("up", 8.0f);

        std::uniform_real_distribution<float> random(-1.0f, 1.0f);

        //--------------------------------------------------------------
        // 衝撃は「質量 × 速度の変化」。質量を掛けて、重い果物も軽いコインも同じだけ動くようにする。
        // 付け外しは反復の後（反復中に同じ View のコンポーネント構成を変えないため）
        //--------------------------------------------------------------
        std::vector<std::pair<Tsukino::ECS::Entity, hlslpp::float3>> requests;
        registry.View<PrizeComponent, Tsukino::BuiltIn::ECS::RigidbodyComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, PrizeComponent&, Tsukino::BuiltIn::ECS::RigidbodyComponent& rb, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(float(transform.position.y) < kOnTableMinY)
                    return;
                const hlslpp::float3 velocity(random(m_rng) * side, up, forward);
                requests.emplace_back(entity, velocity * rb.mass);
            });

        for(const auto& [entity, impulse] : requests) {
            if(registry.HasComponent<Tsukino::BuiltIn::ECS::ImpulseRequestComponent>(entity))
                continue;
            registry.AddComponent<Tsukino::BuiltIn::ECS::ImpulseRequestComponent>(entity).impulse = impulse;
        }
    }

    //----------------------------------------------------------------------------
    //! 効果を終え、カメラを元の位置に戻します。
    //----------------------------------------------------------------------------
    void ShakeMagicSystem::Finish(Tsukino::ECS::Registry& registry) {
        if(m_camera != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_camera)) {
            auto& t    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_camera);
            t.position = m_cameraBase;
            t.dirty    = true;
        }

        m_activeMagic = -1;
        m_camera      = entt::null;
        registry.GetContext<MagicState>().activeMagic = -1;
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   GrowMagicSystem.cpp
//! @brief  魔法「おおきくなーれ」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/GrowMagicSystem.hpp>

#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/EffectEvent.hpp>
#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief この魔法の id（Magic.json の "id"）
        constexpr const char* kMagicId = "grow";
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    GrowMagicSystem::GrowMagicSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
        // ハンドラでは覚えるだけにして、効果は Update で出す
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) { m_pendingCast = e.magicIndex; });
    }

    //----------------------------------------------------------------------------
    //! 撃たれていれば果物を大きくします。
    //----------------------------------------------------------------------------
    void GrowMagicSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        const int cast = m_pendingCast;
        m_pendingCast  = -1;
        if(!registry.HasContext<MagicCatalog>() || !registry.GetContext<MagicCatalog>().IsMagic(cast, kMagicId))
            return;
        if(!registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>() || !registry.HasContext<PrizeFactory>() ||
           !registry.HasContext<GameState>())
            return;

        const MagicDef&     def     = registry.GetContext<MagicCatalog>().Magics()[cast];
        const float         minY    = def.Param("onTableMinY", -2.0f);    // これより下にある果物は台から落ちている途中なので選ばない
        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();

        //--------------------------------------------------------------
        // 台の上の、まだ大きくしていない果物のうち最も手前（Z が大きい）ものを選ぶ
        //--------------------------------------------------------------
        Tsukino::ECS::Entity target = entt::null;
        float                bestZ  = 0.0f;
        registry.View<PrizeComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, PrizeComponent& prize, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(prize.kind != PrizeKind::Fruit || prize.valueMultiplier > 1 || float(transform.position.y) < minY)
                    return;
                if(prize.fruitIndex < 0 || prize.fruitIndex >= static_cast<int>(catalog.Fruits().size()))
                    return;
                if(target == entt::null || float(transform.position.z) > bestZ) {
                    target = entity;
                    bestZ  = transform.position.z;
                }
            });

        if(target == entt::null) {
            // 対象が無ければマナを返す（撃ち損にしない）
            GameState& state = registry.GetContext<GameState>();
            state.mana       = std::min(state.maxMana, state.mana + def.cost);
            m_eventBus.Publish(NoticeEvent{GetTexts(registry).Get("notice.growNoFruit"), GetUiConfig(registry).growNoticeSeconds});
            return;
        }

        //--------------------------------------------------------------
        // 定義を複製して寸法・重さを変え、同じ位置・バリエーションで作り直す
        // （当たり判定の大きさは作った後から変えられないため）
        //--------------------------------------------------------------
        const PrizeComponent prize    = registry.GetComponent<PrizeComponent>(target);
        const hlslpp::float3 position = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(target).position;
        const float          scale    = std::max(1.0f, def.Param("scale", 1.6f));

        FruitDef giant   = catalog.Fruits()[prize.fruitIndex];
        const float oldHalfHeight = giant.HalfHeightOfBounds();
        giant.radius *= scale;
        giant.halfHeight *= scale;
        giant.halfExtent *= scale;
        giant.mass *= scale * scale;

        // 床や下の物にめり込まないよう、大きくなった分だけ持ち上げる
        const hlslpp::float3 newPosition = position + hlslpp::float3(0.0f, giant.HalfHeightOfBounds() - oldHalfHeight, 0.0f);

        const auto&    variants = registry.GetContext<CollectionConfig>().Variants();
        const int      variant  = std::clamp(prize.variantIndex, 0, static_cast<int>(variants.size()) - 1);
        hlslpp::float3 color    = variants[variant].ColorOf(giant);
        float          glow     = variants[variant].glow + def.Param("extraGlow", 1.2f);

        Tsukino::ECS::Entity grown = registry.GetContext<PrizeFactory>().CreateFruit(registry, giant, prize.fruitIndex, prize.variantIndex, color, glow, newPosition);
        registry.GetComponent<PrizeComponent>(grown).valueMultiplier = std::max(1, static_cast<int>(def.Param("valueMultiplier", 2.0f)));
        PrizeFactory::DestroyPrize(registry, target);

        // 演出: 周りで光が弾け、ぽよんと膨らんでから落ち着く
        registry.AddComponent<PopScaleComponent>(grown).baseScale = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(grown).scale;
        m_eventBus.Publish(EffectEvent{"grow", newPosition});

        m_eventBus.Publish(NoticeEvent{GetTexts(registry).Format("notice.grown", {{"name", registry.GetContext<CollectionConfig>().DisplayName(giant, prize.variantIndex)}}), GetUiConfig(registry).growNoticeSeconds});
    }
}    // namespace FruitMagic::ECS

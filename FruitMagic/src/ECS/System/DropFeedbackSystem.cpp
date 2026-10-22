//----------------------------------------------------------------------------
//! @file   DropFeedbackSystem.cpp
//! @brief  落ちた物が払い出しか溝かをその場で見せるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/DropFeedbackSystem.hpp>

#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Event/EffectEvent.hpp>
#include <FruitMagic/Game/EffectsConfig.hpp>
#include <FruitMagic/Game/StageConfig.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    DropFeedbackSystem::DropFeedbackSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
        // ハンドラでは積むだけにして、レジストリへの反映は Update で行う
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) { m_pendingDrops.push_back(e); });
    }

    //----------------------------------------------------------------------------
    //! 落ちた所に粒を頼み、台の縁の帯を光らせます。
    //----------------------------------------------------------------------------
    void DropFeedbackSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        const TableLayout&   layout = GetTableLayout(registry);
        const DropEdgeStyle& style  = GetStageConfig(registry).dropEdges;
        const float effectY = registry.HasContext<EffectsConfig>() ? registry.GetContext<EffectsConfig>().motion.dropEffectY : layout.dropJudgeY;

        //--------------------------------------------------------------
        // 落ちた所に粒を出す。横へこぼれた物は、その側の縁の帯（払い出しなら仕切りより手前の帯）も光らせる
        //--------------------------------------------------------------
        std::vector<PrizeDroppedEvent> drops;
        drops.swap(m_pendingDrops);
        for(const PrizeDroppedEvent& drop : drops) {
            const bool payout = drop.zone == DropZone::Payout;
            m_eventBus.Publish(EffectEvent{payout ? "drop_payout" : "drop_gutter", hlslpp::float3(drop.x, effectY, drop.z)});

            if(std::abs(drop.x) <= layout.fieldHalfWidth)
                continue;    // 手前の端から落ちた（帯は横の縁だけ）
            const int side = drop.x < 0.0f ? -1 : 1;
            registry.View<DropEdgeComponent>().each([&](Tsukino::ECS::Entity, DropEdgeComponent& edge) {
                if(edge.payout == payout && edge.side == side)
                    edge.flash = 1.0f;
            });
        }

        //--------------------------------------------------------------
        // 帯の光をふだんの強さへ戻していく
        //--------------------------------------------------------------
        registry.View<DropEdgeComponent, Tsukino::BuiltIn::ECS::RimGlowComponent>().each(
            [&](Tsukino::ECS::Entity, DropEdgeComponent& edge, Tsukino::BuiltIn::ECS::RimGlowComponent& rim) {
                edge.flash       = std::max(0.0f, edge.flash - deltaTime / style.flashSeconds);
                rim.rimIntensity = style.intensity + (style.flashIntensity - style.intensity) * edge.flash;
                rim.glow         = style.glow + (style.flashGlow - style.glow) * edge.flash;
            });
    }
}    // namespace FruitMagic::ECS

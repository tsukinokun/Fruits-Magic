//----------------------------------------------------------------------------
//! @file   HarvestSystem.cpp
//! @brief  果物の収穫を記録するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HarvestSystem.hpp>

#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/RecentHarvests.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    HarvestSystem::HarvestSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
        // 払い出し口に落ちた果物だけを積む（左右の溝に落ちた果物は失う）
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Fruit && e.zone == DropZone::Payout && e.fruitIndex >= 0)
                m_pendingFruits.push_back(PendingFruit{e.fruitIndex, e.variantIndex, e.valueMultiplier});
        });
    }

    //----------------------------------------------------------------------------
    //! 溜まった収穫を記録します。
    //----------------------------------------------------------------------------
    void HarvestSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<GameState>()) {
            m_pendingFruits.clear();
            return;
        }

        GameState& state = registry.GetContext<GameState>();
        for(const PendingFruit& pending : m_pendingFruits) {
            const int fruitIndex   = pending.fruitIndex;
            const int variantIndex = pending.variantIndex;
            if(variantIndex < 0)
                continue;

            // 果物やバリエーションの種類が後から増えても添字が収まるように広げる
            if(fruitIndex >= static_cast<int>(state.harvestCounts.size()))
                state.harvestCounts.resize(fruitIndex + 1);
            std::vector<int>& counts = state.harvestCounts[fruitIndex];
            if(variantIndex >= static_cast<int>(counts.size()))
                counts.resize(variantIndex + 1, 0);

            const bool isFirst = (counts[variantIndex] == 0);
            counts[variantIndex] += 1;

            // 価値（価値 × バリエーションの倍率 × 大きくした倍率）を、転生用の合計と強化に使う果実の両方に足す
            long long value = 0;
            if(registry.HasContext<FruitCatalog>() && registry.HasContext<CollectionConfig>()) {
                const auto& fruits   = registry.GetContext<FruitCatalog>().Fruits();
                const auto& variants = registry.GetContext<CollectionConfig>().Variants();
                if(fruitIndex < static_cast<int>(fruits.size()) && variantIndex < static_cast<int>(variants.size())) {
                    value = static_cast<long long>(fruits[fruitIndex].value) * variants[variantIndex].valueMultiplier * std::max(1, pending.valueMultiplier);
                    state.harvestValue += value;
                    state.fruitPoints += value;
                }
            }

            // 最近の収穫の記録（画面の横のパネルに出す）
            if(registry.HasContext<RecentHarvests>())
                registry.GetContext<RecentHarvests>().Add(RecentHarvest{fruitIndex, variantIndex, value, isFirst});

            if(isFirst)
                m_eventBus.Publish(ZukanRegisteredEvent{fruitIndex, variantIndex});
        }
        m_pendingFruits.clear();
    }
}    // namespace FruitMagic::ECS

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

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

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
                m_pendingFruits.emplace_back(e.fruitIndex, e.variantIndex);
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
        for(const auto& [fruitIndex, variantIndex] : m_pendingFruits) {
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

            // 価値の合計（価値 × バリエーションの倍率）
            if(registry.HasContext<FruitCatalog>() && registry.HasContext<CollectionConfig>()) {
                const auto& fruits   = registry.GetContext<FruitCatalog>().Fruits();
                const auto& variants = registry.GetContext<CollectionConfig>().Variants();
                if(fruitIndex < static_cast<int>(fruits.size()) && variantIndex < static_cast<int>(variants.size()))
                    state.harvestValue += static_cast<long long>(fruits[fruitIndex].value) * variants[variantIndex].valueMultiplier;
            }

            if(isFirst)
                m_eventBus.Publish(ZukanRegisteredEvent{fruitIndex, variantIndex});
        }
        m_pendingFruits.clear();
    }
}    // namespace FruitMagic::ECS

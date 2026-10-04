//----------------------------------------------------------------------------
//! @file   HarvestSystem.cpp
//! @brief  果物の収穫を記録するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HarvestSystem.hpp>

#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    HarvestSystem::HarvestSystem(Tsukino::ECS::EventBus& eventBus) {
        // 払い出し口に落ちた果物だけを積む（左右の溝に落ちた果物は失う。M3 でマナにする）
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Fruit && e.zone == DropZone::Payout && e.fruitIndex >= 0)
                m_pendingFruits.push_back(e.fruitIndex);
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
        for(int fruitIndex : m_pendingFruits) {
            // 果物の種類が後から増えても添字が収まるように広げる
            if(fruitIndex >= static_cast<int>(state.harvestCounts.size()))
                state.harvestCounts.resize(fruitIndex + 1, 0);
            state.harvestCounts[fruitIndex] += 1;
        }
        m_pendingFruits.clear();
    }
}    // namespace FruitMagic::ECS

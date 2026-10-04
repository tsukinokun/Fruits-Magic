//----------------------------------------------------------------------------
//! @file   ManaSystem.cpp
//! @brief  マナを溜めるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ManaSystem.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/ManaConfig.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    ManaSystem::ManaSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは積むだけにして、レジストリへの反映は Update で行う
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) { m_pendingDrops.push_back(e); });
    }

    //----------------------------------------------------------------------------
    //! 溜まった落下からマナを加算します。
    //----------------------------------------------------------------------------
    void ManaSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<GameState>()) {
            m_pendingDrops.clear();
            return;
        }

        GameState&       state  = registry.GetContext<GameState>();
        const ManaConfig config = registry.HasContext<ManaConfig>() ? registry.GetContext<ManaConfig>() : ManaConfig{};

        for(const PrizeDroppedEvent& e : m_pendingDrops) {
            int gain = 0;
            if(e.kind == PrizeKind::Coin) {
                gain = (e.zone == DropZone::Payout) ? config.coinPayout : config.coinGutter;
            } else if(registry.HasContext<FruitCatalog>()) {
                const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
                if(e.fruitIndex >= 0 && e.fruitIndex < static_cast<int>(fruits.size())) {
                    const int fruitMana = fruits[e.fruitIndex].mana;
                    gain = (e.zone == DropZone::Payout) ? fruitMana : static_cast<int>(std::ceil(fruitMana * config.fruitGutterRatio));
                }
            }
            state.mana = std::min(state.maxMana, state.mana + gain);
        }
        m_pendingDrops.clear();
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   ManaSystem.cpp
//! @brief  マナを溜めるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ManaSystem.hpp>

#include <FruitMagic/Game/CollectionConfig.hpp>
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

        // 図鑑ボーナス: 登録した枠の数だけマナの獲得量が増える
        const float bonusPerEntry = registry.HasContext<CollectionConfig>() ? registry.GetContext<CollectionConfig>().ManaBonusPerEntry() : 0.0f;
        const float multiplier    = 1.0f + bonusPerEntry * static_cast<float>(state.RegisteredCount());

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
            //--------------------------------------------------------------
            // 図鑑ボーナスを掛けた端数は持ち越す（切り上げると、1 のマナに 5% 掛けただけで 2 になってしまう）
            //--------------------------------------------------------------
            m_fraction += static_cast<float>(gain) * multiplier;
            const int whole = static_cast<int>(std::floor(m_fraction));
            m_fraction -= static_cast<float>(whole);
            state.mana = std::min(state.maxMana, state.mana + whole);
        }
        m_pendingDrops.clear();
    }
}    // namespace FruitMagic::ECS

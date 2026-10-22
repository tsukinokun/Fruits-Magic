//----------------------------------------------------------------------------
//! @file   ManaSystem.cpp
//! @brief  マナを溜めるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ManaSystem.hpp>

#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/ManaConfig.hpp>
#include <FruitMagic/Game/TableStats.hpp>

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

        GameState&  state      = registry.GetContext<GameState>();
        const float multiplier = GainMultiplier(registry);

        for(const PrizeDroppedEvent& e : m_pendingDrops) {
            const int gain = BaseGain(registry, e);
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

    //----------------------------------------------------------------------------
    //! 落ちた物1つで増えるマナ（倍率を掛ける前）を返します。
    //----------------------------------------------------------------------------
    int ManaSystem::BaseGain(Tsukino::ECS::Registry& registry, const PrizeDroppedEvent& drop) {
        const ManaConfig config = registry.HasContext<ManaConfig>() ? registry.GetContext<ManaConfig>() : ManaConfig{};
        if(drop.kind == PrizeKind::Coin)
            return (drop.zone == DropZone::Payout) ? config.coinPayout : config.coinGutter;
        if(!registry.HasContext<FruitCatalog>())
            return 0;
        const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
        if(drop.fruitIndex < 0 || drop.fruitIndex >= static_cast<int>(fruits.size()))
            return 0;
        const int fruitMana = fruits[drop.fruitIndex].mana;
        return (drop.zone == DropZone::Payout) ? fruitMana : static_cast<int>(std::ceil(fruitMana * config.fruitGutterRatio));
    }

    //----------------------------------------------------------------------------
    //! マナの獲得量に掛ける倍率を返します。
    //----------------------------------------------------------------------------
    float ManaSystem::GainMultiplier(Tsukino::ECS::Registry& registry) {
        // 図鑑ボーナス: 登録した枠の数だけマナの獲得量が増える
        const float bonusPerEntry = registry.HasContext<CollectionConfig>() ? registry.GetContext<CollectionConfig>().ManaBonusPerEntry() : 0.0f;
        const int   registered    = registry.HasContext<GameState>() ? registry.GetContext<GameState>().RegisteredCount() : 0;
        // 強化「マナの獲得」の倍率も掛ける
        const float upgrade       = registry.HasContext<TableStats>() ? registry.GetContext<TableStats>().manaMultiplier : 1.0f;
        return (1.0f + bonusPerEntry * static_cast<float>(registered)) * upgrade;
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   StatsSystem.cpp
//! @brief  これまでのプレイの累計を数えるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/StatsSystem.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    StatsSystem::StatsSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは数えるだけにして、GameState への反映は Update で行う
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind != PrizeKind::Coin)
                return;
            (e.zone == DropZone::Payout ? m_pendingPaidOut : m_pendingGutter) += 1;
        });
    }

    //----------------------------------------------------------------------------
    //! 累計に足します。
    //----------------------------------------------------------------------------
    void StatsSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<PlayStats>())
            return;

        LifetimeStats&   stats = registry.GetContext<GameState>().stats;
        const PlayStats& play  = registry.GetContext<PlayStats>();

        //--------------------------------------------------------------
        // PlayStats は起動してからの数なので、前のフレームから増えた分だけ足す
        //--------------------------------------------------------------
        stats.coinsLaunched  += play.coinsLaunched - m_lastPlay.coinsLaunched;
        stats.fairyCoins     += play.fairyCoins - m_lastPlay.fairyCoins;
        stats.showerCoins    += play.showerCoins - m_lastPlay.showerCoins;
        stats.rouletteSpins  += play.rouletteSpins - m_lastPlay.rouletteSpins;
        stats.rouletteHits   += play.rouletteHits - m_lastPlay.rouletteHits;
        stats.jackpotChances += play.jackpotChances - m_lastPlay.jackpotChances;
        stats.jackpots       += play.jackpots - m_lastPlay.jackpots;
        stats.magicsCast     += play.magicsCast - m_lastPlay.magicsCast;
        m_lastPlay = play;

        stats.coinsPaidOut  += m_pendingPaidOut;
        stats.coinsToGutter += m_pendingGutter;
        m_pendingPaidOut = 0;
        m_pendingGutter  = 0;

        stats.playSeconds += deltaTime;
    }
}    // namespace FruitMagic::ECS

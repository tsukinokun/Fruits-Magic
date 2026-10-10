//----------------------------------------------------------------------------
//! @file   WalletSystem.cpp
//! @brief  払い出された景品を手持ちに反映するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/WalletSystem.hpp>

#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/TableStats.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    WalletSystem::WalletSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは積むだけにして、レジストリへの反映は Update で行う
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) { m_pendingDrops.push_back(e); });
    }

    //----------------------------------------------------------------------------
    //! 溜まった払い出しを手持ちに反映します。
    //----------------------------------------------------------------------------
    void WalletSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<GameState>()) {
            m_pendingDrops.clear();
            return;
        }

        GameState& state      = registry.GetContext<GameState>();
        const float refundRate = registry.HasContext<TableStats>() ? registry.GetContext<TableStats>().gutterRefundRate : 0.0f;
        for(const PrizeDroppedEvent& e : m_pendingDrops) {
            // 手持ちに戻るのは払い出し口に落ちたコイン。
            // 果物は HarvestSystem が収穫として記録し、左右の溝に落ちた物は失う（マナにはなる）
            if(e.kind != PrizeKind::Coin)
                continue;
            if(e.zone == DropZone::Payout) {
                state.coins += e.value;
                continue;
            }

            //--------------------------------------------------------------
            // 強化「溝のおまもり」: 溝に落ちたコインの一部が戻る。割合の端数は持ち越す（毎回切り捨てると少ない割合で何も戻らない）
            //--------------------------------------------------------------
            m_refundFraction += refundRate * static_cast<float>(e.value);
            const int refund = static_cast<int>(m_refundFraction);
            m_refundFraction -= static_cast<float>(refund);
            state.coins += refund;
        }
        m_pendingDrops.clear();
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   WalletSystem.cpp
//! @brief  払い出された景品を手持ちに反映するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/WalletSystem.hpp>

#include <FruitMagic/Game/GameState.hpp>

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

        GameState& state = registry.GetContext<GameState>();
        for(const PrizeDroppedEvent& e : m_pendingDrops) {
            // 左右の溝に落ちた物は失う（M3 でマナに変換する）
            if(e.zone != DropZone::Payout)
                continue;

            // M1 では果物もコイン換算で手持ちに入れる（M2 で収穫処理に置き換える）
            state.coins += e.value;
        }
        m_pendingDrops.clear();
    }
}    // namespace FruitMagic::ECS

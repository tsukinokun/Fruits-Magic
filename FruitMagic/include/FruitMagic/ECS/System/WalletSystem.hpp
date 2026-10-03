//----------------------------------------------------------------------------
//! @file   WalletSystem.hpp
//! @brief  払い出された景品を手持ちに反映するシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 払い出された景品を手持ち（GameState）に反映するシステムです。
    class WalletSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit WalletSystem(Tsukino::ECS::EventBus& eventBus);

        //! 溜まった払い出しを手持ちに反映します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<PrizeDroppedEvent> m_pendingDrops;      // 次の Update で処理する払い出し
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   CheckerSystem.hpp
//! @brief  チェッカー（払い出し口で左右に動く穴）を動かし、穴に入ったコインを判定するシステム
//! @detail 穴は台の手前端の下、コインが最終的に落ちる払い出し口にあります。
//!         物理的な穴は開けず、PrizeDropSystem が出す落下イベントの「落ちた位置」で判定します。
//!         穴に入ったコインも手持ちには戻り（WalletSystem）、それに加えてルーレットが1回回ります。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! チェッカーを動かし、穴に入ったコインを判定するシステムです。
    class CheckerSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent の購読と CheckerEnteredEvent の発行に使うイベントバス
        explicit CheckerSystem(Tsukino::ECS::EventBus& eventBus);

        //! 穴を動かし、払い出し口に落ちたコインが穴に入ったかを判定します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus&        m_eventBus;          // 判定結果を発行するイベントバス
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<float>             m_pendingCoinXs;     // 次の Update で判定する、払い出し口に落ちたコインの X
    };
}    // namespace FruitMagic::ECS

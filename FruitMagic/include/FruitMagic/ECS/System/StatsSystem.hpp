//----------------------------------------------------------------------------
//! @file   StatsSystem.hpp
//! @brief  これまでのプレイの累計（LifetimeStats）を数えるシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/PlayStats.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! これまでのプレイの累計を数えるシステムです。
    //! 入れたコイン・ルーレット・魔法などは各システムが数えている PlayStats の増えた分を、
    //! コインの払い出しと溝は PrizeDroppedEvent を、遊んだ時間は経過時間を GameState::stats に足します。
    class StatsSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit StatsSystem(Tsukino::ECS::EventBus& eventBus);

        //! 累計に足します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        PlayStats                      m_lastPlay;          // 前のフレームに見た PlayStats（増えた分だけ足すため）
        int                            m_pendingPaidOut = 0;    // まだ足していない、払い出し口に落ちたコイン
        int                            m_pendingGutter  = 0;    // まだ足していない、溝に落ちたコイン
    };
}    // namespace FruitMagic::ECS

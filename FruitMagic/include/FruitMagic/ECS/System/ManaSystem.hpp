//----------------------------------------------------------------------------
//! @file   ManaSystem.hpp
//! @brief  落ちたコイン・果物からマナを溜めるシステム
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

    //! 落ちたコイン・果物からマナ（GameState::mana）を溜めるシステムです。
    //! 量は ManaConfig と FruitDef::mana で決まり、左右の溝に落ちたコインは多めに溜まります。
    class ManaSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit ManaSystem(Tsukino::ECS::EventBus& eventBus);

        //! 溜まった落下からマナを加算します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<PrizeDroppedEvent> m_pendingDrops;      // 次の Update で処理する落下
        float                          m_fraction = 0.0f;   // 図鑑ボーナスを掛けたときの、まだ加算していない端数
    };
}    // namespace FruitMagic::ECS

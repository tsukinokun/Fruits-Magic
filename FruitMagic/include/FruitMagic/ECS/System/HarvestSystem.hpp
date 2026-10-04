//----------------------------------------------------------------------------
//! @file   HarvestSystem.hpp
//! @brief  払い出し口に落ちた果物を収穫として記録するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <utility>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 払い出し口に落ちた果物を、果物 × バリエーションごとの収穫数（GameState::harvestCounts）と
    //! 価値の合計（GameState::harvestValue）に記録し、価値の分だけ果実（GameState::fruitPoints）を増やすシステムです。
    //! 枠を初めて収穫したときは ZukanRegisteredEvent を発行します。
    class HarvestSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent の購読と ZukanRegisteredEvent の発行に使うイベントバス
        explicit HarvestSystem(Tsukino::ECS::EventBus& eventBus);

        //! 溜まった収穫を記録します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus&          m_eventBus;          // 図鑑登録を発行するイベントバス
        Tsukino::ECS::ScopedConnection   m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<std::pair<int, int>> m_pendingFruits;     // 次の Update で記録する（果物の添字, バリエーションの添字）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   HarvestSystem.hpp
//! @brief  払い出し口に落ちた果物を収穫として記録するシステム
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

    //! 払い出し口に落ちた果物を収穫数（GameState::harvestCounts）に記録するシステムです。
    class HarvestSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit HarvestSystem(Tsukino::ECS::EventBus& eventBus);

        //! 溜まった収穫を記録します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<int>               m_pendingFruits;     // 次の Update で記録する果物の添字
    };
}    // namespace FruitMagic::ECS

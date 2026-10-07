//----------------------------------------------------------------------------
//! @file   RecentHarvests.hpp
//! @brief  最近収穫した果物の記録
//! @detail HarvestSystem が収穫のたびに足し、画面の横のパネル（SidePanelSystem）が新しい順に見せます。
//!         セーブはしません（起動するたびに空から始まります）。
//----------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <deque>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 収穫した果物1つの記録です。
    struct RecentHarvest {
        int       fruitIndex   = -1;       // 果物の添字
        int       variantIndex = 0;        // バリエーションの添字
        long long value        = 0;        // 増えた果実（価値 × バリエーションの倍率 × 大きくした倍率）
        bool      isNew        = false;    // 図鑑に初めて載ったか
    };

    //! 最近収穫した果物の記録です。Registry のコンテキストに置きます。
    struct RecentHarvests {
        std::deque<RecentHarvest> entries;         // 新しい順
        std::size_t               capacity = 5;    // 覚えておく件数

        //! 記録を1件足します。件数を超えたら古いものから消します。
        //! @param  [in] harvest 収穫した果物
        void Add(const RecentHarvest& harvest) {
            entries.push_front(harvest);
            while(entries.size() > capacity)
                entries.pop_back();
        }
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   TableStats.hpp
//! @brief  強化で変わる台の性能
//! @detail UpgradeSystem が強化のレベルから計算して書き込み、各システムが読みます。
//!         果樹の段階だけはセーブ対象の GameState::treeLevel に置きます。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/PusherLayout.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 強化で変わる台の性能です。Registry のコンテキストに置きます。
    struct TableStats {
        float pusherAmplitude    = Layout::kPusherAmplitude;    // プッシャーの往復の振幅（cm）。押し幅の強化で増える
        float autoLaunchInterval = 0.0f;                        // 妖精がコインを入れる間隔（秒）。0 なら入れない
        float checkerHalfWidth   = 4.0f;                        // チェッカーの穴の半幅（cm）
    };
}    // namespace FruitMagic

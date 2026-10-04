//----------------------------------------------------------------------------
//! @file   RouletteState.hpp
//! @brief  ルーレットの進行状態（表示用）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! ルーレットの段階です。
    enum class RoulettePhase {
        Idle,        // 回っていない
        Spinning,    // 回転中
        Result,      // 止まって結果を表示中
    };

    //! ルーレットの進行状態です。RouletteSystem が更新し、HUD が表示に使います。Registry のコンテキストに置きます。
    struct RouletteState {
        RoulettePhase phase        = RoulettePhase::Idle;    // 今の段階
        int           stock        = 0;                      // ためている回転の数（回転中の分は含まない）
        int           displayFruit = -1;                     // 今表示している果物の添字（-1 はハズレ）
        bool          resultHit    = false;                  // 結果が当たりか
    };
}    // namespace FruitMagic

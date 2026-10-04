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
        JackpotSpin,      // ジャックポットチャンスの抽選中
        JackpotResult,    // ジャックポットチャンスの結果を表示中
    };

    //! ルーレットの進行状態です。RouletteSystem が更新し、HUD が表示に使います。Registry のコンテキストに置きます。
    struct RouletteState {
        RoulettePhase phase        = RoulettePhase::Idle;    // 今の段階
        int           stock        = 0;                      // ためている回転の数（回転中の分は含まない）
        int           displayFruit = -1;                     // 今表示している果物の添字（-1 はハズレ）
        int           displayVariant = 0;                    // 今表示している果物のバリエーションの添字
        bool          resultHit    = false;                  // 結果が当たりか
        bool          jackpotDisplay = false;                // ジャックポットチャンスの抽選中に「JACKPOT」を表示しているか（false は「ハズレ」）
        bool          jackpotWin     = false;                // ジャックポットチャンスの結果が当たりか
    };
}    // namespace FruitMagic

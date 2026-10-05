//----------------------------------------------------------------------------
//! @file   ReliefState.hpp
//! @brief  おすそわけ（手持ちが尽きかけたときに妖精がくれるコイン）の待ち状態
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! おすそわけの待ち状態です。FairySystem が書き、HudSystem が表示します。
    //! Registry のコンテキストに置きます（セーブしない）。
    struct ReliefState {
        bool  waiting          = false;    // おすそわけを待っているか（手持ちが EconomyConfig::reliefBelow 枚より少ない）
        float secondsRemaining = 0.0f;     // 次の1枚までの時間（秒）
    };
}    // namespace FruitMagic

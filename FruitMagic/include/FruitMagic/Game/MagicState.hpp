//----------------------------------------------------------------------------
//! @file   MagicState.hpp
//! @brief  魔法ごとの効果中の状態
//----------------------------------------------------------------------------
#pragma once
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 魔法ごとの効果の残り時間です。効果を実装するシステムが書き、MagicInputSystem（同じ魔法の再発動を止める）と
    //! HUD（ボタンに残り秒数を出す）が読みます。違う魔法は同時に効果中になれます。Registry のコンテキストに置きます。
    struct MagicState {
        std::vector<float> remaining;    // 効果の残り時間（秒）[魔法の添字]。0 なら効果中でない

        //! 魔法が効果中かを返します。
        //! @param  [in] magicIndex 魔法の添字
        //! @return 効果中なら true
        bool IsActive(int magicIndex) const {
            return magicIndex >= 0 && magicIndex < static_cast<int>(remaining.size()) && remaining[magicIndex] > 0.0f;
        }

        //! 効果の残り時間を書きます（配列が足りなければ広げる）。
        //! @param  [in] magicIndex 魔法の添字
        //! @param  [in] seconds    残り時間（秒）。0 で効果の終わり
        void SetRemaining(int magicIndex, float seconds) {
            if(magicIndex < 0)
                return;
            if(magicIndex >= static_cast<int>(remaining.size()))
                remaining.resize(magicIndex + 1, 0.0f);
            remaining[magicIndex] = seconds > 0.0f ? seconds : 0.0f;
        }
    };
}    // namespace FruitMagic

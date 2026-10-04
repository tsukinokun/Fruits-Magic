//----------------------------------------------------------------------------
//! @file   MagicState.hpp
//! @brief  魔法の効果中かどうかの状態
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 魔法の効果中かどうかです。効果を実装するシステムが立て、MagicInputSystem が再発動を止めるのに使います。
    //! Registry のコンテキストに置きます。
    struct MagicState {
        int activeMagic = -1;    // 効果中の魔法の添字（-1 は効果中でない）
    };
}    // namespace FruitMagic

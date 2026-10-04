//----------------------------------------------------------------------------
//! @file   MagicEffects.hpp
//! @brief  効果中の魔法が台に与えている影響
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 効果中の魔法が台に与えている影響です。魔法の効果システムが書き、台を動かす側（PusherScene）が読みます。
    //! Registry のコンテキストに置きます。
    struct MagicEffects {
        float pusherAmplitudeBonus = 0.0f;    // プッシャーの振幅に足す量（cm）。魔法「ふくらむ」
    };
}    // namespace FruitMagic

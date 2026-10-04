//----------------------------------------------------------------------------
//! @file   ZukanState.hpp
//! @brief  図鑑画面の開閉状態
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 図鑑画面の開閉状態です。Registry のコンテキストに置きます。
    struct ZukanState {
        bool open = false;    // 図鑑を開いているか
    };
}    // namespace FruitMagic

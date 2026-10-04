//----------------------------------------------------------------------------
//! @file   MenuState.hpp
//! @brief  開いている画面（図鑑・強化・おかえり）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 台の上に重ねて開く画面の種類です。同時に開けるのは1つだけです。
    enum class MenuKind {
        None,       // 何も開いていない
        Zukan,      // 図鑑
        Upgrade,    // 台の強化
        Welcome,    // おかえり（閉じている間の報酬。起動時にだけ開く）
    };

    //! 開いている画面です。Registry のコンテキストに置きます。
    struct MenuState {
        MenuKind open = MenuKind::None;    // 開いている画面

        //! 指定の画面が開いているかを返します。
        //! @param  [in] menu 画面の種類
        //! @return 開いていれば true
        bool IsOpen(MenuKind menu) const { return open == menu; }
    };
}    // namespace FruitMagic

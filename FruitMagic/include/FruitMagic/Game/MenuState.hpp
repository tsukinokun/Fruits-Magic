//----------------------------------------------------------------------------
//! @file   MenuState.hpp
//! @brief  開いている画面（図鑑・強化・おかえり・オプション）
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/UpgradeCatalog.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 台の上に重ねて開く画面の種類です。同時に開けるのは1つだけです。
    enum class MenuKind {
        None,       // 何も開いていない
        Zukan,      // 図鑑
        Record,     // 記録（収穫やコインなどの累計とプレイ時間）
        Upgrade,    // 台の強化
        Welcome,    // おかえり（閉じている間の報酬。起動時にだけ開く）
        Options,    // オプション（音量・操作説明・データの消去・終了）
        Moved,      // おひっこし直後（新しい屋台・星のたね・おみやげ。おひっこしした直後にだけ開く）
    };

    //! 図鑑のタブです。
    enum class ZukanTab {
        Fruits,       // 果物（行＝果物、列＝バリエーション）
        Souvenirs,    // おみやげ（カードを格子に並べる）
    };

    //! 開いている画面です。Registry のコンテキストに置きます。
    struct MenuState {
        MenuKind        open       = MenuKind::None;             // 開いている画面
        UpgradeCurrency upgradeTab = UpgradeCurrency::Coins;    // 強化画面で選んでいるタブ（払うもの。閉じても覚えておく）
        ZukanTab        zukanTab   = ZukanTab::Fruits;          // 図鑑で選んでいるタブ（閉じても覚えておく）

        //! 指定の画面が開いているかを返します。
        //! @param  [in] menu 画面の種類
        //! @return 開いていれば true
        bool IsOpen(MenuKind menu) const { return open == menu; }
    };
}    // namespace FruitMagic

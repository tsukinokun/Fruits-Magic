//----------------------------------------------------------------------------
//! @file   ZukanElementComponent.hpp
//! @brief  図鑑画面の中身が変わる要素（果物名・色見本・収穫数・集計）のコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/MenuState.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 図鑑画面の要素の種類です。
    enum class ZukanElementKind {
        RowName,    // 行頭の果物名（登録済みなら名前、未登録なら「？？？」）
        Fruit,      // 枠の果物（FruitIcon の置き台。登録済みなら色付きで回り、未登録なら黒いシルエット）
        Count,      // 枠の収穫数（「×3」）
        Footer,     // 下部の集計（登録数・図鑑ボーナス）
        Tab,             // タブのボタン（スプライト。PointerTargetComponent と一緒に付ける。tab のタブに切り替える）
        TabGroup,        // タブの中身の親（UIVisibilityComponent を持つ。tab を選んでいる間だけ見せる）
        SouvenirIcon,    // おみやげの 3D（FruitIcon の置き台。もらっていれば色付きで回り、まだなら黒いシルエット）
        SouvenirName,    // おみやげの名前（まだなら「？？？」）
        SouvenirLevel,   // おみやげのレベル（「Lv 2 / 5」）
        SouvenirEffect,  // おみやげの効果の今の値（「FP +6%」）
        SouvenirFooter,  // おみやげのタブの下部の集計（もらった数 / 全体）
    };

    //! 図鑑画面の中身が変わる要素です。MenuPageComponent と一緒に付け、図鑑が開いている間 ZukanSystem が更新します。
    struct ZukanElementComponent {
        ZukanElementKind kind         = ZukanElementKind::RowName;    // 種類
        int              fruitIndex   = -1;                           // RowName / Fruit / Count の果物の添字。Souvenir* はおみやげの添字
        int              variantIndex = -1;                           // Fruit / Count のバリエーションの添字
        ZukanTab         tab          = ZukanTab::Fruits;             // Tab・TabGroup の受け持つタブ
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   ZukanElementComponent.hpp
//! @brief  図鑑画面の中身が変わる要素（果物名・色見本・収穫数・集計）のコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 図鑑画面の要素の種類です。
    enum class ZukanElementKind {
        RowName,    // 行頭の果物名（登録済みなら名前、未登録なら「？？？」）
        Fruit,      // 枠の果物（FruitIcon の置き台。登録済みなら色付きで回り、未登録なら黒いシルエット）
        Count,      // 枠の収穫数（「×3」）
        Footer,     // 下部の集計（登録数・図鑑ボーナス）
    };

    //! 図鑑画面の中身が変わる要素です。MenuPageComponent と一緒に付け、図鑑が開いている間 ZukanSystem が更新します。
    struct ZukanElementComponent {
        ZukanElementKind kind         = ZukanElementKind::RowName;    // 種類
        int              fruitIndex   = -1;                           // RowName / Fruit / Count の果物の添字
        int              variantIndex = -1;                           // Fruit / Count のバリエーションの添字
    };
}    // namespace FruitMagic::ECS

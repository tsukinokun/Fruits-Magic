//----------------------------------------------------------------------------
//! @file   CutInElementComponent.hpp
//! @brief  果物が取れたときのカットインの部品を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! カットインの部品の種類です。
    enum class CutInPart {
        Band,     // 帯（画面スプライト）
        Edge,     // 帯の上下の縁の線（画面スプライト。色は果物の色）
        Title,    // 見出し（「図鑑に登録！」など）
        Name,     // 果物の名前
        Fruit,    // 果物の置き台（ScreenModelComponent を持ち、果物の見た目を子に付ける）
    };

    //! カットインの部品です。CutInSystem が出し入れと動きを決めます。
    //! @note 画面スプライトはスケール 0 で描画されないので、出すときのスケールと位置を覚えておく
    struct CutInElementComponent {
        CutInPart      part       = CutInPart::Band;                       // 種類
        hlslpp::float3 shownScale = hlslpp::float3(1.0f, 1.0f, 1.0f);      // 出すときのスケール（スプライト用）
        hlslpp::float2 basePosition = hlslpp::float2(0.0f, 0.0f);          // 止まっているときの位置（画面ピクセル）
    };
}    // namespace FruitMagic::ECS

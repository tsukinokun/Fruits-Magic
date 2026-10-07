//----------------------------------------------------------------------------
//! @file   SidePanelElementComponent.hpp
//! @brief  画面の左右のパネル（台のようす・進み具合とおすすめ）の要素を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! パネルの要素の種類です。
    enum class SidePanelElementKind {
        Static,           // 板・見出しなど、中身が変わらないもの（文字は text）
        RecentSwatch,     // 最近とれた果物の色（row 行目）
        RecentName,       // 最近とれた果物の名前
        RecentValue,      // 最近とれた果物で増えた果実
        TableSwatch,      // 台の上の果物の色（値の高い順に row 行目）
        TableName,        // 台の上の果物の名前と数
        TableValue,       // 台の上の果物の価値
        TableTotal,       // 台の上の果物の合計
        ZukanCount,       // 図鑑の登録数
        ZukanBar,         // 図鑑の進み具合のバー
        ZukanNext,        // 次の魔法まであと何種類か
        UpgradeButton,    // おすすめの強化の板（押すと強化の画面が開く）
        UpgradeName,      // おすすめの強化の名前とレベル
        UpgradeCost,      // おすすめの強化の値段
        Fairy,            // 妖精の自動投入
        Offline,          // おるすばんの上限時間
    };

    //! 画面の左右のパネルの要素です。SidePanelSystem が中身を書き、画面（図鑑など）を開いている間は隠します。
    //! @note 画面スプライトはスケール 0 で描画されないので、出すときのスケールを覚えておく
    struct SidePanelElementComponent {
        SidePanelElementKind kind       = SidePanelElementKind::Static;          // 種類
        int                  row        = -1;                                    // 行（一覧の行のとき）
        hlslpp::float3       shownScale = hlslpp::float3(1.0f, 1.0f, 1.0f);      // 出すときのスケール（スプライト用）
        std::wstring         text;                                               // 決まった文字（Static のとき）
        float                barLeft    = 0.0f;                                  // バーの左端（ZukanBar のとき。画面ピクセル）
        float                barWidth   = 0.0f;                                  // バーの全幅（ZukanBar のとき。画面ピクセル）
    };
}    // namespace FruitMagic::ECS

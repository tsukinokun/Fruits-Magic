//----------------------------------------------------------------------------
//! @file   RecordElementComponent.hpp
//! @brief  記録画面の数の要素のコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 記録画面に出す数の種類です。
    enum class RecordItem {
        HarvestCount,     // 収穫した果物の数
        HarvestValue,     // 収穫で得た FP の合計
        Zukan,            // 図鑑の登録数 / 全体
        FruitPoints,      // 今の FP
        CoinsLaunched,    // 入れたコイン
        CoinsPaidOut,     // 払い出されたコイン
        CoinsToGutter,    // 溝に落ちたコイン
        FairyCoins,       // 妖精が入れたコイン
        ShowerCoins,      // 降ってきたコイン
        RouletteSpins,    // ルーレットを回した回数
        RouletteHits,     // 果物が当たった回数（割合）
        JackpotChances,   // ジャックポットチャンスの回数
        Jackpots,         // ジャックポットに当たった回数
        MagicsCast,       // 魔法を使った回数
        PlayTime,         // 遊んだ時間
    };

    //! 記録画面の数の要素です。MenuPageComponent と一緒に付け、記録画面が開いている間 RecordSystem が書きます。
    struct RecordElementComponent {
        RecordItem item = RecordItem::HarvestCount;    // 出す数
    };
}    // namespace FruitMagic::ECS

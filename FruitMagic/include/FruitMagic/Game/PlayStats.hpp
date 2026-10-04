//----------------------------------------------------------------------------
//! @file   PlayStats.hpp
//! @brief  起動してからのプレイの集計（バランスの計測用）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 起動してからのプレイの集計です。各システムが数え、BalanceProbeSystem がログに出します。
    //! セーブはしません。Registry のコンテキストに置きます。
    struct PlayStats {
        int coinsLaunched  = 0;    // プレイヤーが投入したコイン
        int fairyCoins     = 0;    // 妖精が入れたコイン
        int showerCoins    = 0;    // シャワー（メテオコイン・ジャックポット）で降ったコイン
        int rouletteSpins  = 0;    // ルーレットを回した回数
        int rouletteHits   = 0;    // ルーレットの当たり
        int jackpotChances = 0;    // ジャックポットチャンス
        int jackpots       = 0;    // ジャックポット（チャンスで当たった回数）
        int magicsCast     = 0;    // 魔法を撃った回数
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   LifetimeStats.hpp
//! @brief  これまでのプレイの累計（記録画面に出す。セーブする）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! これまでのプレイの累計です。GameState に持ち、セーブします（StatsSystem が数える）。
    //! 収穫の記録は GameState の収穫数・収穫した価値から出すので、ここには持たない。
    struct LifetimeStats {
        long long coinsLaunched  = 0;       // プレイヤーが入れたコイン
        long long coinsPaidOut   = 0;       // 払い出し口に落ちたコイン
        long long coinsToGutter  = 0;       // 溝に落ちたコイン
        long long fairyCoins     = 0;       // 妖精が入れたコイン
        long long showerCoins    = 0;       // 降ってきたコイン（ルーレット・ジャックポット・メテオコイン）
        long long rouletteSpins  = 0;       // ルーレットを回した回数
        long long rouletteHits   = 0;       // ルーレットで果物が当たった回数
        long long jackpotChances = 0;       // ジャックポットチャンス
        long long jackpots       = 0;       // ジャックポット（チャンスで当たった回数）
        long long magicsCast     = 0;       // 魔法を使った回数
        double    playSeconds    = 0.0;     // 遊んだ時間（秒。閉じていた間は含めない）
    };
}    // namespace FruitMagic

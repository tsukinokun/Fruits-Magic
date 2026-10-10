//----------------------------------------------------------------------------
//! @file   TableStats.hpp
//! @brief  強化で変わる台の性能
//! @detail UpgradeSystem が強化のレベルから計算して書き込み、各システムが読みます。
//!         果樹の段階だけはセーブ対象の GameState::treeLevel に置きます。
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 強化で変わる台の性能です。Registry のコンテキストに置きます。
    struct TableStats {
        //--------------------------------------------------------------
        // 台（コインで強化）
        //--------------------------------------------------------------
        float pusherAmplitude    = 12.0f;    // プッシャーの往復の振幅（cm）。起動時に Table.json の値が入り、押し幅の強化で増える
        float pusherPeriod       = 3.0f;     // プッシャーの往復の周期（秒）。起動時に Table.json の値が入り、速さの強化で短くなる
        float autoLaunchInterval = 0.0f;     // 妖精がコインを入れる間隔（秒）。0 なら入れない
        float checkerHalfWidth   = 4.0f;     // チェッカーの穴の半幅（cm）
        float offlineMaxHours    = 2.0f;     // 閉じている間に報酬が貯まる上限時間（時間）
        int   rouletteMaxStock   = 4;        // ルーレットをためておける回転の数（Roulette.json の値から強化で増える）
        int   rouletteCoinAmount = 5;        // ルーレットのコイン当たりで降らせる枚数（同上）
        float gutterRefundRate   = 0.0f;     // 溝に落ちたコインのうち手持ちに戻る割合（0〜1）

        //--------------------------------------------------------------
        // 果物と魔法（FP で強化）
        //--------------------------------------------------------------
        float fruitValueMultiplier   = 1.0f;    // 収穫したときの FP の倍率
        float variantChanceMultiplier = 1.0f;   // 色違い・金色の出やすさの倍率
        float rouletteHitChance      = 0.4f;    // ルーレットで果物が当たる確率（Roulette.json の値から強化で上がる）
        float manaMultiplier         = 1.0f;    // マナの獲得量の倍率
        float magicDurationMultiplier = 1.0f;   // 魔法の効果時間の倍率
    };
}    // namespace FruitMagic

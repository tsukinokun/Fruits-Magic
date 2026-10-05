//----------------------------------------------------------------------------
//! @file   JackpotConfig.hpp
//! @brief  ジャックポットチャンス（ルーレットのまれな2段階抽選）の設定
//! @detail Assets/Data/Jackpot.json から読み込みます。無い項目は既定値のままです。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! ジャックポットチャンスの設定です。ルーレットを回すたびに chanceRate でチャンスになり、
    //! チャンスの抽選に winRate で当たるとジャックポットです。Registry のコンテキストに置きます。
    struct JackpotConfig {
        float chanceRate       = 0.04f;    // ルーレット1回あたりのジャックポットチャンスの確率（0〜1）
        float winRate          = 0.34f;    // チャンスでジャックポットに当たる確率（0〜1）
        float spinSeconds      = 2.5f;     // チャンスの抽選を見せる時間（秒）
        float resultSeconds    = 2.5f;     // 結果を表示しておく時間（秒）
        int   bonusCoins       = 40;       // ジャックポットで降らせるコイン
        int   consolationCoins = 5;        // チャンスで外れたときの残念賞のコイン
        int   spins            = 3;        // ジャックポットで増えるルーレットの回数（ためておける上限を超えてよい）
        float flipInterval     = 0.12f;    // 抽選中に「JACKPOT」「ハズレ」を切り替える間隔（秒）。通常の回転より少しゆっくり
        float winShowerSeconds  = 3.0f;    // ジャックポットのコインを降らせる時間（秒）
        float loseShowerSeconds = 1.0f;    // 残念賞のコインを降らせる時間（秒）
        float winNoticeSeconds  = 4.0f;    // ジャックポットのお知らせを出しておく時間（秒）

        hlslpp::float3 chanceEffectPosition = hlslpp::float3(0.0f, 12.0f, 5.0f);    // チャンスの光を出す位置
        hlslpp::float3 winEffectPosition    = hlslpp::float3(0.0f, 8.0f, 10.0f);    // ジャックポットの光を出す位置

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Jackpot.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

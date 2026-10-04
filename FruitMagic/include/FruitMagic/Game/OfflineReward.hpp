//----------------------------------------------------------------------------
//! @file   OfflineReward.hpp
//! @brief  閉じている間の報酬（放置）の設定と計算
//! @detail 物理は回さず、強化の値からの期待値で「妖精が入れたコインのうち戻ってきた分」と
//!         「その間に収穫できた果物」を計算します。設定は Assets/Data/Offline.json から読みます。
//----------------------------------------------------------------------------
#pragma once
#include <random>
#include <string>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 放置とセーブの設定です。Registry のコンテキストに置きます。
    struct OfflineConfig {
        float payoutRatio     = 0.5f;     // 妖精が入れたコインのうち、手持ちに戻る割合
        float fruitPerCoin    = 0.03f;    // 妖精が入れたコイン1枚あたりに収穫できる果物の数
        int   minSeconds      = 60;       // これより短い留守では報酬も「おかえり」も出さない（秒）
        float autosaveSeconds = 30.0f;    // 自動セーブの間隔（秒）
        std::string savePath;             // セーブファイルのパス（定義データではなくシーンが決める）

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Offline.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };

    //! 閉じている間の報酬の結果です。「おかえり」画面に出します。
    struct OfflineReport {
        long long awaySeconds    = 0;        // 閉じていた時間（秒）
        long long countedSeconds = 0;        // 報酬の計算に使った時間（上限で切った後、秒）
        bool      capped         = false;    // 上限時間で切ったか
        bool      hasFairy       = false;    // 妖精の自動投入を強化済みか（無ければ報酬は 0）
        int       coins          = 0;        // 増えたコイン
        int       fruitCount     = 0;        // 収穫した果物の数
        long long fruitPoints    = 0;        // 増えた果実
    };

    //! 閉じていた時間から報酬を計算し、GameState に足します。
    //! @param  [in]     registry    レジストリ（GameState・TableStats・FruitCatalog・OfflineConfig を参照する）
    //! @param  [in]     awaySeconds 閉じていた時間（秒）
    //! @param  [in,out] rng         収穫する果物の抽選に使う乱数
    //! @return 報酬の結果
    //! @note   強化（TableStats）を反映した後に呼びます。果物は図鑑には登録しません（図鑑は台の上で実際に収穫したものだけ）
    OfflineReport GrantOfflineReward(Tsukino::ECS::Registry& registry, long long awaySeconds, std::mt19937& rng);

    //! 秒数を「2時間15分」のような表示にします。
    //! @param  [in] seconds 秒数
    //! @return 表示用の文字列
    std::wstring FormatDuration(long long seconds);
}    // namespace FruitMagic

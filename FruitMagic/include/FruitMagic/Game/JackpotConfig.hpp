//----------------------------------------------------------------------------
//! @file   JackpotConfig.hpp
//! @brief  ジャックポット穴の設定
//! @detail Assets/Data/Jackpot.json から読み込みます。無い項目は既定値のままです。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! ジャックポット穴（プッシャー上面の前寄りにある、時々開く小さな穴）の設定です。Registry のコンテキストに置きます。
    struct JackpotConfig {
        float holeHalfWidth = 2.5f;     // 穴の半幅（cm）。コインの中心がこの中に入ると吸い込む
        float holeHalfDepth = 2.5f;     // 穴の奥行の半分（cm）
        float frontOffset   = 5.0f;     // プッシャーの前面から穴の中心までの距離（cm）
        float range         = 18.0f;    // 穴が左右に往復する範囲（中心からの距離、cm）
        float period        = 7.0f;     // 左右の往復の周期（秒）
        float openSeconds   = 4.0f;     // 開いている時間（秒）
        float closedSeconds = 20.0f;    // 閉じている時間（秒）
        int   spins         = 3;        // 当たりで増えるルーレットの回数（ためておける上限を超えてよい）
        int   bonusCoins    = 10;       // 当たりで降らせるコインの枚数

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Jackpot.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

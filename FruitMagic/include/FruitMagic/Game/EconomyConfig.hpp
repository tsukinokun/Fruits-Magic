//----------------------------------------------------------------------------
//! @file   EconomyConfig.hpp
//! @brief  コインのやりくりの設定（初期コイン・手持ちが尽きたときの救済・台の上のコインの上限）
//! @detail Assets/Data/Economy.json から読み込みます。無い項目は既定値のままです。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! コインのやりくりの設定です。Registry のコンテキストに置きます。
    struct EconomyConfig {
        int   startCoins      = 50;      // 新しく始めたときの手持ちのコイン
        int   reliefBelow     = 5;       // 手持ちがこれより少ないと、妖精がコインを1枚ずつくれる（詰み防止）
        float reliefSeconds   = 4.0f;    // 妖精がコインをくれる間隔（秒）
        int   maxCoinsOnTable = 450;     // コインのシャワーで台の上のコインをこれ以上増やさない（物理が重くなりすぎないように）

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Economy.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

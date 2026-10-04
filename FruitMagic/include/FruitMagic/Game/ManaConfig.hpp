//----------------------------------------------------------------------------
//! @file   ManaConfig.hpp
//! @brief  マナの獲得量の設定
//! @detail Assets/Data/Mana.json から読み込みます。無い項目は既定値のままです。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! マナの獲得量の設定です。Registry のコンテキストに置いて共有します。
    struct ManaConfig {
        int   maxMana          = 100;     // マナの上限
        int   coinPayout       = 1;       // 払い出し口に落ちたコイン1枚で増えるマナ
        int   coinGutter       = 3;       // 左右の溝に落ちたコイン1枚で増えるマナ（溝落ちの救済で多め）
        float fruitGutterRatio = 0.5f;    // 溝に落ちた果物のマナの倍率（FruitDef::mana に掛けて切り上げ）

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Mana.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

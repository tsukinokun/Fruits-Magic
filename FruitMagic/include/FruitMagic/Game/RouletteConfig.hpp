//----------------------------------------------------------------------------
//! @file   RouletteConfig.hpp
//! @brief  チェッカー（動く穴）とルーレットの設定
//! @detail Assets/Data/Roulette.json から読み込みます。無い項目は既定値のままです。
//!         穴の幅は強化で変わるので、Assets/Data/Upgrades.json の "checkerWidth" で決めます。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! チェッカーとルーレットの設定です。Registry のコンテキストに置いて共有します。
    struct RouletteConfig {
        float hitChance       = 0.4f;     // 当たりの確率（0〜1）
        float spinSeconds     = 1.5f;     // 1回の回転にかける時間（秒）
        float resultSeconds   = 1.2f;     // 止まった結果を表示しておく時間（秒）
        int   maxStock        = 4;        // ためておける回転の数
        float checkerRange    = 22.0f;    // 穴が往復する範囲（中心からの距離、cm）
        float checkerPeriod   = 4.0f;     // 穴の往復の周期（秒）

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Roulette.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

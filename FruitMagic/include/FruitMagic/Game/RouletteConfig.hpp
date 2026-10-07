//----------------------------------------------------------------------------
//! @file   RouletteConfig.hpp
//! @brief  チェッカー（動く穴）とルーレットの設定
//! @detail Assets/Data/Roulette.json から読み込みます。無い項目は既定値のままです。
//!         穴の幅は強化で変わるので、Assets/Data/Upgrades.json の "checkerWidth" で決めます。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/RouletteState.hpp>

#include <array>
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! チェッカーとルーレットの設定です。Registry のコンテキストに置いて共有します。
    struct RouletteConfig {
        float hitChance       = 0.4f;     // 当たり（果物）の確率（0〜1）
        float coinChance      = 0.3f;     // 果物が外れたときに、コインが当たる確率（0〜1）
        int   coinAmount      = 5;        // コイン当たりで手持ちに入るコイン
        float resultSeconds   = 0.8f;     // 止まった結果を表示しておく時間（秒）
        int   maxStock        = 4;        // ためておける回転の数
        float checkerRange    = 16.0f;    // 穴が往復する範囲（中心からの距離、cm）
        float checkerPeriod   = 4.0f;     // 穴の往復の周期（秒）

        //--------------------------------------------------------------
        // スロット（3リール）の止め方。当たりの確率は上のまま（結果を先に抽選し、リールはそれに合わせて止まる）
        //--------------------------------------------------------------
        std::array<float, kReelCount> reelStopSeconds   = {0.6f, 0.85f, 1.1f};    // 回し始めてから各列が止まるまで（秒。左から）
        float                         reachExtraSeconds = 0.7f;                  // リーチ（1・2列目がそろった）のとき、3列目を長く回す時間（秒）
        float                         reachMissChance   = 0.25f;                 // ハズレのとき、2列だけそろえて「ハズレのリーチ」にする確率（0〜1）
        float                         flySeconds        = 0.45f;                 // 当たりの果物がスロットから台へ飛ぶ時間（秒。着いたら本物を出す）

        //--------------------------------------------------------------
        // 当たりの果物を台に置く位置
        //--------------------------------------------------------------
        float fruitSpawnMargin     = 6.0f;    // 置く範囲を払い出し口の半幅から狭める量（端に置くと押し出されて溝へ落ちやすい）
        float fruitSpawnLift       = 0.5f;    // プッシャー上面から浮かせる高さ（高いと下の物を押し込んでめり込ませる）
        float fruitSpawnBackMargin = 0.5f;    // 大きな果物を背面パネルから離す余白

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Roulette.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

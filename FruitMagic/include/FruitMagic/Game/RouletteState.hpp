//----------------------------------------------------------------------------
//! @file   RouletteState.hpp
//! @brief  ルーレット（3リールのスロット）の進行状態（表示用）
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <array>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! スロットのリールの数です。
    inline constexpr int kReelCount = 3;

    //! リールの絵柄で、コインを表す値です（果物は FruitCatalog::Fruits() の添字）。FruitIcon のコインの値と同じにしてあり、そのまま渡せます。
    inline constexpr int kCoinSymbol = -2;

    //! ルーレットの段階です。
    enum class RoulettePhase {
        Idle,             // 回っていない
        Spinning,         // 回転中（リールが左から順に止まる）
        Result,           // 止まって結果を表示中
        JackpotSpin,      // ジャックポットチャンスの抽選中（大きなスロットが回る）
        JackpotResult,    // ジャックポットチャンスの結果を表示中
    };

    //! ルーレットの進行状態です。RouletteSystem が更新し、スロットの表示（SlotMachineSystem）・HUD・効果音が見ます。
    //! Registry のコンテキストに置きます。
    struct RouletteState {
        RoulettePhase phase          = RoulettePhase::Idle;    // 今の段階
        int           stock          = 0;                      // ためている回転の数（回転中の分は含まない）
        int           displayFruit   = -1;                     // 結果の果物の添字（-1 は果物でない）
        int           displayVariant = 0;                      // 結果の果物のバリエーションの添字
        bool          resultHit      = false;                  // 結果が当たり（果物）か
        int           resultCoins    = 0;                      // 結果がコイン当たりなら、その枚数（0 は無し）
        bool          jackpotWin     = false;                  // ジャックポットチャンスの結果が当たりか

        //--------------------------------------------------------------
        // スロットの止め方（結果は先に抽選してあり、リールはそれに合わせて止まる）
        //--------------------------------------------------------------
        std::array<int, kReelCount>   reelSymbols   = {kCoinSymbol, kCoinSymbol, kCoinSymbol};    // 各列の止める絵柄（果物の添字か kCoinSymbol）
        std::array<float, kReelCount> reelStopTimes = {0.0f, 0.0f, 0.0f};                         // 回し始めてから各列が止まるまで（秒）
        float                         spinTime      = 0.0f;     // 今の回転を始めてからの時間（秒。止まった後も結果の間は進む）
        int                           reelsStopped  = 0;        // 止まった列の数
        bool                          reach         = false;    // 1・2列目がそろった（3列目を長く・ゆっくり回す）
        int                           spinId        = 0;        // 回すたびに1増える（表示が新しい回転に気づくため）
        bool                          jackpotReels  = false;    // 今の回転がジャックポットの大きなスロットか
        int                           reelVariant   = 0;        // 当たりの果物のバリエーション（止まった3つをこの色で見せる。0 は通常）

        //--------------------------------------------------------------
        // 当たった果物が、スロットから台へ飛んでいる途中（着いたら RouletteSystem が本物の果物を出す）
        //--------------------------------------------------------------
        int            flyFruit   = -1;                                     // 飛んでいる果物の添字（-1 は飛んでいない）
        int            flyVariant = 0;                                      // そのバリエーション
        float          flyTime    = 0.0f;                                   // 飛び始めてからの時間（秒）
        hlslpp::float3 flyTarget  = hlslpp::float3(0.0f, 0.0f, 0.0f);       // 着く所（台の上のワールド座標。本物の果物を出す位置）
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   PusherLayout.hpp
//! @brief  プッシャー台の寸法・配置の定数
//! @detail シーンの構築と各システム（投入・落下判定）で共有する寸法をまとめます。
//!         単位はエンジン規約どおり 1unit ≒ 1cm。奥が -Z、手前（プレイヤー側・落下口）が +Z です。
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::Layout
namespace FruitMagic::Layout {
    //--------------------------------------------------------------
    // プレイフィールド
    //--------------------------------------------------------------
    inline constexpr float kFieldHalfWidth = 30.0f;     // プレイフィールドの半幅（X）
    inline constexpr float kFieldFrontZ    = 40.0f;     // 手前端のZ（ここから先へ押し出された物が落ちる）
    inline constexpr float kFieldBackZ     = -80.0f;    // 奥端のZ（プッシャーが引っ込んでも床が切れない位置）

    inline constexpr float kStaticThickness = 10.0f;    // 床・トレイのコライダーの半分の厚み（すり抜け対策で厚めにする）

    //--------------------------------------------------------------
    // プッシャー
    //--------------------------------------------------------------
    inline constexpr float kPusherHalfWidth  = kFieldHalfWidth - 0.5f;    // 半幅（X）
    inline constexpr float kPusherHalfHeight = 5.0f;                      // 半分の高さ（Y）
    inline constexpr float kPusherSinkDepth  = 2.0f;                      // 床へ埋める深さ。床面に下端の角があるとコインがプッシャーの下へ潜り込むため
    inline constexpr float kPusherCenterY    = kPusherHalfHeight - kPusherSinkDepth;    // 中心の高さ
    inline constexpr float kPusherTopY       = kPusherCenterY + kPusherHalfHeight;      // 上面の高さ
    inline constexpr float kPusherHalfDepth  = 20.0f;                     // 半分の奥行（Z）。投入位置が常に上面の上に来るだけの奥行が要る
    inline constexpr float kPusherCenterZ    = -45.0f;                    // 往復の中心Z
    inline constexpr float kPusherAmplitude  = 12.0f;                     // 往復の振幅
    inline constexpr float kPusherPeriod     = 3.0f;                      // 往復の周期（秒）

    //--------------------------------------------------------------
    // 落下口
    //--------------------------------------------------------------
    inline constexpr float kPayoutHalfWidth = 20.0f;    // 正面の払い出し口の半幅。これより外側は左右の溝
    inline constexpr float kDropJudgeY      = -20.0f;   // これより下に落ちた物を「台から落ちた」と判定する高さ（落ちる様子が見えるようトレイの少し上）
    inline constexpr float kTrayTopY        = -30.0f;   // 景品受けトレイ上面の高さ

    //--------------------------------------------------------------
    // コインの投入
    //--------------------------------------------------------------
    inline constexpr float kLaunchLaneHalfWidth = 25.0f;     // 投入位置を動かせる左右の範囲

    //--------------------------------------------------------------
    // 背面パネル（プッシャーが引っ込むときに上面の景品を掻き落とす）
    //--------------------------------------------------------------
    inline constexpr float kBackPanelZ             = kPusherCenterZ - 5.0f;    // 中心のZ。プッシャーが最も前に出たときも上面の上にある位置
    inline constexpr float kBackPanelHalfThickness = 1.0f;                     // 半分の厚み（Z）

    //--------------------------------------------------------------
    // 投入位置のZ。背面パネルの前面と、プッシャーが最も引っ込んだときの前面の中間。
    // ここなら投入したコインは必ずプッシャー上面に落ちる。床へ直接落とすと、物理の接触許容値が
    // メートル単位向けのためコインが床に数cmめり込み、その上をプッシャーが通って下に閉じ込めてしまう
    //--------------------------------------------------------------
    inline constexpr float kBackPanelFrontZ  = kBackPanelZ + kBackPanelHalfThickness;
    inline constexpr float kPusherMinFrontZ  = kPusherCenterZ - kPusherAmplitude + kPusherHalfDepth;
    inline constexpr float kLaunchZ          = (kBackPanelFrontZ + kPusherMinFrontZ) * 0.5f;
    static_assert(kPusherMinFrontZ - kBackPanelFrontZ > 8.0f, "投入位置の前後にコイン（幅5cm）が収まる余裕が要る");
    // 投入位置の高さ。プッシャー上面のすぐ上から落とす。
    // 物理エンジンの接触判定の許容値はメートル単位向けで、cm単位のこのゲームでは高い所から落とすと
    // 1ステップで数cm進んでプッシャーにめり込み、そのまま背面パネルの下へ運ばれてしまうため
    inline constexpr float kLaunchY = kPusherTopY + 1.5f;
    inline constexpr float kLaunchMarkerY = kLaunchY + 4.0f;    // 投入位置の目印の高さ（積もったコインに埋もれないよう少し上）
}    // namespace FruitMagic::Layout

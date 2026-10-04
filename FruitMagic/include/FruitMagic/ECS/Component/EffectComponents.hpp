//----------------------------------------------------------------------------
//! @file   EffectComponents.hpp
//! @brief  演出（光の粒・画面の光・ぽよん・ふわっと浮く文字）のコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 光の粒1つです。ワールド空間の加算スプライトと一緒に付け、EffectsSystem が動かして寿命で消します。
    struct SparkleParticleComponent {
        hlslpp::float3 velocity = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 速度（cm/秒）
        hlslpp::float3 color    = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 色
        float          gravity  = 0.0f;                                // 下向きの加速度（cm/秒^2）
        float          life     = 1.0f;                                // 残りの寿命（秒）
        float          maxLife  = 1.0f;                                // 最初の寿命（秒）
        float          size     = 3.0f;                                // 最初の大きさ（cm）
    };

    //! 画面全体を一瞬光らせる板です。画面スプライト（加算）と一緒に付けます。EffectsSystem が明るさを下げていきます。
    struct ScreenFlashComponent {
        hlslpp::float4 color = hlslpp::float4(0.0f, 0.0f, 0.0f, 0.0f);    // 今の色と強さ（a が 0 で見えない）
        hlslpp::float3 fullScale = hlslpp::float3(1.0f, 1.0f, 1.0f);     // 画面いっぱいにするスケール
    };

    //! ぽよんと膨らんでから元の大きさに戻る見た目の動きです（魔法「おおきくなーれ」）。見た目のスケールだけを変えます。
    struct PopScaleComponent {
        hlslpp::float3 baseScale = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 元のスケール
        float          elapsed   = 0.0f;                                // 始まってからの時間（秒）
        float          duration  = 0.5f;                                // 動きの長さ（秒）
        float          amount    = 0.3f;                                // いちばん膨らんだときに足す割合
    };

    //! 台のプッシャーの目印です（魔法「ふくらむ」の間、輪郭をピンクに光らせるのに使う）。
    struct PusherComponent {
        bool glowing = false;    // 今「ふくらむ」で輪郭を光らせているか（EffectsSystem が書く）
    };

    //! 払い出し口に落ちたときにふわっと浮かんで消える文字です。FontComponent と WorldAnchorComponent と一緒に付けます。
    struct PopupComponent {
        hlslpp::float4 color   = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 文字の色（a は寿命に合わせて下げる）
        float          life    = 1.2f;                                     // 残りの寿命（秒）
        float          maxLife = 1.2f;                                     // 最初の寿命（秒）
        float          rise    = 12.0f;                                    // 寿命の間に浮かぶ高さ（cm）
    };
}    // namespace FruitMagic::ECS

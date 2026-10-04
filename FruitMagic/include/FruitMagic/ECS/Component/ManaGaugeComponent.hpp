//----------------------------------------------------------------------------
//! @file   ManaGaugeComponent.hpp
//! @brief  マナゲージの中身のバーを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! マナゲージの中身のバーです。SpriteComponent と一緒に付け、HudSystem が幅を変えます。
    //! @note 画面スプライトの位置は中心なので、左端を固定したまま伸び縮みさせるため左端と全幅を持つ
    struct ManaGaugeComponent {
        float left        = 0.0f;    // バーの左端（画面ピクセル）
        float fullWidth   = 0.0f;    // 満タンのときの幅（画面ピクセル）
        float height      = 0.0f;    // バーの高さ（画面ピクセル）
        float textureSize = 1.0f;    // 使っているテクスチャの1辺のピクセル数（スケールの換算用）
    };
}    // namespace FruitMagic::ECS

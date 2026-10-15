//----------------------------------------------------------------------------
//! @file   ManaGaugeComponent.hpp
//! @brief  マナの玉の周りの輪（ゲージの中身）を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! マナの玉の周りの輪（ゲージの中身）です。円形の塗り（SpriteFillMode::Radial）の SpriteComponent と一緒に付け、
    //! HudSystem がマナの割合だけ真上から時計回りに塗ります。
    struct ManaGaugeComponent {
        float startAngle = 0.0f;    // 塗り始めの角度（度。0 が真上）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   AssetPaths.hpp
//! @brief  コードから直接読むアセットのパス
//! @detail ロード画面の先読み（AssetPreloader）と実際に使う側で同じパスを指すよう、ここにまとめます。
//!         定義 JSON から決まるもの（果物のモデル・効果音）はここには置きません。
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::AssetPaths
namespace FruitMagic::AssetPaths {
    inline constexpr const char* kBlockModel     = "Assets/Models/Block.fbx";        // 台の部品とコイン・果物（箱型）の見た目
    inline constexpr const char* kBallModel      = "Assets/Models/Ball.fbx";         // 飾りの球・果物（球型）の見た目
    inline constexpr const char* kWhiteTexture   = "Assets/Textures/White.png";      // 画面スプライト用の白い小さな画像
    inline constexpr const char* kSparkleTexture = "Assets/Textures/Sparkle.png";    // 光の粒（きらめき）
    inline constexpr const char* kGlowTexture    = "Assets/Textures/Glow.png";       // 光の粒（ぼんやり）
    inline constexpr const char* kRingTexture    = "Assets/Textures/Ring.png";       // 円形のゲージ（おすそわけ待ち）

    //! 白い画像の1辺のピクセル数です。
    inline constexpr float kWhiteTextureSize = 8.0f;

    //! リングの画像の1辺のピクセル数です（Tools/GenerateTextures.py の SIZE）。
    inline constexpr float kRingTextureSize = 64.0f;
}    // namespace FruitMagic::AssetPaths

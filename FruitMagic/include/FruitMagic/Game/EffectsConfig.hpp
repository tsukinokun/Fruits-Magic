//----------------------------------------------------------------------------
//! @file   EffectsConfig.hpp
//! @brief  光の粒（キラキラ）と画面の光り方の設定
//! @detail Assets/Data/Effects.json から読み込みます。出来事ごとの「プリセット」に、粒の数・色・速さ・寿命・
//!         大きさと、画面全体を一瞬光らせる色を書きます。コードはプリセットの名前で出すだけです。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>
#include <unordered_map>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 光の粒の画像の種類です。
    enum class SparkleTexture {
        Sparkle,    // 星形（キラッ）。Assets/Textures/Sparkle.png
        Glow,       // ぼかした円（ふわっ）。Assets/Textures/Glow.png
    };

    //! 光の粒をまとめて出す1回分の設定です。
    struct EffectPreset {
        SparkleTexture              texture = SparkleTexture::Sparkle;   // 粒の画像
        int                         count   = 10;                        // 粒の数
        float                       speed   = 10.0f;                     // 横方向へ飛び散る速さの最大（cm/秒）
        float                       up      = 10.0f;                     // 上向きの初速（cm/秒）
        float                       gravity = 8.0f;                      // 下向きの加速度（cm/秒^2）
        float                       life    = 1.0f;                      // 寿命（秒）
        float                       size    = 3.0f;                      // 大きさ（cm）
        float                       spread  = 2.0f;                      // 出す位置のばらつき（中心からの距離、cm）
        std::vector<hlslpp::float3> colors;                              // 色（粒ごとに順番に使う）
        hlslpp::float4              flash   = hlslpp::float4(0, 0, 0, 0);  // 画面全体を光らせる色と強さ（a が 0 なら光らせない）
    };

    //! 光の粒の設定です。Registry のコンテキストに置きます。
    class EffectsConfig {
    public:

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Effects.json）
        //! @return 読み込めたら true（読めなくても動くが、粒は出ない）
        bool Load(const std::string& path);

        //! プリセットを探します。
        //! @param  [in] name プリセットの名前
        //! @return プリセット。無ければ nullptr
        const EffectPreset* Find(const std::string& name) const;

        //! 同時に出しておける粒の数の上限を返します。
        //! @return 上限
        int MaxParticles() const { return m_maxParticles; }

    private:
        std::unordered_map<std::string, EffectPreset> m_presets;               // 名前ごとのプリセット
        int                                           m_maxParticles = 400;    // 同時に出しておける粒の数の上限
    };
}    // namespace FruitMagic

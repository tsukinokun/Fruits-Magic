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

    //! 光の粒の出し方・動き方の共通の設定です（Effects.json の "system"）。
    struct EffectsMotion {
        hlslpp::float3 castPosition   = hlslpp::float3(0.0f, 6.0f, 5.0f);    // 魔法を撃ったときの粒を出す位置（台の中央の少し上）
        float          dropEffectY    = -12.0f;    // 払い出し口に落ちた物の演出を出す高さ（落ちたと判定する高さより少し上。手前の縁の下から見える所）
        float          dropEffectAhead = 4.0f;     // その位置を台の手前端からどれだけ手前にするか
        float          idleInterval   = 0.6f;      // 色違い・金色の果物から粒をこぼす間隔（秒）
        float          idleMinY       = -2.0f;     // これより下の果物（台から落ちている途中）からはこぼさない
        float          idleRise       = 3.0f;      // こぼす粒を果物の中心からどれだけ上に出すか
        float          flashFadeSpeed = 2.5f;      // 画面の光が消えるまでの速さ（強さ / 秒）
        float          drag           = 1.5f;      // 横方向の速さが落ちていく割合（/ 秒）。ふわっと止まるように
        float          startHeight    = 1.5f;      // 粒を出す高さのばらつき（cm）
        float          upMin          = 0.5f;      // 上向きの初速の最小（プリセットの up に対する割合。残りはランダム）
        float          lifeMin        = 0.7f;      // 寿命の最小（プリセットの life に対する割合。残りはランダム）
        float          sizeMin        = 0.7f;      // 大きさの最小（プリセットの size に対する割合）
        float          sizeRange      = 0.6f;      // 大きさのばらつき（同じく割合）
        float          shrinkMin      = 0.4f;      // 寿命の終わりの大きさ（最初の大きさに対する割合）
    };

    //! 台の手前に出る「+N」「〇〇！」の文字の設定です（Effects.json の "popup"）。
    struct PopupStyle {
        float          coinGatherSeconds = 0.25f;    // コインをまとめる時間（秒）。この間に落ちた分を1つの「+N」にする
        int            maxPopups         = 10;       // 同時に出しておく上限（多すぎると読めないので、超えたら古いものから消す）
        float          y                 = 6.0f;     // 出る高さ（台の手前の縁の少し上。これより下は画面下の魔法ボタンに隠れる）
        float          rise              = 12.0f;    // 寿命の間に浮かぶ高さ（cm）
        float          fadeStart         = 2.5f;     // 薄くし始める速さ（寿命の残りの割合 × これ が 1 を下回ると薄くなる）
        hlslpp::float4 coinColor         = hlslpp::float4(1.0f, 0.9f, 0.35f, 1.0f);    // コインの文字の色
        hlslpp::float3 outlineColor      = hlslpp::float3(0.2f, 0.08f, 0.12f);         // 文字の縁の色
        float          outlineWidth      = 2.0f;     // 文字の縁の太さ
        float          coinLife          = 1.0f;     // コインの文字の寿命（秒）
        float          coinScaleStep     = 0.05f;    // 1枚ごとに大きくする量
        float          coinScaleMaxBonus = 0.5f;     // 大きくする量の上限
        float          fruitLife         = 1.6f;     // 果物の文字の寿命（秒）
        float          fruitLighten      = 0.35f;    // 果物の色を白へ寄せる割合（読みやすく）
        float          fruitScale        = 0.95f;    // 通常の果物の文字の大きさ
        float          variantScale      = 1.1f;     // 色違い・金色の果物の文字の大きさ
    };

    //! 光の粒の設定です。Registry のコンテキストに置きます。
    class EffectsConfig {
    public:
        EffectsMotion motion;    // 粒の出し方・動き方
        PopupStyle    popup;     // 台の手前に出る文字

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

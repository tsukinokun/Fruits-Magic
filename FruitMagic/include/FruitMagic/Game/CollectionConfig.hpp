//----------------------------------------------------------------------------
//! @file   CollectionConfig.hpp
//! @brief  果物のバリエーション（通常・色違い・金色）と図鑑ボーナスの設定
//! @detail Assets/Data/Collection.json から読み込みます。バリエーションを足すと図鑑の列も増えます。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <random>
#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    struct FruitDef;    // 前方宣言

    //! バリエーションの色の決め方です。
    enum class VariantColorMode {
        Base,     // 果物の色（FruitDef::color）
        Shiny,    // 果物の色違いの色（FruitDef::shinyColor）
        Fixed,    // バリエーションで決めた色（金色など）
    };

    //! 果物のバリエーション1つの定義です。
    struct VariantDef {
        std::string      id;                                              // 識別子
        std::wstring     name;                                            // 表示名（通常は空。「いちご（色違い）」の括弧の中）
        float            chance          = 0.0f;                          // 出る確率（先頭のバリエーションは「どれにも当たらなかったとき」なので使わない）
        int              valueMultiplier = 1;                             // 価値の倍率（転生ポイントの計算に使う）
        VariantColorMode colorMode       = VariantColorMode::Base;        // 色の決め方
        hlslpp::float3   color           = hlslpp::float3(1.0f, 0.8f, 0.2f);    // colorMode が Fixed のときの色
        float            glow            = 0.6f;                          // 輪郭の光の強さ

        //! この果物をこのバリエーションで見せる色を返します。
        //! @param  [in] fruit 果物の定義
        //! @return 表示色
        hlslpp::float3 ColorOf(const FruitDef& fruit) const;
    };

    //! バリエーションと図鑑ボーナスの設定です。Registry のコンテキストに置いて共有します。
    class CollectionConfig {
    public:

        //! 設定ファイルを読み込みます。読めなかった場合は「通常」だけの1種類になります。
        //! @param  [in] path 設定ファイル（Collection.json）
        //! @return 読み込めたら true
        bool Load(const std::string& path);

        //! バリエーションの一覧を返します（先頭が通常）。
        //! @return バリエーションの一覧
        const std::vector<VariantDef>& Variants() const { return m_variants; }

        //! バリエーションを抽選します。
        //! @param  [in,out] rng 乱数生成器
        //! @return バリエーションの添字（どれにも当たらなければ 0 = 通常）
        int PickVariant(std::mt19937& rng) const;

        //! 図鑑1枠あたりのマナ獲得の上乗せ率を返します（0.05 なら 5%）。
        //! @return 上乗せ率
        float ManaBonusPerEntry() const { return m_manaBonusPerEntry; }

        //! 果物名にバリエーション名を付けた表示名を返します（例: 「いちご（色違い）」）。
        //! @param  [in] fruit        果物の定義
        //! @param  [in] variantIndex バリエーションの添字
        //! @return 表示名
        std::wstring DisplayName(const FruitDef& fruit, int variantIndex) const;

    private:
        std::vector<VariantDef> m_variants;                    // バリエーションの一覧
        float                   m_manaBonusPerEntry = 0.05f;   // 図鑑1枠あたりのマナ獲得の上乗せ率
    };
}    // namespace FruitMagic

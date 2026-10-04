//----------------------------------------------------------------------------
//! @file   PrizeComponent.hpp
//! @brief  台から落ちると取れる物（コイン・果物）を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 景品の種類です。
    enum class PrizeKind {
        Coin,     // コイン
        Fruit,    // 果物（M2 で使う）
    };

    //! 台から落ちると取れる物です。PrizeDropSystem が落下を判定します。
    struct PrizeComponent {
        PrizeKind kind  = PrizeKind::Coin;    // 景品の種類
        int       value = 1;                  // 価値（コインは手持ちに戻る枚数、果物は FruitDef::value）
        int       fruitIndex = -1;            // 果物のとき、FruitCatalog::Fruits() の添字
        int       variantIndex = 0;           // 果物のとき、CollectionConfig::Variants() の添字（0 は通常）
    };
}    // namespace FruitMagic::ECS

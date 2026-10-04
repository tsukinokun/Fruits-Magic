//----------------------------------------------------------------------------
//! @file   PrizeDroppedEvent.hpp
//! @brief  景品が台から落ちたことを知らせるイベント
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Component/PrizeComponent.hpp>

#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 景品が落ちた場所です。
    enum class DropZone {
        Payout,    // 正面の払い出し口（手に入る）
        Gutter,    // 左右の溝（失う）
    };

    //! 景品が台から落ちたことを知らせるイベントです。PrizeDropSystem が発行します。
    //! @note 受け取った時点で prize は破棄予約済み。コンポーネントには触らないこと
    struct PrizeDroppedEvent {
        Tsukino::ECS::Entity prize = entt::null;          // 落ちた景品
        PrizeKind            kind  = PrizeKind::Coin;     // 景品の種類
        int                  value = 0;                   // 景品の価値
        int                  fruitIndex = -1;             // 果物のとき、FruitCatalog::Fruits() の添字
        DropZone             zone  = DropZone::Payout;    // 落ちた場所
        float                x     = 0.0f;                // 落ちた時点の左右位置（チェッカーの判定に使う）
    };
}    // namespace FruitMagic::ECS

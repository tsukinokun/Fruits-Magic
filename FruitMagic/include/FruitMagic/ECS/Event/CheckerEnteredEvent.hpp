//----------------------------------------------------------------------------
//! @file   CheckerEnteredEvent.hpp
//! @brief  チェッカーにコインが入ったことを知らせるイベント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! チェッカーにコインが入ったことを知らせるイベントです。CheckerSystem が発行し、RouletteSystem が受け取ります。
    struct CheckerEnteredEvent {
        float x = 0.0f;    // 入った位置のX（演出用）
    };
}    // namespace FruitMagic::ECS

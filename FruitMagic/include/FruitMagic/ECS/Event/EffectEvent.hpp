//----------------------------------------------------------------------------
//! @file   EffectEvent.hpp
//! @brief  光の粒（キラキラ）を出すことを頼むイベント
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 光の粒を出すことを頼むイベントです。EffectsSystem が Effects.json のプリセットに従って出します。
    struct EffectEvent {
        std::string    preset;                                         // プリセットの名前（Effects.json の "presets" のキー）
        hlslpp::float3 position = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 出す位置（ワールド座標）
    };
}    // namespace FruitMagic::ECS

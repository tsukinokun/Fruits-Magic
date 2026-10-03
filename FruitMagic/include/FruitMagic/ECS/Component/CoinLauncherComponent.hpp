//----------------------------------------------------------------------------
//! @file   CoinLauncherComponent.hpp
//! @brief  コインの投入口（レーン位置と連打防止）を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! コインの投入口です。このコンポーネントを持つエンティティは投入位置の目印として動きます。
    struct CoinLauncherComponent {
        float laneX     = 0.0f;     // 現在の投入位置（X）
        float laneSpeed = 40.0f;    // キー操作で動かす速さ（cm/秒）
        float interval  = 0.15f;    // 投入の最短間隔（秒）
        float cooldown  = 0.0f;     // 次に投入できるまでの残り時間（秒）
    };
}    // namespace FruitMagic::ECS

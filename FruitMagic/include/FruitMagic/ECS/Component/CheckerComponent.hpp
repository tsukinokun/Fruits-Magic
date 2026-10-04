//----------------------------------------------------------------------------
//! @file   CheckerComponent.hpp
//! @brief  チェッカー（コインが入るとルーレットが回る、左右に動く穴）を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! チェッカーです。このコンポーネントを持つエンティティが穴の目印として左右に動きます。
    //! 大きさ・範囲・周期は RouletteConfig から読みます。
    struct CheckerComponent {
        float time = 0.0f;    // 往復に使う経過時間（秒）
        float x    = 0.0f;    // 現在の穴の中心X
    };
}    // namespace FruitMagic::ECS

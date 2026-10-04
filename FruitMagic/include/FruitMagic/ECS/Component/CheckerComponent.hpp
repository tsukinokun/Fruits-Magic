//----------------------------------------------------------------------------
//! @file   CheckerComponent.hpp
//! @brief  チェッカー（コインが入るとルーレットが回る、左右に動く穴）を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! チェッカーです。このコンポーネントを持つエンティティが穴の目印として左右に動きます。
    //! 範囲・周期は RouletteConfig から、穴の幅は強化で変わる TableStats から読みます。
    struct CheckerComponent {
        float time              = 0.0f;    // 往復に使う経過時間（秒）
        float x                 = 0.0f;    // 現在の穴の中心X
        float scalePerHalfWidth = 0.0f;    // 目印の X スケール ÷ 穴の半幅。穴の幅が変わったら目印も合わせて伸ばす（0 なら伸ばさない）
    };
}    // namespace FruitMagic::ECS

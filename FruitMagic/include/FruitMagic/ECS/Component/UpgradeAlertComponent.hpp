//----------------------------------------------------------------------------
//! @file   UpgradeAlertComponent.hpp
//! @brief  強化のボタンの「!」の印（買える強化があるときだけ出す）のコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 強化のボタンの「!」の印です。UIVisibilityComponent と一緒に付け、
    //! HudSystem が買える強化があるときだけ見せて、呼吸させます。
    struct UpgradeAlertComponent {
        bool shown = false;    // 前のフレームに見せていたか（出始めにポップさせるため）
    };
}    // namespace FruitMagic::ECS

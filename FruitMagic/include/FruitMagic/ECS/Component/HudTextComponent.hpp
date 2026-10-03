//----------------------------------------------------------------------------
//! @file   HudTextComponent.hpp
//! @brief  HUD のテキストが何を表示するかを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! HUD に表示する内容の種類です。
    enum class HudTextKind {
        Coins,          // 手持ちのコイン枚数
        DropPopup,      // 直近の払い出し（+1 / 溝）
        ControlsHint,   // 操作説明
    };

    //! HUD のテキストです。FontComponent と一緒に付け、HudSystem が文字列を更新します。
    struct HudTextComponent {
        HudTextKind kind = HudTextKind::Coins;    // 表示する内容
    };
}    // namespace FruitMagic::ECS

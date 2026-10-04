//----------------------------------------------------------------------------
//! @file   MagicButtonComponent.hpp
//! @brief  画面下の魔法ボタンを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法ボタンです。SpriteComponent と PointerTargetComponent と一緒に付けます。
    //! クリックは MagicInputSystem が、色と文字は HudSystem が扱います。
    struct MagicButtonComponent {
        int                  slot  = 1;            // 枠の番号（1〜kMagicSlotCount。数字キーと同じ）
        Tsukino::ECS::Entity label = entt::null;   // ボタンの上の文字（FontComponent を持つエンティティ）
    };
}    // namespace FruitMagic::ECS

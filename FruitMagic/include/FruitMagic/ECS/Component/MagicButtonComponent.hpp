//----------------------------------------------------------------------------
//! @file   MagicButtonComponent.hpp
//! @brief  画面下の魔法ボタンを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法ボタン（丸いボタン）です。SpriteComponent と PointerTargetComponent と一緒に付けます。
    //! クリックは MagicInputSystem が、色・アイコン・残り時間の輪・文字は HudSystem が扱います。
    struct MagicButtonComponent {
        int                          slot          = 1;             // 枠の番号（1〜kMagicSlotCount。数字キーと同じ）
        Tsukino::ECS::Entity         label         = entt::null;    // ボタンの下の名前とマナ（FontComponent を持つエンティティ）
        Tsukino::ECS::Entity         icon          = entt::null;    // ボタンの上のアイコン（SpriteComponent）
        Tsukino::ECS::Entity         ring          = entt::null;    // 効果中の残り時間の輪（円形の塗りの SpriteComponent と UIVisibilityComponent）
        Tsukino::Asset::AssetHandle  iconTexture;                   // 覚えた魔法のアイコン
        Tsukino::Asset::AssetHandle  lockedTexture;                 // まだ覚えていないときのアイコン
    };
}    // namespace FruitMagic::ECS

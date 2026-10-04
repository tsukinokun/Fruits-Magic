//----------------------------------------------------------------------------
//! @file   MenuComponent.hpp
//! @brief  画面（図鑑・強化）を構成する要素と、画面を開閉するボタンのコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/MenuState.hpp>

#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Core/Input/KeyCodes.hpp>

#include <hlsl++.h>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面の要素です。MenuSystem が開閉に合わせて表示・非表示を切り替えます。
    //! @note 画面スプライトはスケール 0 で描画も当たり判定もされず、文字は空文字で描画されない（エンジンの描画システムの挙動）ので、
    //!       閉じている間はスケールを 0 に、文字を空にする。開くときに戻すスケールを覚えておく
    struct MenuPageComponent {
        MenuKind       menu      = MenuKind::Zukan;                     // 属する画面
        hlslpp::float3 openScale = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 開いているときのスケール（スプライト用）
        std::wstring   text;                                            // 決まった文字（タイトル・見出し）。空なら各画面のシステムが書く
    };

    //! 画面を開閉するボタンです。SpriteComponent と PointerTargetComponent と一緒に付けます。
    struct MenuButtonComponent {
        MenuKind                 menu       = MenuKind::Zukan;                 // 開閉する画面
        Tsukino::Input::KeyCode  key        = Tsukino::Input::KeyCode::Tab;    // 同じ操作をするキー
        Tsukino::ECS::Entity     label      = entt::null;                      // ボタンの上の文字（FontComponent を持つエンティティ）
        std::wstring             closedText;                                   // 閉じているときの文字（「図鑑 (Tab)」）
        std::wstring             openText;                                     // 開いているときの文字（「閉じる (Tab)」）
    };
}    // namespace FruitMagic::ECS

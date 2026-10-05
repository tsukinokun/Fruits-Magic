//----------------------------------------------------------------------------
//! @file   OptionsDialogComponent.hpp
//! @brief  データ消去の確認ウィンドウの部品のコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 確認ウィンドウの部品の種類です。
    enum class OptionsDialogPart {
        Dimmer,         // 後ろを暗くする板（画面全体。クリックを受け止める）
        Window,         // ウィンドウの背景
        Step,           // 「確認 n / 3」
        Message,        // 1行目
        Note,           // 2行目
        Yes,            // 消すボタン（最後の回で実行）
        No,             // やめるボタン
        ButtonLabel,    // ボタンの上の文字
    };

    //! 確認ウィンドウの部品です。MenuPageComponent は付けず、確認を出している間だけ OptionsSystem が表示します
    //! （スプライトはスケール 0 で隠し、文字は空にする）。ボタンは PointerTargetComponent も持ち、label にボタンの上の文字を入れます。
    struct OptionsDialogComponent {
        OptionsDialogPart    part       = OptionsDialogPart::Window;    // 種類
        Tsukino::ECS::Entity label      = entt::null;                   // ボタンの上の文字（ボタンのとき）
        hlslpp::float3       shownScale = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 出しているときのスケール（スプライトのとき）
    };
}    // namespace FruitMagic::ECS

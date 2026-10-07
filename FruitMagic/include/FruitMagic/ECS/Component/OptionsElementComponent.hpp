//----------------------------------------------------------------------------
//! @file   OptionsElementComponent.hpp
//! @brief  オプション画面の中身が変わる要素（ボタン・音量の表示）のコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! オプション画面の要素の種類です。
    enum class OptionsElementKind {
        BgmDown,      // BGM の音量を下げるボタン
        BgmUp,        // BGM の音量を上げるボタン
        BgmValue,     // BGM の音量の表示（「70%」）
        SeDown,       // 効果音の音量を下げるボタン
        SeUp,         // 効果音の音量を上げるボタン
        SeValue,      // 効果音の音量の表示
        Mute,         // 消音の切り替えボタン
        Hint,         // 画面下の操作説明の表示の切り替えボタン
        CutIn,        // 果物が取れたときのカットインの切り替えボタン
        Reset,        // データを消して最初から（2回押すと実行）
        Quit,         // ゲームを終了
    };

    //! オプション画面の要素です。MenuPageComponent と一緒に付け、オプション画面が開いている間 OptionsSystem が更新します。
    //! ボタンは SpriteComponent と PointerTargetComponent も持ち、label にボタンの上の文字を入れます。
    struct OptionsElementComponent {
        OptionsElementKind   kind  = OptionsElementKind::BgmValue;    // 種類
        Tsukino::ECS::Entity label = entt::null;                      // ボタンの上の文字（ボタンのとき）
    };
}    // namespace FruitMagic::ECS

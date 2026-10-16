//----------------------------------------------------------------------------
//! @file   OptionsSystem.cpp
//! @brief  オプション画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/OptionsSystem.hpp>

#include <FruitMagic/ECS/Component/OptionsElementComponent.hpp>
#include <FruitMagic/Game/ConfirmState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/SceneRequest.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UiMotion.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cmath>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    namespace {

        // カーソルが重なっているボタンの色を少し明るくする
        hlslpp::float4 Brighten(const hlslpp::float4& color) {
            return hlslpp::float4(color.xyz + (hlslpp::float3(1.0f, 1.0f, 1.0f) - color.xyz) * 0.2f, color.w);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! ボタンの入力を読み、オプション画面を更新します。
    //----------------------------------------------------------------------------
    void OptionsSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<Settings>() || !registry.HasContext<ConfirmState>())
            return;
        if(registry.GetContext<MenuState>().IsOpen(MenuKind::Options))
            UpdatePage(registry, registry.GetContext<ConfirmState>().IsConfirming());
    }

    //----------------------------------------------------------------------------
    //! オプション画面のボタンのクリックを処理し、ボタンの文字・色と音量の表示を更新します。
    //----------------------------------------------------------------------------
    void OptionsSystem::UpdatePage(Tsukino::ECS::Registry& registry, bool confirming) {
        Settings&       settings = registry.GetContext<Settings>();
        const UiConfig& ui       = GetUiConfig(registry);
        const Texts&    texts    = GetTexts(registry);

        //--------------------------------------------------------------
        // クリックされたボタン（確認ウィンドウを出している間は受け付けない）
        //--------------------------------------------------------------
        bool clicked = false;
        bool changed = false;
        registry.View<OptionsElementComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, OptionsElementComponent& element, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                if(!pointer.clicked || clicked || confirming)
                    return;
                clicked = true;
                switch(element.kind) {
                    case OptionsElementKind::BgmDown: Settings::StepVolume(settings.bgmVolume, -1); changed = true; break;
                    case OptionsElementKind::BgmUp: Settings::StepVolume(settings.bgmVolume, 1); changed = true; break;
                    case OptionsElementKind::SeDown: Settings::StepVolume(settings.seVolume, -1); changed = true; break;
                    case OptionsElementKind::SeUp: Settings::StepVolume(settings.seVolume, 1); changed = true; break;
                    case OptionsElementKind::Mute: settings.muted = !settings.muted; changed = true; break;
                    case OptionsElementKind::Hint: settings.showControlsHint = !settings.showControlsHint; changed = true; break;
                    case OptionsElementKind::CutIn: settings.showCutIn = !settings.showCutIn; changed = true; break;
                    case OptionsElementKind::Reset: registry.GetContext<ConfirmState>().Open(ConfirmPurpose::Reset, MenuKind::Options); break;    // 確認ウィンドウを出す
                    case OptionsElementKind::Quit:
                        if(registry.HasContext<SceneRequest>())
                            registry.GetContext<SceneRequest>().quit = true;
                        break;
                    default:
                        break;
                }
            });
        if(changed)
            settings.Save();

        //--------------------------------------------------------------
        // ボタンの色・文字と音量の表示
        //--------------------------------------------------------------
        auto percent = [&](float volume) { return texts.Format("options.percent", {{"n", std::to_wstring(static_cast<int>(std::lround(volume * 100.0f)))}}); };

        registry.View<OptionsElementComponent>().each([&](Tsukino::ECS::Entity entity, OptionsElementComponent& element) {
            // 表示だけの要素（音量の％）
            if(element.kind == OptionsElementKind::BgmValue || element.kind == OptionsElementKind::SeValue) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity))
                    font->text = percent(element.kind == OptionsElementKind::BgmValue ? settings.bgmVolume : settings.seVolume);
                return;
            }

            // ボタン: 色（種類ごと。カーソルが重なっていれば明るく）と文字
            auto*          sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
            auto*          pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
            const bool     hovered = pointer && pointer->hovered;
            hlslpp::float4 color   = hovered ? ui.optionsButtonHover : ui.optionsButtonFill;
            bool           tinted  = false;    // 種類ごとの色を持つボタン（カーソルが重なったら明るくする）
            std::wstring   text;
            switch(element.kind) {
                case OptionsElementKind::BgmDown:
                case OptionsElementKind::SeDown: text = texts.Get("options.minus"); break;
                case OptionsElementKind::BgmUp:
                case OptionsElementKind::SeUp: text = texts.Get("options.plus"); break;
                case OptionsElementKind::Mute:
                    text   = texts.Get(settings.muted ? "options.on" : "options.off");
                    color  = settings.muted ? ui.optionsToggleOn : ui.optionsToggleOff;
                    tinted = true;
                    break;
                case OptionsElementKind::Hint:
                    text   = texts.Get(settings.showControlsHint ? "options.show" : "options.hide");
                    color  = settings.showControlsHint ? ui.optionsToggleOn : ui.optionsToggleOff;
                    tinted = true;
                    break;
                case OptionsElementKind::CutIn:
                    text   = texts.Get(settings.showCutIn ? "options.on" : "options.off");
                    color  = settings.showCutIn ? ui.optionsToggleOn : ui.optionsToggleOff;
                    tinted = true;
                    break;
                case OptionsElementKind::Reset:
                    text   = texts.Get("options.reset");
                    color  = ui.optionsResetColor;
                    tinted = true;
                    break;
                case OptionsElementKind::Quit:
                    text   = texts.Get("options.quit");
                    color  = ui.optionsQuitColor;
                    tinted = true;
                    break;
                default:
                    break;
            }
            if(hovered && tinted)
                color = Brighten(color);

            if(sprite)
                sprite->tintColor = color;
            if(element.label != entt::null) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(element.label))
                    font->text = text;
            }
        });
    }

}    // namespace FruitMagic::ECS

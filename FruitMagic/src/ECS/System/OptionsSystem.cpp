//----------------------------------------------------------------------------
//! @file   OptionsSystem.cpp
//! @brief  オプション画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/OptionsSystem.hpp>

#include <FruitMagic/ECS/Component/OptionsDialogComponent.hpp>
#include <FruitMagic/ECS/Component/OptionsElementComponent.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/OptionsState.hpp>
#include <FruitMagic/Game/SceneRequest.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

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
    //! ボタンの入力を読み、オプション画面と確認ウィンドウを更新します。
    //----------------------------------------------------------------------------
    void OptionsSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<Settings>() || !registry.HasContext<OptionsState>())
            return;

        OptionsState& state = registry.GetContext<OptionsState>();
        if(!registry.GetContext<MenuState>().IsOpen(MenuKind::Options))
            state.confirmStep = 0;    // 閉じたら確認は取り消す

        // 確認の回が変わったら、消すボタンを押せるまでの待ちを数え直す
        if(state.confirmStep != m_shownStep) {
            m_shownStep = state.confirmStep;
            m_stepTimer = 0.0f;
        }
        m_stepTimer += deltaTime;

        if(registry.GetContext<MenuState>().IsOpen(MenuKind::Options))
            UpdatePage(registry, state.IsConfirming());
        UpdateDialog(registry);
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
                    case OptionsElementKind::Reset: registry.GetContext<OptionsState>().confirmStep = 1; break;    // 確認ウィンドウを出す
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

    //----------------------------------------------------------------------------
    //! 確認ウィンドウのボタンのクリックを処理し、表示を更新します。
    //----------------------------------------------------------------------------
    void OptionsSystem::UpdateDialog(Tsukino::ECS::Registry& registry) {
        OptionsState&   state = registry.GetContext<OptionsState>();
        const UiConfig& ui    = GetUiConfig(registry);
        const Texts&    texts = GetTexts(registry);

        //--------------------------------------------------------------
        // クリック: 消す → 次の回へ（最後の回なら実行）。やめる → 閉じる。
        // 出てすぐの間は消すを受け付けない（連打で3回通らないように）
        //--------------------------------------------------------------
        const bool yesReady = m_stepTimer >= ui.confirmDelay;
        if(state.IsConfirming()) {
            bool yes = false;
            bool no  = false;
            registry.View<OptionsDialogComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
                [&](Tsukino::ECS::Entity, OptionsDialogComponent& part, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                    if(!pointer.clicked)
                        return;
                    if(part.part == OptionsDialogPart::Yes && yesReady)
                        yes = true;
                    else if(part.part == OptionsDialogPart::No)
                        no = true;
                });
            if(no) {
                state.confirmStep = 0;
            } else if(yes) {
                if(state.confirmStep >= kResetConfirmSteps) {
                    state.confirmStep = 0;
                    if(registry.HasContext<SceneRequest>())
                        registry.GetContext<SceneRequest>().restart = true;
                    Tsukino::Core::Log::Info("OptionsSystem: restart requested.");
                } else {
                    ++state.confirmStep;
                    Tsukino::Core::Log::Info("OptionsSystem: reset confirm " + std::to_string(state.confirmStep) + "/" + std::to_string(kResetConfirmSteps) + ".");
                }
            }
        }

        //--------------------------------------------------------------
        // 表示（出していない間は、スプライトはスケール 0、文字は空）
        //--------------------------------------------------------------
        const bool        shown = state.IsConfirming();
        const std::string n     = std::to_string(state.confirmStep);
        registry.View<OptionsDialogComponent>().each([&](Tsukino::ECS::Entity entity, OptionsDialogComponent& part) {
            if(auto* transform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(entity);
               transform && registry.HasComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                transform->scale = shown ? part.shownScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                transform->dirty = true;
            }

            std::wstring text;
            if(shown) {
                switch(part.part) {
                    case OptionsDialogPart::Step: text = texts.Format("options.confirm.step", {{"n", std::wstring(n.begin(), n.end())}}); break;
                    case OptionsDialogPart::Message: text = texts.Get("options.confirm.message" + n); break;
                    case OptionsDialogPart::Note: text = texts.Get("options.confirm.note" + n); break;
                    default: break;
                }
            }
            if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity); font && part.part != OptionsDialogPart::ButtonLabel)
                font->text = text;

            // ボタン: 色と上の文字
            if(part.part != OptionsDialogPart::Yes && part.part != OptionsDialogPart::No)
                return;
            auto*          pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
            const bool     hovered = pointer && pointer->hovered;
            hlslpp::float4 color   = ui.confirmNoColor;
            std::wstring   label;
            if(part.part == OptionsDialogPart::Yes) {
                color = yesReady ? ui.confirmYesColor : ui.confirmYesWaitColor;
                label = shown ? texts.Get("options.confirm.yes" + n) : std::wstring();
                if(hovered && yesReady)
                    color = Brighten(color);
            } else {
                label = shown ? texts.Get("options.confirm.no") : std::wstring();
                if(hovered)
                    color = Brighten(color);
            }
            if(auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity))
                sprite->tintColor = color;
            if(part.label != entt::null) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(part.label))
                    font->text = label;
            }
        });
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   ConfirmDialogSystem.cpp
//! @brief  確認ウィンドウのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ConfirmDialogSystem.hpp>

#include <FruitMagic/ECS/Component/ConfirmDialogComponent.hpp>
#include <FruitMagic/Game/ConfirmState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/MoveRules.hpp>
#include <FruitMagic/Game/SceneRequest.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UiMotion.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    namespace {

        // カーソルが重なっているボタンの色を少し明るくする
        hlslpp::float4 Brighten(const hlslpp::float4& color) {
            return hlslpp::float4(color.xyz + (hlslpp::float3(1.0f, 1.0f, 1.0f) - color.xyz) * 0.2f, color.w);
        }

        // 確認の文言のキーの頭（Texts.json）
        std::string TextPrefix(ConfirmPurpose purpose) {
            return (purpose == ConfirmPurpose::Move) ? "move.confirm." : "options.confirm.";
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! ボタンの入力を読み、確認ウィンドウを更新します。
    //----------------------------------------------------------------------------
    void ConfirmDialogSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<ConfirmState>() || !registry.HasContext<MenuState>())
            return;

        ConfirmState&   state = registry.GetContext<ConfirmState>();
        const UiConfig& ui    = GetUiConfig(registry);
        const Texts&    texts = GetTexts(registry);

        // 出した画面が閉じたら取り消す
        if(state.IsConfirming() && !registry.GetContext<MenuState>().IsOpen(state.owner))
            state.Close();

        //--------------------------------------------------------------
        // 確認の回が変わったら、「はい」を押せるまでの待ちを数え直す。
        // 出すたびに（回が進んだときも）ウィンドウをポップさせ、最初に出したときは暗い板をフェードインさせる
        //--------------------------------------------------------------
        if(state.step != m_shownStep) {
            if(state.step > 0) {
                const bool firstShown = m_shownStep == 0;
                registry.View<ConfirmDialogComponent>().each([&](Tsukino::ECS::Entity entity, ConfirmDialogComponent& part) {
                    if(part.part == ConfirmDialogPart::Window)
                        PlayPanelOpen(registry, entity);
                    else if(part.part == ConfirmDialogPart::Dimmer && firstShown)
                        PlayFadeIn(registry, entity);
                });
            }
            m_shownStep = state.step;
            m_stepTimer = 0.0f;
        }
        m_stepTimer += deltaTime;

        //--------------------------------------------------------------
        // クリック: はい → 次の回へ（最後の回なら実行）。やめる → 閉じる
        //--------------------------------------------------------------
        const bool yesReady = m_stepTimer >= ui.confirmDelay;
        if(state.IsConfirming()) {
            bool yes = false;
            bool no  = false;
            registry.View<ConfirmDialogComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
                [&](Tsukino::ECS::Entity, ConfirmDialogComponent& part, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                    if(!pointer.clicked)
                        return;
                    if(part.part == ConfirmDialogPart::Yes && yesReady)
                        yes = true;
                    else if(part.part == ConfirmDialogPart::No)
                        no = true;
                });
            if(no) {
                state.Close();
            } else if(yes && state.step < ConfirmSteps(state.purpose)) {
                ++state.step;
                Tsukino::Core::Log::Info("ConfirmDialogSystem: confirm " + std::to_string(state.step) + "/" + std::to_string(ConfirmSteps(state.purpose)) + ".");
            } else if(yes) {
                if(registry.HasContext<SceneRequest>()) {
                    SceneRequest& request = registry.GetContext<SceneRequest>();
                    if(state.purpose == ConfirmPurpose::Reset)
                        request.restart = true;
                    else if(state.purpose == ConfirmPurpose::Move)
                        request.move = true;
                }
                Tsukino::Core::Log::Info(std::string("ConfirmDialogSystem: ") + (state.purpose == ConfirmPurpose::Move ? "move" : "restart") + " requested.");
                state.Close();
            }
        }

        //--------------------------------------------------------------
        // 表示（出していない間は、スプライトはスケール 0、文字は空）
        //--------------------------------------------------------------
        const bool         shown  = state.IsConfirming();
        const std::string  prefix = TextPrefix(state.purpose);
        const std::string  n      = std::to_string(state.step);
        const std::wstring seeds  = shown && state.purpose == ConfirmPurpose::Move ? std::to_wstring(MoveRules::SeedsIfMovedNow(registry)) : std::wstring();
        auto line = [&](const std::string& key) { return texts.Format(prefix + key, {{"seeds", seeds}}); };

        registry.View<ConfirmDialogComponent>().each([&](Tsukino::ECS::Entity entity, ConfirmDialogComponent& part) {
            if(auto* transform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(entity);
               transform && registry.HasComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                transform->scale = shown ? part.shownScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                transform->dirty = true;
            }

            std::wstring text;
            if(shown) {
                switch(part.part) {
                    case ConfirmDialogPart::Step:
                        // 1回だけの確認では何回目かを出さない
                        if(ConfirmSteps(state.purpose) > 1)
                            text = texts.Format(prefix + "step", {{"n", std::wstring(n.begin(), n.end())}});
                        break;
                    case ConfirmDialogPart::Message: text = line("message" + n); break;
                    case ConfirmDialogPart::Note: text = line("note" + n); break;
                    default: break;
                }
            }
            if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity); font && part.part != ConfirmDialogPart::ButtonLabel)
                font->text = text;

            // ボタン: 色と上の文字
            if(part.part != ConfirmDialogPart::Yes && part.part != ConfirmDialogPart::No)
                return;
            auto*          pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
            const bool     hovered = pointer && pointer->hovered;
            hlslpp::float4 color   = ui.confirmNoColor;
            std::wstring   label;
            if(part.part == ConfirmDialogPart::Yes) {
                color = yesReady ? ui.confirmYesColor : ui.confirmYesWaitColor;
                label = shown ? line("yes" + n) : std::wstring();
                if(hovered && yesReady)
                    color = Brighten(color);
            } else {
                label = shown ? line("no") : std::wstring();
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

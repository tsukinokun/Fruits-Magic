//----------------------------------------------------------------------------
//! @file   MenuSystem.cpp
//! @brief  画面の開閉のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/MenuSystem.hpp>

#include <FruitMagic/ECS/Component/MenuComponent.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/ECS/Component/FruitIconComponent.hpp>
#include <FruitMagic/Game/ConfirmState.hpp>
#include <FruitMagic/Game/FruitIcon.hpp>
#include <FruitMagic/Game/UiMotion.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScrollViewComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 開閉の入力を読み、画面の要素の表示を切り替えます。
    //----------------------------------------------------------------------------
    void MenuSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>())
            return;

        MenuState&                                 state = registry.GetContext<MenuState>();
        Tsukino::EngineIntegration::EngineContext* ctx   = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        Tsukino::Input::InputSystem*               input = ctx ? ctx->inputSystem : nullptr;

        // 確認ウィンドウを出している間は、Esc で確認だけ閉じ、ほかの開閉とスクロールは受け付けない
        ConfirmState* confirm    = registry.HasContext<ConfirmState>() ? &registry.GetContext<ConfirmState>() : nullptr;
        const bool    confirming = confirm && confirm->IsConfirming();
        if(confirming && input && input->IsKeyPressed(Tsukino::Input::KeyCode::Escape))
            confirm->Close();

        //--------------------------------------------------------------
        // 開閉（ボタンのクリック、またはボタンに割り当てたキー）。
        // 開いている画面のボタンなら閉じ、別の画面のボタンならそちらに切り替える
        //--------------------------------------------------------------
        MenuKind toggled = MenuKind::None;
        registry.View<MenuButtonComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, MenuButtonComponent& button, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                const bool keyPressed = input && button.key != Tsukino::Input::KeyCode::None && input->IsKeyPressed(button.key);
                const bool pressed    = pointer.clicked || keyPressed;
                if(pressed && !confirming && (button.canOpen || state.IsOpen(button.menu)))
                    toggled = button.menu;
            });
        if(toggled != MenuKind::None)
            state.open = state.IsOpen(toggled) ? MenuKind::None : toggled;

        // Esc: 何かの画面が開いていれば閉じ、何も開いていなければオプションを開く
        if(toggled == MenuKind::None && !confirming && input && input->IsKeyPressed(Tsukino::Input::KeyCode::Escape))
            state.open = (state.open != MenuKind::None) ? MenuKind::None : MenuKind::Options;

        // ボタンの文字（開いている画面のボタンは「閉じる」）
        registry.View<MenuButtonComponent>().each([&](Tsukino::ECS::Entity, MenuButtonComponent& button) {
            if(button.label != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label))
                registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label).text = state.IsOpen(button.menu) ? button.openText : button.closedText;
        });

        //--------------------------------------------------------------
        // 行のスクロールは、開いている画面のものだけ受け付ける。開いたときは一番上から見せる
        //--------------------------------------------------------------
        const bool justOpened = state.open != m_lastOpen;
        m_lastOpen            = state.open;
        registry.View<MenuPageComponent, Tsukino::BuiltIn::ECS::ScrollViewComponent>().each(
            [&](Tsukino::ECS::Entity, MenuPageComponent& page, Tsukino::BuiltIn::ECS::ScrollViewComponent& scroll) {
                const bool open = state.IsOpen(page.menu);
                if(open && justOpened)
                    scroll.ScrollToTop();
                scroll.enabled = open && !confirming;    // 確認中は止める（確認を閉じたら同じ位置から）
            });

        // 開いた画面の板をポップさせる（ほかの部品は板の動きを写すので一緒に動く）
        if(justOpened && state.open != MenuKind::None) {
            registry.View<MenuPageComponent>().each([&](Tsukino::ECS::Entity entity, MenuPageComponent& page) {
                if(page.motionRoot && state.IsOpen(page.menu))
                    PlayPanelOpen(registry, entity);
            });
        }

        //--------------------------------------------------------------
        // 画面の要素の表示・非表示
        //--------------------------------------------------------------
        registry.View<MenuPageComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, MenuPageComponent& page, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                const bool visible = state.IsOpen(page.menu);

                // スプライト: 閉じている間はスケール 0（描画も当たり判定もされない）
                if(registry.HasComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                    transform.scale = visible ? page.openScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                    transform.dirty = true;
                }

                // 画面に出す 3D（FruitIcon の置き台）: 閉じている間は隠す（モデルはスケールでは隠れないため）
                if(registry.HasComponent<FruitIconComponent>(entity))
                    SetFruitIconVisible(registry, entity, visible);

                // 文字: 閉じている間は空文字（描画されない）。決まった文字はここで書く
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity)) {
                    if(!visible)
                        font->text.clear();
                    else if(!page.text.empty())
                        font->text = page.text;
                }
            });
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   CoinLauncherSystem.cpp
//! @brief  プレイヤーの入力でコインを投入するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/CoinLauncherSystem.hpp>

#include <FruitMagic/ECS/Component/CoinLauncherComponent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Window.hpp>

#include <algorithm>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 投入位置の更新とコインの投入を行います。
    //----------------------------------------------------------------------------
    void CoinLauncherSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!ctx || !ctx->inputSystem || !registry.HasContext<GameState>() || !registry.HasContext<PrizeFactory>())
            return;

        Tsukino::Input::InputSystem& input   = *ctx->inputSystem;
        GameState&                   state   = registry.GetContext<GameState>();
        PrizeFactory&                factory = registry.GetContext<PrizeFactory>();

        //--------------------------------------------------------------
        // マウスの位置（ウィンドウ内のX座標）。動いたときだけ追従させ、
        // キー操作中にマウスが止まっていれば上書きしない
        //--------------------------------------------------------------
        int mouseX = 0;
        int mouseY = 0;
        input.GetMousePosition(&mouseX, &mouseY);
        const bool mouseMoved = (m_lastMouseX >= 0 && mouseX != m_lastMouseX);
        m_lastMouseX          = mouseX;

        const int windowWidth = (ctx->window != nullptr) ? ctx->window->GetWidth() : 0;

        //--------------------------------------------------------------
        // 投入は「押した瞬間」で判定（押しっぱなしで連射しない）。
        // 魔法ボタンなどクリックできる UI の上でのクリックは、その UI の操作なので投入しない
        //--------------------------------------------------------------
        bool pointerOnUi = false;
        registry.View<Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) { pointerOnUi = pointerOnUi || pointer.hovered; });

        const bool launchPressed = input.IsKeyPressed(Tsukino::Input::KeyCode::Space) ||
                                   (input.IsKeyPressed(Tsukino::Input::KeyCode::LButton) && !pointerOnUi);

        std::vector<hlslpp::float3> launchPositions;    // このフレームに投入するコインの位置

        auto view = registry.View<CoinLauncherComponent, Tsukino::BuiltIn::ECS::TransformComponent>();
        view.each([&](Tsukino::ECS::Entity, CoinLauncherComponent& launcher, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
            //--------------------------------------------------------------
            // 投入位置の移動。キーもマウスも画面上の左右に合わせる（画面の右はワールドの kScreenRightX 向き）
            //--------------------------------------------------------------
            if(input.IsKeyDown(Tsukino::Input::KeyCode::Left))
                launcher.laneX -= Layout::kScreenRightX * launcher.laneSpeed * deltaTime;
            if(input.IsKeyDown(Tsukino::Input::KeyCode::Right))
                launcher.laneX += Layout::kScreenRightX * launcher.laneSpeed * deltaTime;

            if(mouseMoved && windowWidth > 0) {
                // 画面の左端〜右端を、投入できる範囲の画面上の左端〜右端に対応させる
                const float t  = std::clamp(static_cast<float>(mouseX) / static_cast<float>(windowWidth), 0.0f, 1.0f);
                launcher.laneX = (t * 2.0f - 1.0f) * Layout::kScreenRightX * Layout::kLaunchLaneHalfWidth;
            }

            launcher.laneX = std::clamp(launcher.laneX, -Layout::kLaunchLaneHalfWidth, Layout::kLaunchLaneHalfWidth);

            // 目印を投入位置へ動かす
            transform.position = hlslpp::float3(launcher.laneX, Layout::kLaunchMarkerY, Layout::kLaunchZ);
            transform.dirty    = true;

            //--------------------------------------------------------------
            // コインの投入
            //--------------------------------------------------------------
            launcher.cooldown = std::max(0.0f, launcher.cooldown - deltaTime);
            if(!launchPressed || launcher.cooldown > 0.0f || state.coins <= 0)
                return;

            state.coins -= 1;
            launcher.cooldown = launcher.interval;
            launchPositions.push_back(hlslpp::float3(launcher.laneX, Layout::kLaunchY, Layout::kLaunchZ));
        });

        //--------------------------------------------------------------
        // 生成は View の反復の後で行う
        // （反復中に TransformComponent を持つエンティティを増やすと反復が壊れるため）
        //--------------------------------------------------------------
        for(const hlslpp::float3& position : launchPositions) {
            factory.CreateCoin(registry, position);
        }
        if(registry.HasContext<PlayStats>())
            registry.GetContext<PlayStats>().coinsLaunched += static_cast<int>(launchPositions.size());
    }
}    // namespace FruitMagic::ECS

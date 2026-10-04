//----------------------------------------------------------------------------
//! @file   DebugTreeLevelSystem.cpp
//! @brief  （開発用）果樹の段階を切り替えるシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/DebugTreeLevelSystem.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! F2 が押されたら果樹の段階を1つ上げます（最大の次は 0 に戻ります）。
    //----------------------------------------------------------------------------
    void DebugTreeLevelSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!ctx || !ctx->inputSystem || !registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>())
            return;
        if(!ctx->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::F2))
            return;

        // 最大の段階は、定義されている果物の解放段階の最大値
        int maxLevel = 0;
        for(const FruitDef& def : registry.GetContext<FruitCatalog>().Fruits()) {
            maxLevel = std::max(maxLevel, def.unlockTreeLevel);
        }

        GameState& state = registry.GetContext<GameState>();
        state.treeLevel  = (state.treeLevel >= maxLevel) ? 0 : state.treeLevel + 1;
        Tsukino::Core::Log::Info("DebugTreeLevel: tree level = " + std::to_string(state.treeLevel));
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   DebugResourceSystem.cpp
//! @brief  （開発用）コインと果実を増やすシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/DebugResourceSystem.hpp>

#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Log.hpp>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 1回で増やす量
        constexpr int kAmount     = 100;
        constexpr int kSeedAmount = 5;    // 星のたね（おひっこしをしなくても星のたねの強化を試せるように）
    }    // namespace

    //----------------------------------------------------------------------------
    //! F2 が押されたらコイン・FP・星のたねを増やし、マナを満タンにします。
    //----------------------------------------------------------------------------
    void DebugResourceSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!ctx || !ctx->inputSystem || !registry.HasContext<GameState>())
            return;
        if(!ctx->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::F2))
            return;

        GameState& state = registry.GetContext<GameState>();
        state.coins += kAmount;
        state.fruitPoints += kAmount;
        state.starSeeds += kSeedAmount;
        state.mana = state.maxMana;    // 魔法をすぐ試せるように
        Tsukino::Core::Log::Info("DebugResource: coins = " + std::to_string(state.coins) + ", fruit = " + std::to_string(state.fruitPoints) +
                                 ", seeds = " + std::to_string(state.starSeeds));
    }
}    // namespace FruitMagic::ECS

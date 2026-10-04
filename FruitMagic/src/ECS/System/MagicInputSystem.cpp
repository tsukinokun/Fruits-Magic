//----------------------------------------------------------------------------
//! @file   MagicInputSystem.cpp
//! @brief  魔法を撃つ入力のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/MagicInputSystem.hpp>

#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    MagicInputSystem::MagicInputSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
    }

    //----------------------------------------------------------------------------
    //! 入力を読み、撃てる魔法なら撃ちます。
    //----------------------------------------------------------------------------
    void MagicInputSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<MagicCatalog>() || !registry.HasContext<MagicState>())
            return;

        //--------------------------------------------------------------
        // 押された枠を調べる（数字キーが優先。同じフレームに複数押されたら若い番号）
        //--------------------------------------------------------------
        int slot = 0;

        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(ctx && ctx->inputSystem) {
            for(int s = 1; s <= kMagicSlotCount && slot == 0; ++s) {
                const auto key = static_cast<Tsukino::Input::KeyCode>(static_cast<int>(Tsukino::Input::KeyCode::D0) + s);
                if(ctx->inputSystem->IsKeyPressed(key))
                    slot = s;
            }
        }

        if(slot == 0) {
            registry.View<MagicButtonComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
                [&](Tsukino::ECS::Entity, MagicButtonComponent& button, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                    if(pointer.clicked && slot == 0)
                        slot = button.slot;
                });
        }
        if(slot == 0)
            return;

        //--------------------------------------------------------------
        // 撃てるか（割り当て・解放・効果中でない・マナ）を確かめてから、マナを引いて発動
        //--------------------------------------------------------------
        const MagicCatalog& catalog    = registry.GetContext<MagicCatalog>();
        const int           magicIndex = catalog.FindBySlot(slot);
        if(magicIndex < 0)
            return;

        const MagicDef& def   = catalog.Magics()[magicIndex];
        GameState&      state = registry.GetContext<GameState>();
        if(!def.unlocked || registry.GetContext<MagicState>().activeMagic >= 0 || state.mana < def.cost)
            return;

        state.mana -= def.cost;
        m_eventBus.Publish(MagicCastEvent{magicIndex});
    }
}    // namespace FruitMagic::ECS

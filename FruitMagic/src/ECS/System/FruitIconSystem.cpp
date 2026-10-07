//----------------------------------------------------------------------------
//! @file   FruitIconSystem.cpp
//! @brief  画面に出す果物（FruitIconComponent）を回すシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/FruitIconSystem.hpp>

#include <FruitMagic/ECS/Component/FruitIconComponent.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 置き台を回します。
    //----------------------------------------------------------------------------
    void FruitIconSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        registry.View<FruitIconComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity, FruitIconComponent& icon, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(icon.spinSpeed <= 0.0f || !icon.visible || icon.visual == entt::null)
                    return;
                icon.spin          = std::fmod(icon.spin + icon.spinSpeed * deltaTime, 360.0f);
                transform.rotation = PrizeFactory::EulerDegrees(hlslpp::float3(icon.tilt, icon.spin, 0.0f));
                transform.dirty    = true;
            });
    }
}    // namespace FruitMagic::ECS

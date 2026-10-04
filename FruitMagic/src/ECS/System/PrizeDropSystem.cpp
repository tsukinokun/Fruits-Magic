//----------------------------------------------------------------------------
//! @file   PrizeDropSystem.cpp
//! @brief  景品が台から落ちたことを判定するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/PrizeDropSystem.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <cmath>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    PrizeDropSystem::PrizeDropSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
    }

    //----------------------------------------------------------------------------
    //! 落ちた景品を判定します。
    //----------------------------------------------------------------------------
    void PrizeDropSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        //--------------------------------------------------------------
        // 判定ラインより下にある景品を集める
        // （View の反復中にハンドラ側でエンティティを触られないよう、発行は反復の後で行う）
        //--------------------------------------------------------------
        std::vector<PrizeDroppedEvent> dropped;

        auto view = registry.View<PrizeComponent, Tsukino::BuiltIn::ECS::TransformComponent>();
        view.each([&](Tsukino::ECS::Entity entity, PrizeComponent& prize, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
            if(float(transform.position.y) >= Layout::kDropJudgeY)
                return;

            // 落ちた時点の左右位置で、正面の払い出し口か左右の溝かを決める
            const bool isPayout = std::abs(float(transform.position.x)) <= Layout::kPayoutHalfWidth;

            PrizeDroppedEvent e;
            e.prize = entity;
            e.kind  = prize.kind;
            e.value = prize.value;
            e.fruitIndex = prize.fruitIndex;
            e.variantIndex = prize.variantIndex;
            e.valueMultiplier = prize.valueMultiplier;
            e.zone  = isPayout ? DropZone::Payout : DropZone::Gutter;
            e.x     = transform.position.x;
            dropped.push_back(e);
        });

        //--------------------------------------------------------------
        // 発行して破棄予約（実際の破棄は全システム更新後の FlushDestroyQueue）
        //--------------------------------------------------------------
        for(const PrizeDroppedEvent& e : dropped) {
            PrizeFactory::DestroyPrize(registry, e.prize);
            m_eventBus.Publish(e);
        }
    }
}    // namespace FruitMagic::ECS

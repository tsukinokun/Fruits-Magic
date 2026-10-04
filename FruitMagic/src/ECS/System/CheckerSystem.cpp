//----------------------------------------------------------------------------
//! @file   CheckerSystem.cpp
//! @brief  チェッカー（払い出し口で左右に動く穴）のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/CheckerSystem.hpp>

#include <FruitMagic/ECS/Component/CheckerComponent.hpp>
#include <FruitMagic/ECS/Event/CheckerEnteredEvent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    CheckerSystem::CheckerSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
        // 払い出し口に落ちたコインだけを積む（溝に落ちた物と果物は対象外）
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Coin && e.zone == DropZone::Payout)
                m_pendingCoinXs.push_back(e.x);
        });
    }

    //----------------------------------------------------------------------------
    //! 穴を動かし、払い出し口に落ちたコインが穴に入ったかを判定します。
    //----------------------------------------------------------------------------
    void CheckerSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        const RouletteConfig config = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};

        // 穴が払い出し口からはみ出さない範囲で往復させる
        const float range = std::max(0.0f, std::min(config.checkerRange, Layout::kPayoutHalfWidth - config.checkerHalfWidth));

        //--------------------------------------------------------------
        // 穴を左右に往復させる（目印の Transform も一緒に動かす）
        //--------------------------------------------------------------
        std::vector<float> holes;
        auto checkerView = registry.View<CheckerComponent, Tsukino::BuiltIn::ECS::TransformComponent>();
        checkerView.each([&](Tsukino::ECS::Entity, CheckerComponent& checker, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
            checker.time = std::fmod(checker.time + deltaTime, config.checkerPeriod);
            checker.x    = std::sin(2.0f * kPi * checker.time / config.checkerPeriod) * range;

            transform.position = hlslpp::float3(checker.x, float(transform.position.y), float(transform.position.z));
            transform.dirty    = true;
            holes.push_back(checker.x);
        });

        //--------------------------------------------------------------
        // 払い出し口に落ちたコインのうち、落ちた位置が穴の範囲だったものでルーレットを回す。
        // コインの破棄と手持ちへの加算は PrizeDropSystem / WalletSystem が済ませている
        //--------------------------------------------------------------
        for(float coinX : m_pendingCoinXs) {
            for(float holeX : holes) {
                if(std::abs(coinX - holeX) <= config.checkerHalfWidth) {
                    m_eventBus.Publish(CheckerEnteredEvent{coinX});
                    break;
                }
            }
        }
        m_pendingCoinXs.clear();
    }
}    // namespace FruitMagic::ECS

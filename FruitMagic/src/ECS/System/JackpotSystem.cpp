//----------------------------------------------------------------------------
//! @file   JackpotSystem.cpp
//! @brief  ジャックポット穴のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/JackpotSystem.hpp>

#include <FruitMagic/ECS/Component/JackpotHoleComponent.hpp>
#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/JackpotConfig.hpp>
#include <FruitMagic/Game/MagicEffects.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/RouletteState.hpp>

#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cmath>
#include <string>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //! @brief 上面に乗っているとみなすコインの中心の高さの上限（上面からの距離、cm）。積み重なった上のコインは落ちない
        constexpr float kOnTopMaxHeight = 1.5f;

        //! @brief 1回の当たりで出す「ジャックポット！」の表示時間（秒）
        constexpr float kNoticeSeconds = 3.0f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    JackpotSystem::JackpotSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus) {
    }

    //----------------------------------------------------------------------------
    //! 穴を動かし、コインを吸い込みます。
    //----------------------------------------------------------------------------
    void JackpotSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<PusherRef>() || !registry.HasContext<JackpotConfig>())
            return;

        const Tsukino::ECS::Entity pusher = registry.GetContext<PusherRef>().entity;
        auto*                      pusherTransform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(pusher);
        if(!pusherTransform)
            return;

        const JackpotConfig& config     = registry.GetContext<JackpotConfig>();
        const float          pusherFront = float(pusherTransform->position.z) + Layout::kPusherHalfDepth;

        //--------------------------------------------------------------
        // 穴を動かし、開閉を進める（閉じている時間 → 開いている時間 の繰り返し）
        //--------------------------------------------------------------
        struct Hole {
            float x;
            float z;
        };
        std::vector<Hole> openHoles;
        registry.View<JackpotHoleComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, JackpotHoleComponent& hole, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                hole.moveTime = std::fmod(hole.moveTime + deltaTime, config.period);
                const float cycle = config.openSeconds + config.closedSeconds;
                hole.openTime     = (cycle > 0.0f) ? std::fmod(hole.openTime + deltaTime, cycle) : 0.0f;
                hole.open         = hole.openTime >= config.closedSeconds && config.openSeconds > 0.0f;

                const float x      = std::sin(2.0f * kPi * hole.moveTime / config.period) * config.range;
                const float z      = pusherFront - config.frontOffset;
                transform.position = hlslpp::float3(x, Layout::kPusherTopY + 0.05f, z);
                transform.dirty    = true;

                // 開いている間だけ金色に光らせる
                if(auto* glow = registry.try_get<Tsukino::BuiltIn::ECS::RimGlowComponent>(entity))
                    glow->active = hole.open;

                if(hole.open)
                    openHoles.push_back({x, z});
            });
        if(openHoles.empty())
            return;

        //--------------------------------------------------------------
        // 開いた穴の上に来たコインを吸い込む（破棄は反復の後）
        //--------------------------------------------------------------
        std::vector<Tsukino::ECS::Entity> swallowed;
        registry.View<PrizeComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, PrizeComponent& prize, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(prize.kind != PrizeKind::Coin)
                    return;
                const float y = transform.position.y;
                if(y < Layout::kPusherTopY || y > Layout::kPusherTopY + kOnTopMaxHeight)
                    return;
                for(const Hole& hole : openHoles) {
                    if(std::abs(float(transform.position.x) - hole.x) <= config.holeHalfWidth &&
                       std::abs(float(transform.position.z) - hole.z) <= config.holeHalfDepth) {
                        swallowed.push_back(entity);
                        return;
                    }
                }
            });

        for(Tsukino::ECS::Entity coin : swallowed) {
            registry.QueueDestroy(coin);

            //--------------------------------------------------------------
            // 当たり: ルーレットの回数を増やし、コインを降らせる
            //--------------------------------------------------------------
            int hits = 0;
            if(registry.HasContext<PlayStats>())
                hits = ++registry.GetContext<PlayStats>().jackpots;
            if(registry.HasContext<RouletteState>())
                registry.GetContext<RouletteState>().stock += config.spins;
            if(registry.HasContext<CoinShowerState>())
                registry.GetContext<CoinShowerState>().Add(config.bonusCoins, 1.5f);

            m_eventBus.Publish(NoticeEvent{L"ジャックポット！  ルーレット +" + std::to_wstring(config.spins) + L"  コイン +" + std::to_wstring(config.bonusCoins),
                                           kNoticeSeconds});
            Tsukino::Core::Log::Info("Jackpot: hit #" + std::to_string(hits));
        }
    }
}    // namespace FruitMagic::ECS

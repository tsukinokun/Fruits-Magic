//----------------------------------------------------------------------------
//! @file   CoinShowerSystem.cpp
//! @brief  コインのシャワーのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/CoinShowerSystem.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/EffectEvent.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/EconomyConfig.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    CoinShowerSystem::CoinShowerSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus)
        , m_rng(std::random_device{}()) {
    }

    //----------------------------------------------------------------------------
    //! 依頼を進めます。
    //----------------------------------------------------------------------------
    void CoinShowerSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<CoinShowerState>() || !registry.HasContext<PrizeFactory>())
            return;

        CoinShowerState& shower = registry.GetContext<CoinShowerState>();
        if(shower.requests.empty())
            return;

        //--------------------------------------------------------------
        // 台の上のコインが多すぎる間は降らせるのを待つ（物理が重くなりすぎてゲームが進まなくなるため）
        //--------------------------------------------------------------
        if(registry.HasContext<EconomyConfig>()) {
            int coins = 0;
            registry.View<PrizeComponent>().each([&](Tsukino::ECS::Entity, PrizeComponent& prize) {
                if(prize.kind == PrizeKind::Coin)
                    ++coins;
            });
            if(coins >= registry.GetContext<EconomyConfig>().maxCoinsOnTable)
                return;
        }

        //--------------------------------------------------------------
        // 今フレームに落とす枚数を数える（生成は後でまとめて）
        //--------------------------------------------------------------
        const TableLayout& layout = GetTableLayout(registry);
        int                count  = 0;
        for(CoinShowerState::Request& r : shower.requests) {
            r.timer -= deltaTime;
            while(r.remaining > 0 && r.timer <= 0.0f && count < layout.showerMaxPerFrame) {
                r.remaining -= 1;
                r.timer += r.interval;
                ++count;
            }
        }
        shower.requests.erase(std::remove_if(shower.requests.begin(), shower.requests.end(),
                                             [](const CoinShowerState::Request& r) { return r.remaining <= 0; }),
                              shower.requests.end());

        //--------------------------------------------------------------
        // 落とす範囲。プッシャーが最も前に出ても届かない手前側（届く所に落とすとプッシャーの上や下に入り込む）
        //--------------------------------------------------------------
        PrizeFactory&                         factory = registry.GetContext<PrizeFactory>();
        const float                           halfX   = layout.fieldHalfWidth - layout.coinHalfExtent.x - layout.showerSideMargin;
        const float                           minZ    = layout.pusherMinFrontZ + layout.pusherMaxAmplitude * 2.0f + layout.showerBackMargin;    // 奥の端
        const float                           maxZ    = std::max(minZ, layout.fieldFrontZ - layout.showerFrontMargin);    // 手前の端
        std::uniform_real_distribution<float> randomX(-halfX, halfX);
        std::uniform_real_distribution<float> randomZ(minZ, maxZ);
        for(int i = 0; i < count; ++i) {
            const hlslpp::float3 position(randomX(m_rng), layout.showerDropY, randomZ(m_rng));
            factory.CreateCoin(registry, position, factory.RandomCoinRotation(m_rng));
            m_eventBus.Publish(EffectEvent{"showerCoin", position});
        }
        if(registry.HasContext<PlayStats>())
            registry.GetContext<PlayStats>().showerCoins += count;
    }
}    // namespace FruitMagic::ECS

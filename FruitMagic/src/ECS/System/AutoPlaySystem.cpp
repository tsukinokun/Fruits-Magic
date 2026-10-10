//----------------------------------------------------------------------------
//! @file   AutoPlaySystem.cpp
//! @brief  （バランスの計測用）自動で遊ぶシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/AutoPlaySystem.hpp>

#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 投入の間隔（秒）。台を見ながらクリックし続ける人くらいの速さ
        constexpr float kLaunchInterval = 0.6f;

        //! @brief 魔法・強化を考える間隔（秒）
        constexpr float kDecideInterval = 1.0f;

        //! @brief 強化を買っても残しておくコイン（投入用）
        constexpr int kCoinReserve = 20;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    AutoPlaySystem::AutoPlaySystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus)
        , m_rng(std::random_device{}()) {
    }

    //----------------------------------------------------------------------------
    //! 投入・魔法・強化を行います。
    //----------------------------------------------------------------------------
    void AutoPlaySystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<PrizeFactory>())
            return;

        GameState& state = registry.GetContext<GameState>();
        const TableLayout& layout = GetTableLayout(registry);

        //--------------------------------------------------------------
        // 投入（プレイヤーの投入と同じ位置・同じく手持ちから引く）
        //--------------------------------------------------------------
        m_launchTimer -= deltaTime;
        if(m_launchTimer <= 0.0f && state.coins > 0) {
            m_launchTimer = kLaunchInterval;
            const float x = std::uniform_real_distribution<float>(-layout.launchLaneHalfWidth, layout.launchLaneHalfWidth)(m_rng);
            registry.GetContext<PrizeFactory>().CreateCoin(registry, hlslpp::float3(x, layout.LaunchY(), layout.LaunchZ()));
            state.coins -= 1;
            if(registry.HasContext<PlayStats>())
                registry.GetContext<PlayStats>().coinsLaunched += 1;
        }

        m_decideTimer -= deltaTime;
        if(m_decideTimer > 0.0f)
            return;
        m_decideTimer = kDecideInterval;

        //--------------------------------------------------------------
        // 魔法: 撃てるもののうち、いちばんコストの高いものを撃つ
        //--------------------------------------------------------------
        if(registry.HasContext<MagicCatalog>() && registry.HasContext<MagicState>()) {
            const MagicCatalog& catalog    = registry.GetContext<MagicCatalog>();
            const MagicState&   magic      = registry.GetContext<MagicState>();
            const int           registered = state.RegisteredCount();

            int best = -1;
            for(int i = 0; i < static_cast<int>(catalog.Magics().size()); ++i) {
                const MagicDef& def = catalog.Magics()[i];
                if(!catalog.IsUnlocked(i, registered) || magic.IsActive(i) || state.mana < def.cost)
                    continue;
                if(best < 0 || def.cost > catalog.Magics()[best].cost)
                    best = i;
            }
            if(best >= 0) {
                state.mana -= catalog.Magics()[best].cost;
                if(registry.HasContext<PlayStats>())
                    registry.GetContext<PlayStats>().magicsCast += 1;
                m_eventBus.Publish(MagicCastEvent{best});
            }
        }

        //--------------------------------------------------------------
        // 強化: 買えるもののうち、手持ちに対していちばん安いもの（コインの強化は投入用のコインを残す）
        //--------------------------------------------------------------
        if(registry.HasContext<UpgradeCatalog>()) {
            const auto& defs     = registry.GetContext<UpgradeCatalog>().Upgrades();
            int         cheapest = -1;
            double      price    = 0.0;
            for(int i = 0; i < static_cast<int>(defs.size()); ++i) {
                const int level = state.UpgradeLevelOf(defs[i].id);
                if(level >= defs[i].MaxLevel())
                    continue;
                const long long cost   = defs[i].levels[level].cost;
                const bool      coins  = defs[i].currency == UpgradeCurrency::Coins;
                const long long held   = coins ? state.coins - kCoinReserve : state.fruitPoints;
                if(held < cost)
                    continue;
                const double relative = static_cast<double>(cost) / std::max(1.0, static_cast<double>(held));
                if(cheapest < 0 || relative < price) {
                    cheapest = i;
                    price    = relative;
                }
            }
            if(cheapest >= 0)
                UpgradeSystem::Purchase(registry, cheapest);
        }
    }
}    // namespace FruitMagic::ECS

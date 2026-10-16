//----------------------------------------------------------------------------
//! @file   MoveRules.cpp
//! @brief  屋台のおひっこしの決まりの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/MoveRules.hpp>

#include <FruitMagic/ECS/System/UpgradeSystem.hpp>
#include <FruitMagic/Game/EconomyConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MoveConfig.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <cmath>
#include <string>

// 名前空間 : FruitMagic::MoveRules
namespace FruitMagic::MoveRules {

    namespace {
        // 設定（無ければ既定値）
        MoveConfig Config(Tsukino::ECS::Registry& registry) {
            return registry.HasContext<MoveConfig>() ? registry.GetContext<MoveConfig>() : MoveConfig{};
        }

        // 星のたねの強化の効果の値（強化していなければ定義の base）
        float SeedUpgradeValue(Tsukino::ECS::Registry& registry, const char* id) {
            if(!registry.HasContext<UpgradeCatalog>())
                return 0.0f;
            const UpgradeDef* def = registry.GetContext<UpgradeCatalog>().Find(id);
            return def ? def->ValueAt(registry.GetContext<GameState>().UpgradeLevelOf(id)) : 0.0f;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! おひっこしが解放されているかを返します。
    //----------------------------------------------------------------------------
    bool IsUnlocked(Tsukino::ECS::Registry& registry) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>())
            return false;

        const GameState& state  = registry.GetContext<GameState>();
        const auto&      fruits = registry.GetContext<FruitCatalog>().Fruits();
        const int        stage  = Config(registry).unlockTreeStage;
        for(size_t f = 0; f < fruits.size() && f < state.harvestCounts.size(); ++f) {
            if(fruits[f].unlockTreeLevel < stage)
                continue;
            const auto& counts = state.harvestCounts[f];
            if(std::any_of(counts.begin(), counts.end(), [](int n) { return n > 0; }))
                return true;
        }
        return false;
    }

    //----------------------------------------------------------------------------
    //! 今おひっこししたらもらえる星のたねの数を返します。
    //----------------------------------------------------------------------------
    long long SeedsIfMovedNow(Tsukino::ECS::Registry& registry) {
        if(!IsUnlocked(registry))
            return 0;

        // 収穫の価値の平方根（たくさん稼ぐほど伸びがゆるやか）に、星のたねの強化の倍率を掛ける
        const double value      = static_cast<double>(std::max(0LL, registry.GetContext<GameState>().harvestValue));
        const double multiplier = registry.HasContext<TableStats>() ? registry.GetContext<TableStats>().seedGainMultiplier : 1.0;
        return static_cast<long long>(std::floor(std::sqrt(value / Config(registry).seedDivisor) * multiplier));
    }

    //----------------------------------------------------------------------------
    //! 今おひっこしできるかを返します。
    //----------------------------------------------------------------------------
    bool CanMove(Tsukino::ECS::Registry& registry) {
        return SeedsIfMovedNow(registry) >= Config(registry).minSeeds;
    }

    //----------------------------------------------------------------------------
    //! おひっこしします。
    //----------------------------------------------------------------------------
    long long Move(Tsukino::ECS::Registry& registry) {
        if(!CanMove(registry))
            return 0;

        GameState&      state = registry.GetContext<GameState>();
        const long long seeds = SeedsIfMovedNow(registry);

        //--------------------------------------------------------------
        // 受け取るもの
        //--------------------------------------------------------------
        state.starSeeds += seeds;
        ++state.moveCount;
        ++state.stats.moves;
        state.stats.seedsEarned += seeds;

        //--------------------------------------------------------------
        // 戻すもの: 強化はコインと FP のものを消し、星のたねのものだけ残す（定義から消えた強化も消す）
        //--------------------------------------------------------------
        if(registry.HasContext<UpgradeCatalog>()) {
            const UpgradeCatalog& catalog = registry.GetContext<UpgradeCatalog>();
            for(auto it = state.upgradeLevels.begin(); it != state.upgradeLevels.end();) {
                const UpgradeDef* def = catalog.Find(it->first);
                it                    = (def && def->currency == UpgradeCurrency::Seeds) ? std::next(it) : state.upgradeLevels.erase(it);
            }
        } else {
            state.upgradeLevels.clear();
        }

        // 最初の妖精（星のたねの強化。値は妖精の自動投入のレベル）
        const int fairyLevel = static_cast<int>(SeedUpgradeValue(registry, "startFairy"));
        if(fairyLevel > 0)
            state.upgradeLevels["fairy"] = fairyLevel;

        const int startCoins = registry.HasContext<EconomyConfig>() ? registry.GetContext<EconomyConfig>().startCoins : EconomyConfig{}.startCoins;
        state.coins          = startCoins + std::max(0, static_cast<int>(SeedUpgradeValue(registry, "startCoins")));
        state.fruitPoints    = 0;
        state.mana           = 0;
        state.harvestValue   = 0;
        ECS::UpgradeSystem::ApplyUpgrades(registry);

        Tsukino::Core::Log::Info("MoveRules: moved (#" + std::to_string(state.moveCount) + ", +" + std::to_string(seeds) + " seeds, total " + std::to_string(state.starSeeds) + ").");
        return seeds;
    }
}    // namespace FruitMagic::MoveRules

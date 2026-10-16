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
#include <FruitMagic/Game/SouvenirCatalog.hpp>
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
    //! 今おひっこししたらもらえるおみやげを引き、レベルを上げます。
    //----------------------------------------------------------------------------
    std::vector<SouvenirGift> DrawSouvenirs(Tsukino::ECS::Registry& registry, std::mt19937& rng) {
        std::vector<SouvenirGift> gifts;
        if(!registry.HasContext<SouvenirCatalog>() || !registry.HasContext<GameState>())
            return gifts;

        const SouvenirCatalog& catalog = registry.GetContext<SouvenirCatalog>();
        GameState&             state   = registry.GetContext<GameState>();
        const auto&            defs    = catalog.Souvenirs();

        // もらう数（収穫の価値が valuePerExtra 増えるごとに1つ多く。上限あり）
        const int count = std::min(catalog.maxPerMove, 1 + static_cast<int>(static_cast<float>(std::max(0LL, state.harvestValue)) / catalog.valuePerExtra));

        // 手持ちのコインが多いほど、レアなおみやげの重みを上げる（0〜1 の割合）
        const float boost = std::clamp(static_cast<float>(state.coins) / catalog.coinsForFullBoost, 0.0f, 1.0f);
        auto weightOf     = [&](int rarity) {
            const float base  = rarity < static_cast<int>(catalog.rarityWeights.size()) ? catalog.rarityWeights[rarity] : 0.0f;
            const float extra = rarity < static_cast<int>(catalog.rarityBoost.size()) ? catalog.rarityBoost[rarity] : 0.0f;
            return base * (1.0f + extra * boost);
        };

        for(int n = 0; n < count; ++n) {
            //--------------------------------------------------------------
            // まだ上限でないおみやげの中から、レア度の重みで引く（同じレア度の中では同じ確率。
            // 1つあたりの重みは「レア度の重み ÷ そのレア度のおみやげの数」なので、数が増えてもレア度の出やすさは変わらない）
            //--------------------------------------------------------------
            int perRarity[4] = {0, 0, 0, 0};
            for(const SouvenirDef& def : defs) {
                if(state.SouvenirLevelOf(def.id) < def.maxLevel)
                    ++perRarity[def.rarity];
            }
            std::vector<double> weights(defs.size(), 0.0);
            for(size_t i = 0; i < defs.size(); ++i) {
                if(state.SouvenirLevelOf(defs[i].id) < defs[i].maxLevel)
                    weights[i] = weightOf(defs[i].rarity) / static_cast<double>(perRarity[defs[i].rarity]);
            }
            if(std::all_of(weights.begin(), weights.end(), [](double w) { return w <= 0.0; }))
                break;    // すべて上限

            std::discrete_distribution<int> pick(weights.begin(), weights.end());
            const int                       index = pick(rng);
            int&                            level = state.souvenirLevels[defs[index].id];
            ++level;
            gifts.push_back({index, level, level == 1});
            Tsukino::Core::Log::Info("MoveRules: souvenir " + defs[index].id + " -> Lv " + std::to_string(level) + " (coin boost " + std::to_string(boost) + ").");
        }
        return gifts;
    }

    //----------------------------------------------------------------------------
    //! おひっこしします。
    //----------------------------------------------------------------------------
    MoveResult Move(Tsukino::ECS::Registry& registry) {
        MoveResult result;
        if(!CanMove(registry))
            return result;

        GameState& state = registry.GetContext<GameState>();
        result.seeds     = SeedsIfMovedNow(registry);

        //--------------------------------------------------------------
        // 受け取るもの（おみやげは、手持ちのコインと収穫の価値を戻す前に引く）
        //--------------------------------------------------------------
        state.starSeeds += result.seeds;
        ++state.moveCount;
        ++state.stats.moves;
        state.stats.seedsEarned += result.seeds;
        std::mt19937 rng(std::random_device{}());
        result.gifts = DrawSouvenirs(registry, rng);

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

        // 最初のコイン（Economy.json の値 + 星のたねの強化 + おみやげ）
        int startCoins = registry.HasContext<EconomyConfig>() ? registry.GetContext<EconomyConfig>().startCoins : EconomyConfig{}.startCoins;
        startCoins += std::max(0, static_cast<int>(SeedUpgradeValue(registry, "startCoins")));
        if(registry.HasContext<SouvenirCatalog>()) {
            for(const SouvenirDef& def : registry.GetContext<SouvenirCatalog>().Souvenirs()) {
                if(def.effect == SouvenirEffect::StartCoins)
                    startCoins += static_cast<int>(def.perLevel * static_cast<float>(state.SouvenirLevelOf(def.id)));
            }
        }
        state.coins = startCoins;
        state.fruitPoints    = 0;
        state.mana           = 0;
        state.harvestValue   = 0;
        ECS::UpgradeSystem::ApplyUpgrades(registry);

        Tsukino::Core::Log::Info("MoveRules: moved (#" + std::to_string(state.moveCount) + ", +" + std::to_string(result.seeds) + " seeds, total " +
                                 std::to_string(state.starSeeds) + ", " + std::to_string(result.gifts.size()) + " souvenirs).");
        return result;
    }
}    // namespace FruitMagic::MoveRules

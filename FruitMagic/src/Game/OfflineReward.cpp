//----------------------------------------------------------------------------
//! @file   OfflineReward.cpp
//! @brief  閉じている間の報酬の設定と計算の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/OfflineReward.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/JsonReader.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool OfflineConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "OfflineConfig"))
            return false;

        Json::Read(doc, "payoutRatio", payoutRatio);
        Json::Read(doc, "fruitPerCoin", fruitPerCoin);
        Json::Read(doc, "minSeconds", minSeconds);
        Json::Read(doc, "autosaveSeconds", autosaveSeconds);

        payoutRatio     = std::max(0.0f, payoutRatio);
        fruitPerCoin    = std::max(0.0f, fruitPerCoin);
        minSeconds      = std::max(0, minSeconds);
        autosaveSeconds = std::max(5.0f, autosaveSeconds);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 閉じていた時間から報酬を計算し、GameState に足します。
    //----------------------------------------------------------------------------
    OfflineReport GrantOfflineReward(Tsukino::ECS::Registry& registry, long long awaySeconds, std::mt19937& rng) {
        OfflineReport report;
        report.awaySeconds = std::max(0LL, awaySeconds);    // 時計が戻っていたら 0 とみなす
        if(!registry.HasContext<GameState>() || !registry.HasContext<TableStats>() || !registry.HasContext<FruitCatalog>())
            return report;

        GameState&          state   = registry.GetContext<GameState>();
        const TableStats&   stats   = registry.GetContext<TableStats>();
        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        const OfflineConfig config  = registry.HasContext<OfflineConfig>() ? registry.GetContext<OfflineConfig>() : OfflineConfig{};

        //--------------------------------------------------------------
        // 上限時間で切る
        //--------------------------------------------------------------
        const long long maxSeconds = static_cast<long long>(std::max(0.0f, stats.offlineMaxHours) * 3600.0f);
        report.countedSeconds      = std::min(report.awaySeconds, maxSeconds);
        report.capped              = report.awaySeconds > maxSeconds;
        report.hasFairy            = stats.autoLaunchInterval > 0.0f;
        // 短い留守（起動し直しただけ等）では報酬を出さない。「おかえり」も出ないので、黙って増えることがないように
        if(!report.hasFairy || report.awaySeconds < config.minSeconds)
            return report;

        //--------------------------------------------------------------
        // 妖精が入れたコインの枚数から、戻ってきたコインと収穫できた果物を期待値で求める
        //--------------------------------------------------------------
        const double fairyCoins = static_cast<double>(report.countedSeconds) / stats.autoLaunchInterval;
        report.coins            = static_cast<int>(std::floor(fairyCoins * config.payoutRatio));

        // 果物の数は端数を確率で切り上げる（短い留守でも時々1個もらえるように）
        const double fruits = fairyCoins * config.fruitPerCoin;
        report.fruitCount   = static_cast<int>(std::floor(fruits));
        if(std::uniform_real_distribution<double>(0.0, 1.0)(rng) < fruits - std::floor(fruits))
            report.fruitCount += 1;

        // どの果物が採れたかは今の果樹の段階で抽選し、価値（強化「果物の価値」も掛ける）を FP に足す（バリエーションは通常のみ）
        double fruitPoints = 0.0;
        for(int i = 0; i < report.fruitCount; ++i) {
            const int fruitIndex = catalog.PickSpawnable(state.treeLevel, rng);
            if(fruitIndex >= 0)
                fruitPoints += catalog.Fruits()[fruitIndex].value * stats.fruitValueMultiplier;
        }
        report.fruitPoints = std::llround(fruitPoints);

        state.coins += report.coins;
        state.fruitPoints += report.fruitPoints;
        state.harvestValue += report.fruitPoints;

        Tsukino::Core::Log::Info("OfflineReward: away " + std::to_string(report.awaySeconds) + "s (counted " + std::to_string(report.countedSeconds) +
                                 "s), coins +" + std::to_string(report.coins) + ", fruits " + std::to_string(report.fruitCount) + " (+" +
                                 std::to_string(report.fruitPoints) + ")");
        return report;
    }

    //----------------------------------------------------------------------------
    //! 秒数を「2時間15分」のような表示にします。
    //----------------------------------------------------------------------------
    std::wstring FormatDuration(long long seconds, const Texts& texts) {
        const long long hours   = seconds / 3600;
        const long long minutes = (seconds % 3600) / 60;
        if(hours > 0)
            return texts.Format("duration.hours", {{"n", std::to_wstring(hours)}}) +
                   (minutes > 0 ? texts.Format("duration.minutes", {{"n", std::to_wstring(minutes)}}) : std::wstring());
        if(minutes > 0)
            return texts.Format("duration.minutes", {{"n", std::to_wstring(minutes)}});
        return texts.Format("duration.seconds", {{"n", std::to_wstring(seconds)}});
    }
}    // namespace FruitMagic

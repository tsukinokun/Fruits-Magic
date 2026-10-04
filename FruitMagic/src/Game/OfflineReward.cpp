//----------------------------------------------------------------------------
//! @file   OfflineReward.cpp
//! @brief  閉じている間の報酬の設定と計算の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/OfflineReward.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/TableStats.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

        //--------------------------------------------------------------
        //! 項目があれば数値を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（項目が無ければそのまま）
        //--------------------------------------------------------------
        template <class T>
        void ReadNumber(const rj::Value& obj, const char* key, T& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsNumber())
                out = static_cast<T>(it->value.GetDouble());
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool OfflineConfig::Load(const std::string& path) {
        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn("OfflineConfig: cannot read " + path + ". Using defaults.");
            return false;
        }

        ReadNumber(doc, "payoutRatio", payoutRatio);
        ReadNumber(doc, "fruitPerCoin", fruitPerCoin);
        ReadNumber(doc, "minSeconds", minSeconds);
        ReadNumber(doc, "autosaveSeconds", autosaveSeconds);

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

        // どの果物が採れたかは今の果樹の段階で抽選し、価値を果実に足す（バリエーションは通常のみ）
        for(int i = 0; i < report.fruitCount; ++i) {
            const int fruitIndex = catalog.PickSpawnable(state.treeLevel, rng);
            if(fruitIndex >= 0)
                report.fruitPoints += catalog.Fruits()[fruitIndex].value;
        }

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
    std::wstring FormatDuration(long long seconds) {
        const long long hours   = seconds / 3600;
        const long long minutes = (seconds % 3600) / 60;
        if(hours > 0)
            return std::to_wstring(hours) + L"時間" + (minutes > 0 ? std::to_wstring(minutes) + L"分" : std::wstring());
        if(minutes > 0)
            return std::to_wstring(minutes) + L"分";
        return std::to_wstring(seconds) + L"秒";
    }
}    // namespace FruitMagic

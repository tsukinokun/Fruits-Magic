//----------------------------------------------------------------------------
//! @file   RecordSystem.cpp
//! @brief  記録画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/RecordSystem.hpp>

#include <FruitMagic/ECS/Component/RecordElementComponent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/OfflineReward.hpp>
#include <FruitMagic/Game/Texts.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cmath>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 記録画面が開いていれば、数を書きます。
    //----------------------------------------------------------------------------
    void RecordSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<GameState>())
            return;
        // 閉じている間の文字は MenuSystem が消す
        if(!registry.GetContext<MenuState>().IsOpen(MenuKind::Record))
            return;

        const GameState&     state = registry.GetContext<GameState>();
        const LifetimeStats& stats = state.stats;
        const Texts&         texts = GetTexts(registry);

        // 図鑑の枠の数（果物 × バリエーション）
        int zukanTotal = 0;
        if(registry.HasContext<FruitCatalog>() && registry.HasContext<CollectionConfig>())
            zukanTotal = static_cast<int>(registry.GetContext<FruitCatalog>().Fruits().size() * registry.GetContext<CollectionConfig>().Variants().size());

        auto n     = [](long long value) { return std::to_wstring(value); };
        auto count = [&](const char* key, long long value) { return texts.Format(key, {{"n", n(value)}}); };

        registry.View<RecordElementComponent, Tsukino::BuiltIn::ECS::FontComponent>().each(
            [&](Tsukino::ECS::Entity, RecordElementComponent& element, Tsukino::BuiltIn::ECS::FontComponent& font) {
                switch(element.item) {
                    case RecordItem::HarvestCount: font.text = count("record.fruits", state.HarvestTotal()); break;
                    case RecordItem::HarvestValue: font.text = count("record.count", state.harvestValue); break;
                    case RecordItem::Zukan:
                        font.text = texts.Format("record.ratio", {{"n", n(state.RegisteredCount())}, {"total", n(zukanTotal)}});
                        break;
                    case RecordItem::FruitPoints: font.text = count("record.count", state.fruitPoints); break;
                    case RecordItem::CoinsLaunched: font.text = count("record.coins", stats.coinsLaunched); break;
                    case RecordItem::CoinsPaidOut: font.text = count("record.coins", stats.coinsPaidOut); break;
                    case RecordItem::CoinsToGutter: font.text = count("record.coins", stats.coinsToGutter); break;
                    case RecordItem::FairyCoins: font.text = count("record.coins", stats.fairyCoins); break;
                    case RecordItem::ShowerCoins: font.text = count("record.coins", stats.showerCoins); break;
                    case RecordItem::RouletteSpins: font.text = count("record.times", stats.rouletteSpins); break;
                    case RecordItem::RouletteHits: {
                        const long long rate = (stats.rouletteSpins > 0) ? std::llround(100.0 * static_cast<double>(stats.rouletteHits) / static_cast<double>(stats.rouletteSpins)) : 0;
                        font.text            = texts.Format("record.rate", {{"n", n(stats.rouletteHits)}, {"rate", n(rate)}});
                        break;
                    }
                    case RecordItem::JackpotChances: font.text = count("record.times", stats.jackpotChances); break;
                    case RecordItem::Jackpots: font.text = count("record.times", stats.jackpots); break;
                    case RecordItem::MagicsCast: font.text = count("record.times", stats.magicsCast); break;
                    case RecordItem::PlayTime: font.text = FormatDuration(static_cast<long long>(stats.playSeconds), texts); break;
                }
            });
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   SidePanelSystem.cpp
//! @brief  画面の左右のパネル（台のようす・進み具合とおすすめ）を更新するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/SidePanelSystem.hpp>

#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Component/SidePanelElementComponent.hpp>
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/FruitIcon.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/OfflineReward.hpp>
#include <FruitMagic/Game/RecentHarvests.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //--------------------------------------------------------------
        // 台の上の果物を、果物とバリエーションごとにまとめたもの
        //--------------------------------------------------------------
        struct TableFruit {
            int       fruitIndex   = -1;
            int       variantIndex = 0;
            int       count        = 0;    // 個数
            long long value        = 0;    // 全部とれたときに増える果実
        };

        //--------------------------------------------------------------
        // おすすめの強化
        //--------------------------------------------------------------
        struct Recommendation {
            const UpgradeDef* def    = nullptr;    // 強化（すべて最大なら nullptr）
            int               level  = 0;          // 今のレベル
            bool              canBuy = false;      // 今の手持ちで買えるか
        };

        //--------------------------------------------------------------
        //! 秒数を表示用の文字列にします（小数は1桁まで。整数なら小数を付けない）。
        //! @param  [in] seconds 秒数
        //! @return 表示用の文字列
        //--------------------------------------------------------------
        std::wstring FormatSeconds(float seconds) {
            const int tenths = static_cast<int>(std::lround(seconds * 10.0f));
            if(tenths % 10 == 0)
                return std::to_wstring(tenths / 10);
            return std::to_wstring(tenths / 10) + L"." + std::to_wstring(tenths % 10);
        }

        //--------------------------------------------------------------
        //! 台の上の果物を、果物とバリエーションごとにまとめ、価値の高い順に並べます。
        //! @param  [in] registry レジストリ
        //! @return まとめた果物（価値の高い順）
        //--------------------------------------------------------------
        std::vector<TableFruit> CollectTableFruits(Tsukino::ECS::Registry& registry) {
            std::vector<TableFruit> result;
            const auto* variants = registry.HasContext<CollectionConfig>() ? &registry.GetContext<CollectionConfig>().Variants() : nullptr;

            registry.View<PrizeComponent>().each([&](Tsukino::ECS::Entity, PrizeComponent& prize) {
                if(prize.kind != PrizeKind::Fruit || prize.fruitIndex < 0)
                    return;

                // 収穫したときと同じ計算（価値 × バリエーションの倍率 × 大きくした倍率）
                long long value = static_cast<long long>(prize.value) * std::max(1, prize.valueMultiplier);
                if(variants && prize.variantIndex >= 0 && prize.variantIndex < static_cast<int>(variants->size()))
                    value *= (*variants)[prize.variantIndex].valueMultiplier;

                auto it = std::find_if(result.begin(), result.end(), [&](const TableFruit& f) {
                    return f.fruitIndex == prize.fruitIndex && f.variantIndex == prize.variantIndex;
                });
                if(it == result.end())
                    it = result.insert(result.end(), TableFruit{prize.fruitIndex, prize.variantIndex, 0, 0});
                it->count += 1;
                it->value += value;
            });

            std::sort(result.begin(), result.end(), [](const TableFruit& a, const TableFruit& b) { return a.value > b.value; });
            return result;
        }

        //--------------------------------------------------------------
        //! おすすめの強化を選びます。今買えるものがあればその中で一番安いもの、無ければ一番安いものです。
        //! @param  [in] registry レジストリ
        //! @param  [in] state    プレイヤーの資源
        //! @return おすすめの強化
        //--------------------------------------------------------------
        Recommendation PickUpgrade(Tsukino::ECS::Registry& registry, const GameState& state) {
            Recommendation best;
            if(!registry.HasContext<UpgradeCatalog>())
                return best;

            for(const UpgradeDef& def : registry.GetContext<UpgradeCatalog>().Upgrades()) {
                const int level = state.UpgradeLevelOf(def.id);
                if(level >= def.MaxLevel())
                    continue;

                const bool canBuy = UpgradeSystem::CanAfford(def, state);
                if(best.def) {
                    // 買えるものを優先し、同じなら必要なコイン（同じなら果実）の少ない方
                    const UpgradeLevel& next = def.levels[level];
                    const UpgradeLevel& held = best.def->levels[best.level];
                    if(canBuy != best.canBuy) {
                        if(!canBuy)
                            continue;
                    } else if(next.coins != held.coins ? next.coins > held.coins : next.fruit >= held.fruit) {
                        continue;
                    }
                }
                best = Recommendation{&def, level, canBuy};
            }
            return best;
        }

        //--------------------------------------------------------------
        //! 果物の表示名（バリエーション名付き）と色を返します。
        //! @param  [in]  registry     レジストリ
        //! @param  [in]  fruitIndex   果物の添字
        //! @param  [in]  variantIndex バリエーションの添字
        //! @param  [out] color        表示色
        //! @return 表示名。範囲外なら空文字列
        //--------------------------------------------------------------
        std::wstring FruitLabel(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex, hlslpp::float3& color) {
            color = hlslpp::float3(1.0f, 1.0f, 1.0f);
            if(!registry.HasContext<FruitCatalog>())
                return std::wstring();
            const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
            if(fruitIndex < 0 || fruitIndex >= static_cast<int>(fruits.size()))
                return std::wstring();

            const FruitDef& def = fruits[fruitIndex];
            color               = def.color;
            if(!registry.HasContext<CollectionConfig>())
                return def.name;

            const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
            if(variantIndex >= 0 && variantIndex < static_cast<int>(collection.Variants().size()))
                color = collection.Variants()[variantIndex].ColorOf(def);
            return collection.DisplayName(def, variantIndex);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! パネルの文字と色を更新します。
    //----------------------------------------------------------------------------
    void SidePanelSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        const UiConfig& ui    = GetUiConfig(registry);
        const Texts&    texts = GetTexts(registry);

        // 画面（図鑑・強化・おかえり・オプション）を開いている間は隠す（画面の後ろで文字が重ならないように）
        const bool visible = !registry.HasContext<MenuState>() || registry.GetContext<MenuState>().open == MenuKind::None;

        //--------------------------------------------------------------
        // 出す情報を集める（隠している間は集めない）
        //--------------------------------------------------------------
        const GameState                 state   = registry.HasContext<GameState>() ? registry.GetContext<GameState>() : GameState{};
        static const RecentHarvests     kNoHarvests;
        const RecentHarvests&           recent  = registry.HasContext<RecentHarvests>() ? registry.GetContext<RecentHarvests>() : kNoHarvests;
        const TableStats                stats   = registry.HasContext<TableStats>() ? registry.GetContext<TableStats>() : TableStats{};
        std::vector<TableFruit>         table;
        Recommendation                  upgrade;
        int                             registered = 0;
        int                             total      = 0;
        const MagicDef*                 nextMagic  = nullptr;
        if(visible) {
            table      = CollectTableFruits(registry);
            upgrade    = PickUpgrade(registry, state);
            registered = state.RegisteredCount();
            if(registry.HasContext<FruitCatalog>() && registry.HasContext<CollectionConfig>())
                total = static_cast<int>(registry.GetContext<FruitCatalog>().Fruits().size() * registry.GetContext<CollectionConfig>().Variants().size());

            // まだ覚えていない魔法のうち、一番早く覚えるもの
            if(registry.HasContext<MagicCatalog>()) {
                for(const MagicDef& magic : registry.GetContext<MagicCatalog>().Magics()) {
                    if(magic.unlockZukan > registered && (!nextMagic || magic.unlockZukan < nextMagic->unlockZukan))
                        nextMagic = &magic;
                }
            }
        }

        // 一覧の行の中身があるか
        auto hasRecent = [&](int row) { return row >= 0 && row < static_cast<int>(recent.entries.size()); };
        auto hasTable  = [&](int row) { return row >= 0 && row < static_cast<int>(table.size()); };

        registry.View<SidePanelElementComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, SidePanelElementComponent& element, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                //--------------------------------------------------------------
                // 行頭の果物（3D）。行の中身が変わったときだけ作り直し、行が空のときとパネルを隠している間は隠す
                //--------------------------------------------------------------
                if(element.kind == SidePanelElementKind::RecentFruit || element.kind == SidePanelElementKind::TableFruit) {
                    int fruitIndex   = -1;
                    int variantIndex = 0;
                    if(element.kind == SidePanelElementKind::RecentFruit && hasRecent(element.row)) {
                        fruitIndex   = recent.entries[element.row].fruitIndex;
                        variantIndex = recent.entries[element.row].variantIndex;
                    } else if(element.kind == SidePanelElementKind::TableFruit && hasTable(element.row)) {
                        fruitIndex   = table[element.row].fruitIndex;
                        variantIndex = table[element.row].variantIndex;
                    }
                    // 隠している間は台の上を数えていないので、中身はそのままにして隠すだけ
                    if(visible)
                        SetFruitIcon(registry, entity, fruitIndex, variantIndex, false, ui.sideIconSize);
                    SetFruitIconVisible(registry, entity, visible && fruitIndex >= 0);
                    return;
                }

                //--------------------------------------------------------------
                // スプライト（板・バー・おすすめの強化の板）
                //--------------------------------------------------------------
                if(auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                    bool           shown = visible;
                    hlslpp::float3 scale = element.shownScale;
                    switch(element.kind) {
                        case SidePanelElementKind::ZukanBar: {
                            // 左端を固定したまま、登録した割合だけ伸ばす
                            const float ratio  = total > 0 ? std::clamp(static_cast<float>(registered) / static_cast<float>(total), 0.0f, 1.0f) : 0.0f;
                            shown              = shown && ratio > 0.0f;
                            scale              = hlslpp::float3(float(element.shownScale.x) * ratio, float(element.shownScale.y), 1.0f);
                            transform.position = hlslpp::float3(element.barLeft + element.barWidth * ratio * 0.5f, float(transform.position.y), 0.0f);
                            break;
                        }

                        case SidePanelElementKind::UpgradeButton: {
                            shown               = shown && upgrade.def != nullptr;
                            const auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                            sprite->tintColor   = (pointer && pointer->hovered) ? ui.sideButtonHoverColor : upgrade.canBuy ? ui.sideButtonReadyColor : ui.sideButtonColor;
                            break;
                        }

                        default:
                            break;
                    }
                    transform.scale = shown ? scale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                    transform.dirty = true;
                }

                //--------------------------------------------------------------
                // 文字
                //--------------------------------------------------------------
                auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
                if(!font)
                    return;
                font->text.clear();
                if(!visible)
                    return;

                hlslpp::float3 color;
                switch(element.kind) {
                    case SidePanelElementKind::Static:
                        font->text = element.text;
                        break;

                    case SidePanelElementKind::RecentName:
                        if(hasRecent(element.row)) {
                            const RecentHarvest& harvest = recent.entries[element.row];
                            const std::wstring   name    = FruitLabel(registry, harvest.fruitIndex, harvest.variantIndex, color);
                            font->text                   = harvest.isNew ? texts.Format("side.recentNew", {{"name", name}}) : name;
                            font->color                  = harvest.isNew ? ui.sideNewColor : ui.sideText.color;
                        } else if(element.row == 0) {
                            font->text  = texts.Get("side.recentEmpty");
                            font->color = ui.sideDimColor;
                        }
                        break;

                    case SidePanelElementKind::RecentValue:
                        if(hasRecent(element.row))
                            font->text = texts.Format("side.recentValue", {{"n", std::to_wstring(recent.entries[element.row].value)}});
                        break;

                    case SidePanelElementKind::TableName:
                        if(hasTable(element.row)) {
                            const TableFruit& fruit = table[element.row];
                            font->text  = texts.Format("side.tableName", {{"name", FruitLabel(registry, fruit.fruitIndex, fruit.variantIndex, color)}, {"n", std::to_wstring(fruit.count)}});
                            font->color = ui.sideText.color;
                        } else if(element.row == 0) {
                            font->text  = texts.Get("side.tableEmpty");
                            font->color = ui.sideDimColor;
                        }
                        break;

                    case SidePanelElementKind::TableValue:
                        if(hasTable(element.row))
                            font->text = texts.Format("side.tableValue", {{"n", std::to_wstring(table[element.row].value)}});
                        break;

                    case SidePanelElementKind::TableTotal: {
                        if(table.empty())
                            break;
                        int       count = 0;
                        long long value = 0;
                        for(const TableFruit& fruit : table) {
                            count += fruit.count;
                            value += fruit.value;
                        }
                        font->text = texts.Format("side.tableTotal", {{"n", std::to_wstring(count)}, {"value", std::to_wstring(value)}});
                        break;
                    }

                    case SidePanelElementKind::ZukanCount:
                        font->text = texts.Format("side.zukanCount", {{"registered", std::to_wstring(registered)}, {"total", std::to_wstring(total)}});
                        break;

                    case SidePanelElementKind::ZukanNext:
                        font->text = nextMagic ? texts.Format("side.zukanNext", {{"n", std::to_wstring(nextMagic->unlockZukan - registered)}, {"name", nextMagic->name}})
                                               : texts.Get("side.zukanAllMagic");
                        break;

                    case SidePanelElementKind::UpgradeName:
                        if(upgrade.def) {
                            font->text  = texts.Format("side.upgradeName", {{"name", upgrade.def->name}, {"level", std::to_wstring(upgrade.level)}, {"next", std::to_wstring(upgrade.level + 1)}});
                            font->color = ui.sideText.color;
                        } else {
                            font->text  = texts.Get("side.upgradeAllMax");
                            font->color = ui.sideDimColor;
                        }
                        break;

                    case SidePanelElementKind::UpgradeCost:
                        if(upgrade.def) {
                            const UpgradeLevel& next = upgrade.def->levels[upgrade.level];
                            std::wstring        cost = texts.Format("upgrade.costCoins", {{"n", std::to_wstring(next.coins)}});
                            if(next.fruit > 0)
                                cost += texts.Format("upgrade.costFruit", {{"n", std::to_wstring(next.fruit)}});
                            font->text  = texts.Format(upgrade.canBuy ? "side.upgradeReady" : "side.upgradeShort", {{"cost", cost}});
                            font->color = upgrade.canBuy ? ui.sideValue.color : ui.sideDimColor;
                        }
                        break;

                    case SidePanelElementKind::Fairy:
                        if(stats.autoLaunchInterval > 0.0f) {
                            font->text  = texts.Format("side.fairy", {{"n", FormatSeconds(stats.autoLaunchInterval)}});
                            font->color = ui.sideText.color;
                        } else {
                            font->text  = texts.Get("side.noFairy");
                            font->color = ui.sideDimColor;
                        }
                        break;

                    case SidePanelElementKind::Offline:
                        font->text = texts.Format("side.offline", {{"limit", FormatDuration(static_cast<long long>(stats.offlineMaxHours * 3600.0f), texts)}});
                        break;

                    default:
                        break;
                }
            });
    }
}    // namespace FruitMagic::ECS

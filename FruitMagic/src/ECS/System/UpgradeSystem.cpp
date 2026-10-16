//----------------------------------------------------------------------------
//! @file   UpgradeSystem.cpp
//! @brief  台の強化のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>

#include <FruitMagic/ECS/Component/UpgradeElementComponent.hpp>
#include <FruitMagic/Game/ConfirmState.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/ManaConfig.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/MoveConfig.hpp>
#include <FruitMagic/Game/MoveRules.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/SouvenirCatalog.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UiMotion.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UIVisibilityComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 購入の入力を読み、強化画面を更新します。
    //----------------------------------------------------------------------------
    void UpgradeSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<GameState>() || !registry.HasContext<UpgradeCatalog>())
            return;
        const bool open = registry.GetContext<MenuState>().IsOpen(MenuKind::Upgrade);
        const bool justOpened = open && !m_wasOpen;
        m_wasOpen             = open;
        if(!open)
            return;

        const UpgradeCatalog& catalog = registry.GetContext<UpgradeCatalog>();
        const auto&           defs    = catalog.Upgrades();

        //--------------------------------------------------------------
        // タブ・購入ボタン・おひっこしボタンのクリック（隠れているタブの中身はエンジンが当たり判定から外す。
        // 確認ウィンドウを出している間は受け付けない）
        //--------------------------------------------------------------
        MenuState&    menu         = registry.GetContext<MenuState>();
        ConfirmState* confirm      = registry.HasContext<ConfirmState>() ? &registry.GetContext<ConfirmState>() : nullptr;
        const bool    confirming   = confirm && confirm->IsConfirming();
        const bool    canMove      = MoveRules::CanMove(registry);
        int           clickedIndex = -1;
        registry.View<UpgradeElementComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, UpgradeElementComponent& element, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                if(!pointer.clicked || confirming)
                    return;
                if(element.kind == UpgradeElementKind::BuyButton)
                    clickedIndex = element.upgradeIndex;
                else if(element.kind == UpgradeElementKind::Tab)
                    menu.upgradeTab = element.currency;
                else if(element.kind == UpgradeElementKind::MoveButton && canMove && confirm)
                    confirm->Open(ConfirmPurpose::Move, MenuKind::Upgrade);
            });
        if(clickedIndex >= 0)
            Purchase(registry, clickedIndex);

        // 選んでいるタブの中身だけを見せる
        registry.View<UpgradeElementComponent, Tsukino::BuiltIn::ECS::UIVisibilityComponent>().each(
            [&](Tsukino::ECS::Entity, UpgradeElementComponent& element, Tsukino::BuiltIn::ECS::UIVisibilityComponent& visibility) {
                if(element.kind == UpgradeElementKind::TabGroup)
                    visibility.visible = (element.currency == menu.upgradeTab);
            });

        //--------------------------------------------------------------
        // 画面の文字・ボタンの色
        //--------------------------------------------------------------
        const GameState& state = registry.GetContext<GameState>();
        const UiConfig&  ui    = GetUiConfig(registry);
        const Texts&     texts = GetTexts(registry);

        // 手持ちの表示は、買ったときに減っていく様子が見えるよう実際の数へ追いつかせる（開いた瞬間はそろえる）
        if(justOpened) {
            m_walletCoins.Snap(state.coins);
            m_walletFruit.Snap(state.fruitPoints);
            m_walletSeeds.Snap(state.starSeeds);
        }
        const long long shownCoins = m_walletCoins.Update(state.coins, ui.motion.countUpSpeed, deltaTime);
        const long long shownFruit = m_walletFruit.Update(state.fruitPoints, ui.motion.countUpSpeed, deltaTime);
        const long long shownSeeds = m_walletSeeds.Update(state.starSeeds, ui.motion.countUpSpeed, deltaTime);

        registry.View<UpgradeElementComponent>().each([&](Tsukino::ECS::Entity entity, UpgradeElementComponent& element) {
            if(element.kind == UpgradeElementKind::Wallet) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity))
                    font->text = texts.Format("upgrade.wallet", {{"coins", std::to_wstring(shownCoins)}, {"fruit", std::to_wstring(shownFruit)},
                                                                 {"seeds", std::to_wstring(shownSeeds)}});
                if(m_walletCoins.Increased() || m_walletFruit.Increased() || m_walletSeeds.Increased())
                    PlayPunch(registry, entity);
                return;
            }

            //--------------------------------------------------------------
            // おひっこし（星のたねのタブの上）: ボタンの色・文字と、もらえる星のたねの案内
            //--------------------------------------------------------------
            if(element.kind == UpgradeElementKind::MoveButton) {
                auto* sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                if(sprite)
                    sprite->tintColor = !canMove ? ui.upgradeCannotBuyColor : (pointer && pointer->hovered) ? ui.upgradeMoveHoverColor : ui.upgradeMoveColor;
                SetButtonInteractable(registry, entity, canMove);
                SetPulse(registry, entity, canMove);
                return;
            }
            if(element.kind == UpgradeElementKind::MoveLabel || element.kind == UpgradeElementKind::MoveInfo) {
                auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
                if(!font)
                    return;
                if(element.kind == UpgradeElementKind::MoveLabel) {
                    font->text = texts.Get("move.button");
                } else if(!MoveRules::IsUnlocked(registry)) {
                    const int stage = registry.HasContext<MoveConfig>() ? registry.GetContext<MoveConfig>().unlockTreeStage : MoveConfig{}.unlockTreeStage;
                    font->text      = texts.Format("move.locked", {{"stage", std::to_wstring(stage)}});
                } else {
                    font->text = texts.Format(canMove ? "move.info" : "move.notYet", {{"seeds", std::to_wstring(MoveRules::SeedsIfMovedNow(registry))}});
                }
                return;
            }
            if(element.kind == UpgradeElementKind::Tab) {
                auto* sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                if(sprite)
                    sprite->tintColor = (element.currency == menu.upgradeTab) ? ui.upgradeTabSelectedColor
                                        : (pointer && pointer->hovered)     ? ui.upgradeTabHoverColor
                                                                            : ui.upgradeTabColor;
                return;
            }
            if(element.kind == UpgradeElementKind::TabGroup)
                return;

            if(element.upgradeIndex < 0 || element.upgradeIndex >= static_cast<int>(defs.size()))
                return;

            const UpgradeDef& def     = defs[element.upgradeIndex];
            const int         level   = state.UpgradeLevelOf(def.id);
            const bool        maxed   = level >= def.MaxLevel();
            const bool        canBuy  = CanAfford(def, state);

            if(element.kind == UpgradeElementKind::CostTag) {
                if(auto* visibility = registry.try_get<Tsukino::BuiltIn::ECS::UIVisibilityComponent>(entity))
                    visibility->visible = !maxed;
                return;
            }

            if(element.kind == UpgradeElementKind::BuyButton) {
                auto* sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                if(sprite)
                    sprite->tintColor = maxed ? ui.upgradeMaxedColor : !canBuy ? ui.upgradeCannotBuyColor : (pointer && pointer->hovered) ? ui.upgradeBuyHoverColor : ui.upgradeBuyColor;
                // 買える間だけ手ごたえを出し、呼吸させて押せることを知らせる
                SetButtonInteractable(registry, entity, !maxed && canBuy);
                SetPulse(registry, entity, !maxed && canBuy);
                return;
            }

            auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
            if(!font)
                return;

            switch(element.kind) {
                case UpgradeElementKind::Name:
                    font->text = texts.Format("upgrade.name", {{"name", def.name}, {"level", std::to_wstring(level)}, {"max", std::to_wstring(def.MaxLevel())}});
                    break;

                case UpgradeElementKind::Effect: {
                    const std::wstring now = def.FormatValue(def.ValueAt(level));
                    font->text             = maxed ? texts.Format("upgrade.effectMax", {{"now", now}})
                                                   : texts.Format("upgrade.effectNext", {{"now", now}, {"next", def.FormatValue(def.ValueAt(level + 1))}});
                    break;
                }

                case UpgradeElementKind::Cost: {
                    if(maxed) {
                        font->text.clear();
                        break;
                    }
                    font->text  = CostText(def, state, texts);
                    font->color = canBuy ? ui.upgradeAffordableColor : ui.upgradeShortColor;
                    break;
                }

                case UpgradeElementKind::BuyLabel:
                    font->text = texts.Get(maxed ? "upgrade.maxed" : canBuy ? "upgrade.buy" : "upgrade.short");
                    break;

                default:
                    break;
            }
        });
    }

    //----------------------------------------------------------------------------
    //! 強化の次のレベルを今の手持ちで買えるかを返します。
    //----------------------------------------------------------------------------
    bool UpgradeSystem::CanAfford(const UpgradeDef& def, const GameState& state) {
        const int level = state.UpgradeLevelOf(def.id);
        if(level >= def.MaxLevel())
            return false;
        const long long cost = def.levels[level].cost;
        switch(def.currency) {
            case UpgradeCurrency::Coins: return state.coins >= cost;
            case UpgradeCurrency::Fruit: return state.fruitPoints >= cost;
            case UpgradeCurrency::Seeds: return state.starSeeds >= cost;
        }
        return false;
    }

    //----------------------------------------------------------------------------
    //! 強化の次のレベルの値段を文字にします。
    //----------------------------------------------------------------------------
    std::wstring UpgradeSystem::CostText(const UpgradeDef& def, const GameState& state, const Texts& texts) {
        const int level = state.UpgradeLevelOf(def.id);
        if(level >= def.MaxLevel())
            return std::wstring();
        const char* key = (def.currency == UpgradeCurrency::Coins)   ? "upgrade.costCoins"
                          : (def.currency == UpgradeCurrency::Fruit) ? "upgrade.costFruit"
                                                                     : "upgrade.costSeeds";
        return texts.Format(key, {{"n", std::to_wstring(def.levels[level].cost)}});
    }

    //----------------------------------------------------------------------------
    //! 今の強化のレベルから、台の性能と果樹の段階を計算し直します。
    //----------------------------------------------------------------------------
    void UpgradeSystem::ApplyUpgrades(Tsukino::ECS::Registry& registry) {
        if(!registry.HasContext<UpgradeCatalog>() || !registry.HasContext<GameState>() || !registry.HasContext<TableStats>())
            return;

        GameState&  state = registry.GetContext<GameState>();
        TableStats& stats = registry.GetContext<TableStats>();
        const TableLayout& layout = GetTableLayout(registry);

        //--------------------------------------------------------------
        // 強化の無い項目は定義データの既定の値（Table.json・Roulette.json・Mana.json）。
        // 下で強化のある項目だけを上書きする
        //--------------------------------------------------------------
        const RouletteConfig roulette = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};
        stats                    = TableStats{};
        stats.pusherAmplitude    = layout.pusherAmplitude;
        stats.pusherPeriod       = layout.pusherPeriod;
        stats.rouletteMaxStock   = roulette.maxStock;
        stats.rouletteCoinAmount = roulette.coinAmount;
        stats.rouletteHitChance  = roulette.hitChance;
        state.maxMana            = registry.HasContext<ManaConfig>() ? registry.GetContext<ManaConfig>().maxMana : ManaConfig{}.maxMana;

        const auto& defs = registry.GetContext<UpgradeCatalog>().Upgrades();
        for(const UpgradeDef& def : defs) {
            if(def.currency == UpgradeCurrency::Seeds)
                continue;    // 星のたねの強化は、下でコインと FP の強化の値に重ねる
            const float value = def.ValueAt(state.UpgradeLevelOf(def.id));

            //--------------------------------------------------------------
            // 台（コイン）
            //--------------------------------------------------------------
            if(def.id == "pusherStroke") {
                // 振幅の上限はプッシャーの奥行で決まる（Table.json の pusher.maxAmplitude）。超える値は丸める
                stats.pusherAmplitude = std::clamp(value, 1.0f, layout.pusherMaxAmplitude);
            } else if(def.id == "pusherSpeed") {
                // データは往復の周期（秒）。短すぎると景品を弾き飛ばすので下限を付ける
                stats.pusherPeriod = std::max(1.0f, value);
            } else if(def.id == "fairy") {
                stats.autoLaunchInterval = std::max(0.0f, value);
            } else if(def.id == "checkerWidth") {
                // データは穴の全幅。払い出し口からはみ出さない幅に丸める
                stats.checkerHalfWidth = std::clamp(value * 0.5f, 0.5f, layout.payoutHalfWidth);
            } else if(def.id == "offlineHours") {
                stats.offlineMaxHours = std::max(0.0f, value);
            } else if(def.id == "rouletteStock") {
                stats.rouletteMaxStock = std::max(1, static_cast<int>(value));
            } else if(def.id == "rouletteCoins") {
                stats.rouletteCoinAmount = std::max(0, static_cast<int>(value));
            } else if(def.id == "gutterCharm") {
                // データは戻る割合（%）
                stats.gutterRefundRate = std::clamp(value / 100.0f, 0.0f, 1.0f);
            }
            //--------------------------------------------------------------
            // 果物と魔法（FP）
            //--------------------------------------------------------------
            else if(def.id == "treeLevel") {
                state.treeLevel = std::max(0, static_cast<int>(value));
            } else if(def.id == "fruitValue") {
                // データは倍率の %（150 なら 1.5 倍）
                stats.fruitValueMultiplier = std::max(1.0f, value / 100.0f);
            } else if(def.id == "variantChance") {
                stats.variantChanceMultiplier = std::max(1.0f, value / 100.0f);
            } else if(def.id == "rouletteHit") {
                // データは確率（%）
                stats.rouletteHitChance = std::clamp(value / 100.0f, 0.0f, 1.0f);
            } else if(def.id == "manaGain") {
                stats.manaMultiplier = std::max(1.0f, value / 100.0f);
            } else if(def.id == "maxMana") {
                state.maxMana = std::max(1, static_cast<int>(value));
            } else if(def.id == "magicDuration") {
                stats.magicDurationMultiplier = std::max(1.0f, value / 100.0f);
            } else {
                Tsukino::Core::Log::Warn("UpgradeSystem: upgrade \"" + def.id + "\" has no effect.");
            }
        }

        //--------------------------------------------------------------
        // おひっこしをまたいで残る強化（星のたね）。倍率は FP の強化の倍率に掛け、おるすばんは時間を足す
        //--------------------------------------------------------------
        for(const UpgradeDef& def : defs) {
            if(def.currency != UpgradeCurrency::Seeds)
                continue;
            const float value = def.ValueAt(state.UpgradeLevelOf(def.id));
            if(def.id == "seedFruitValue") {
                stats.fruitValueMultiplier *= std::max(1.0f, value / 100.0f);
            } else if(def.id == "seedMana") {
                stats.manaMultiplier *= std::max(1.0f, value / 100.0f);
            } else if(def.id == "seedVariant") {
                stats.variantChanceMultiplier *= std::max(1.0f, value / 100.0f);
            } else if(def.id == "seedOffline") {
                // データは足す時間
                stats.offlineMaxHours += std::max(0.0f, value);
            } else if(def.id == "seedGain") {
                stats.seedGainMultiplier = std::max(1.0f, value / 100.0f);
            } else if(def.id == "startCoins" || def.id == "startFairy") {
                // おひっこしの時に MoveRules::Move が使う（台の性能には効かない）
            } else {
                Tsukino::Core::Log::Warn("UpgradeSystem: upgrade \"" + def.id + "\" has no effect.");
            }
        }

        //--------------------------------------------------------------
        // おみやげ（おひっこしでもらう。レベル × 1レベルごとの量）。倍率は上の倍率に掛け、確率と時間は足す
        //--------------------------------------------------------------
        if(registry.HasContext<SouvenirCatalog>()) {
            for(const SouvenirDef& def : registry.GetContext<SouvenirCatalog>().Souvenirs()) {
                const int level = state.SouvenirLevelOf(def.id);
                if(level <= 0)
                    continue;
                const float amount = def.perLevel * static_cast<float>(level);
                switch(def.effect) {
                    case SouvenirEffect::FruitValue: stats.fruitValueMultiplier *= 1.0f + amount / 100.0f; break;
                    case SouvenirEffect::Mana: stats.manaMultiplier *= 1.0f + amount / 100.0f; break;
                    case SouvenirEffect::Variant: stats.variantChanceMultiplier *= 1.0f + amount / 100.0f; break;
                    case SouvenirEffect::SeedGain: stats.seedGainMultiplier *= 1.0f + amount / 100.0f; break;
                    case SouvenirEffect::RouletteHit: stats.rouletteHitChance = std::min(1.0f, stats.rouletteHitChance + amount / 100.0f); break;
                    case SouvenirEffect::MagicDuration: stats.magicDurationMultiplier *= 1.0f + amount / 100.0f; break;
                    case SouvenirEffect::OfflineHours: stats.offlineMaxHours += amount; break;
                    case SouvenirEffect::StartCoins: break;    // おひっこしの時に MoveRules::Move が足す
                }
            }
        }
        state.mana = std::min(state.mana, state.maxMana);
    }

    //----------------------------------------------------------------------------
    //! 強化を1段階買います。
    //----------------------------------------------------------------------------
    bool UpgradeSystem::Purchase(Tsukino::ECS::Registry& registry, int upgradeIndex) {
        const auto& defs = registry.GetContext<UpgradeCatalog>().Upgrades();
        if(upgradeIndex < 0 || upgradeIndex >= static_cast<int>(defs.size()))
            return false;

        GameState&        state = registry.GetContext<GameState>();
        const UpgradeDef& def   = defs[upgradeIndex];
        if(!CanAfford(def, state))
            return false;

        const int       level = state.UpgradeLevelOf(def.id);
        const long long cost  = def.levels[level].cost;
        switch(def.currency) {
            case UpgradeCurrency::Coins: state.coins -= static_cast<int>(cost); break;
            case UpgradeCurrency::Fruit: state.fruitPoints -= cost; break;
            case UpgradeCurrency::Seeds: state.starSeeds -= cost; break;
        }
        state.upgradeLevels[def.id] = level + 1;

        ApplyUpgrades(registry);
        Tsukino::Core::Log::Info("UpgradeSystem: " + def.id + " -> Lv " + std::to_string(level + 1));
        return true;
    }
}    // namespace FruitMagic::ECS

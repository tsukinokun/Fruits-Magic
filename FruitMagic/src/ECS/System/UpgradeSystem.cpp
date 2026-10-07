//----------------------------------------------------------------------------
//! @file   UpgradeSystem.cpp
//! @brief  台の強化のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>

#include <FruitMagic/ECS/Component/UpgradeElementComponent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 購入の入力を読み、強化画面を更新します。
    //----------------------------------------------------------------------------
    void UpgradeSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<GameState>() || !registry.HasContext<UpgradeCatalog>())
            return;
        if(!registry.GetContext<MenuState>().IsOpen(MenuKind::Upgrade))
            return;

        const UpgradeCatalog& catalog = registry.GetContext<UpgradeCatalog>();
        const auto&           defs    = catalog.Upgrades();

        //--------------------------------------------------------------
        // 購入ボタンのクリック
        //--------------------------------------------------------------
        int clickedIndex = -1;
        registry.View<UpgradeElementComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, UpgradeElementComponent& element, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                if(element.kind == UpgradeElementKind::BuyButton && pointer.clicked)
                    clickedIndex = element.upgradeIndex;
            });
        if(clickedIndex >= 0)
            Purchase(registry, clickedIndex);

        //--------------------------------------------------------------
        // 画面の文字・ボタンの色
        //--------------------------------------------------------------
        const GameState& state = registry.GetContext<GameState>();
        const UiConfig&  ui    = GetUiConfig(registry);
        const Texts&     texts = GetTexts(registry);
        registry.View<UpgradeElementComponent>().each([&](Tsukino::ECS::Entity entity, UpgradeElementComponent& element) {
            if(element.kind == UpgradeElementKind::Wallet) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity))
                    font->text = texts.Format("upgrade.wallet", {{"coins", std::to_wstring(state.coins)}, {"fruit", std::to_wstring(state.fruitPoints)}});
                return;
            }

            if(element.upgradeIndex < 0 || element.upgradeIndex >= static_cast<int>(defs.size()))
                return;

            const UpgradeDef& def     = defs[element.upgradeIndex];
            const int         level   = state.UpgradeLevelOf(def.id);
            const bool        maxed   = level >= def.MaxLevel();
            const bool        canBuy  = CanAfford(def, state);

            if(element.kind == UpgradeElementKind::BuyButton) {
                auto* sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                if(sprite)
                    sprite->tintColor = maxed ? ui.upgradeMaxedColor : !canBuy ? ui.upgradeCannotBuyColor : (pointer && pointer->hovered) ? ui.upgradeBuyHoverColor : ui.upgradeBuyColor;
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
                    const UpgradeLevel& next = def.levels[level];
                    std::wstring        text = texts.Format("upgrade.costCoins", {{"n", std::to_wstring(next.coins)}});
                    if(next.fruit > 0)
                        text += texts.Format("upgrade.costFruit", {{"n", std::to_wstring(next.fruit)}});
                    font->text  = text;
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
        const UpgradeLevel& next = def.levels[level];
        return state.coins >= next.coins && state.fruitPoints >= next.fruit;
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

        // 押し幅の強化が無いときは台の既定の振幅（Table.json）
        stats.pusherAmplitude = layout.pusherAmplitude;

        for(const UpgradeDef& def : registry.GetContext<UpgradeCatalog>().Upgrades()) {
            const float value = def.ValueAt(state.UpgradeLevelOf(def.id));

            if(def.id == "pusherStroke") {
                // 振幅の上限はプッシャーの奥行で決まる（Table.json の pusher.maxAmplitude）。超える値は丸める
                stats.pusherAmplitude = std::clamp(value, 1.0f, layout.pusherMaxAmplitude);
            } else if(def.id == "treeLevel") {
                state.treeLevel = std::max(0, static_cast<int>(value));
            } else if(def.id == "fairy") {
                stats.autoLaunchInterval = std::max(0.0f, value);
            } else if(def.id == "checkerWidth") {
                // データは穴の全幅。払い出し口からはみ出さない幅に丸める
                stats.checkerHalfWidth = std::clamp(value * 0.5f, 0.5f, layout.payoutHalfWidth);
            } else if(def.id == "offlineHours") {
                stats.offlineMaxHours = std::max(0.0f, value);
            } else {
                Tsukino::Core::Log::Warn("UpgradeSystem: upgrade \"" + def.id + "\" has no effect. Known ids: pusherStroke, treeLevel, fairy, checkerWidth, offlineHours.");
            }
        }
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

        const int           level = state.UpgradeLevelOf(def.id);
        const UpgradeLevel& next  = def.levels[level];
        state.coins -= next.coins;
        state.fruitPoints -= next.fruit;
        state.upgradeLevels[def.id] = level + 1;

        ApplyUpgrades(registry);
        Tsukino::Core::Log::Info("UpgradeSystem: " + def.id + " -> Lv " + std::to_string(level + 1));
        return true;
    }
}    // namespace FruitMagic::ECS

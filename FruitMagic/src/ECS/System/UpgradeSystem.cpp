//----------------------------------------------------------------------------
//! @file   UpgradeSystem.cpp
//! @brief  台の強化のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/UpgradeSystem.hpp>

#include <FruitMagic/ECS/Component/UpgradeElementComponent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>
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
    namespace {
        //--------------------------------------------------------------
        // 購入ボタンの色
        //--------------------------------------------------------------
        const hlslpp::float4 kBuyColor        = hlslpp::float4(0.35f, 0.78f, 0.45f, 1.0f);    // 買える
        const hlslpp::float4 kBuyHoverColor   = hlslpp::float4(0.5f, 0.92f, 0.6f, 1.0f);      // 買える（カーソルが重なっている）
        const hlslpp::float4 kCannotBuyColor  = hlslpp::float4(0.3f, 0.28f, 0.34f, 1.0f);     // 手持ちが足りない
        const hlslpp::float4 kMaxedColor      = hlslpp::float4(0.75f, 0.6f, 0.25f, 1.0f);     // 最大レベル

        //--------------------------------------------------------------
        // 価格の文字の色
        //--------------------------------------------------------------
        const hlslpp::float4 kAffordableTextColor = hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f);    // 足りる
        const hlslpp::float4 kShortTextColor      = hlslpp::float4(1.0f, 0.55f, 0.55f, 1.0f);   // 足りない

        //--------------------------------------------------------------
        //! 強化の次のレベルを今の手持ちで買えるかを返します。
        //! @param  [in] def   強化の定義
        //! @param  [in] state プレイヤーの資源
        //! @return 買えれば true（最大レベルなら false）
        //--------------------------------------------------------------
        bool CanAfford(const UpgradeDef& def, const GameState& state) {
            const int level = state.UpgradeLevelOf(def.id);
            if(level >= def.MaxLevel())
                return false;
            const UpgradeLevel& next = def.levels[level];
            return state.coins >= next.coins && state.fruitPoints >= next.fruit;
        }
    }    // namespace

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
        registry.View<UpgradeElementComponent>().each([&](Tsukino::ECS::Entity entity, UpgradeElementComponent& element) {
            if(element.kind == UpgradeElementKind::Wallet) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity))
                    font->text = L"手持ち   コイン " + std::to_wstring(state.coins) + L"     果実 " + std::to_wstring(state.fruitPoints);
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
                    sprite->tintColor = maxed ? kMaxedColor : !canBuy ? kCannotBuyColor : (pointer && pointer->hovered) ? kBuyHoverColor : kBuyColor;
                return;
            }

            auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
            if(!font)
                return;

            switch(element.kind) {
                case UpgradeElementKind::Name:
                    font->text = def.name + L"   Lv " + std::to_wstring(level) + L" / " + std::to_wstring(def.MaxLevel());
                    break;

                case UpgradeElementKind::Effect: {
                    const std::wstring now = def.FormatValue(def.ValueAt(level));
                    font->text             = maxed ? now + L"（最大）" : now + L" → " + def.FormatValue(def.ValueAt(level + 1));
                    break;
                }

                case UpgradeElementKind::Cost: {
                    if(maxed) {
                        font->text.clear();
                        break;
                    }
                    const UpgradeLevel& next = def.levels[level];
                    std::wstring        text = L"コイン " + std::to_wstring(next.coins);
                    if(next.fruit > 0)
                        text += L"   果実 " + std::to_wstring(next.fruit);
                    font->text  = text;
                    font->color = canBuy ? kAffordableTextColor : kShortTextColor;
                    break;
                }

                case UpgradeElementKind::BuyLabel:
                    font->text = maxed ? L"最大" : canBuy ? L"強化する" : L"足りない";
                    break;

                default:
                    break;
            }
        });
    }

    //----------------------------------------------------------------------------
    //! 今の強化のレベルから、台の性能と果樹の段階を計算し直します。
    //----------------------------------------------------------------------------
    void UpgradeSystem::ApplyUpgrades(Tsukino::ECS::Registry& registry) {
        if(!registry.HasContext<UpgradeCatalog>() || !registry.HasContext<GameState>() || !registry.HasContext<TableStats>())
            return;

        GameState&  state = registry.GetContext<GameState>();
        TableStats& stats = registry.GetContext<TableStats>();

        for(const UpgradeDef& def : registry.GetContext<UpgradeCatalog>().Upgrades()) {
            const float value = def.ValueAt(state.UpgradeLevelOf(def.id));

            if(def.id == "pusherStroke") {
                // 振幅の上限はプッシャーの奥行で決まる（PusherLayout.hpp）。超える値は丸める
                stats.pusherAmplitude = std::clamp(value, 1.0f, Layout::kPusherMaxAmplitude);
            } else if(def.id == "treeLevel") {
                state.treeLevel = std::max(0, static_cast<int>(value));
            } else if(def.id == "fairy") {
                stats.autoLaunchInterval = std::max(0.0f, value);
            } else if(def.id == "checkerWidth") {
                // データは穴の全幅。払い出し口からはみ出さない幅に丸める
                stats.checkerHalfWidth = std::clamp(value * 0.5f, 0.5f, Layout::kPayoutHalfWidth);
            } else {
                Tsukino::Core::Log::Warn("UpgradeSystem: upgrade \"" + def.id + "\" has no effect. Known ids: pusherStroke, treeLevel, fairy, checkerWidth.");
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

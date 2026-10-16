//----------------------------------------------------------------------------
//! @file   ZukanSystem.cpp
//! @brief  図鑑画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ZukanSystem.hpp>

#include <FruitMagic/ECS/Component/FruitIconComponent.hpp>
#include <FruitMagic/ECS/Component/ZukanElementComponent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/FruitIcon.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MenuState.hpp>
#include <FruitMagic/Game/SouvenirCatalog.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UIVisibilityComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cmath>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //--------------------------------------------------------------
        //! 枠の収穫数を返します（記録が無ければ 0）。
        //! @param  [in] state        プレイヤーの資源
        //! @param  [in] fruitIndex   果物の添字
        //! @param  [in] variantIndex バリエーションの添字
        //! @return 収穫数
        //--------------------------------------------------------------
        int CountOf(const GameState& state, int fruitIndex, int variantIndex) {
            if(fruitIndex < 0 || fruitIndex >= static_cast<int>(state.harvestCounts.size()))
                return 0;
            const auto& counts = state.harvestCounts[fruitIndex];
            return (variantIndex >= 0 && variantIndex < static_cast<int>(counts.size())) ? counts[variantIndex] : 0;
        }

        //--------------------------------------------------------------
        //! 果物のどれかのバリエーションを収穫済みかを返します。
        //! @param  [in] state      プレイヤーの資源
        //! @param  [in] fruitIndex 果物の添字
        //! @return 1つでも収穫済みなら true
        //--------------------------------------------------------------
        bool AnyRegistered(const GameState& state, int fruitIndex) {
            if(fruitIndex < 0 || fruitIndex >= static_cast<int>(state.harvestCounts.size()))
                return false;
            for(int n : state.harvestCounts[fruitIndex]) {
                if(n > 0)
                    return true;
            }
            return false;
        }

        //--------------------------------------------------------------
        //! おみやげの効果の文言のキー（Texts.json の souvenir.effect の下）を返します。
        //--------------------------------------------------------------
        std::string EffectKey(SouvenirEffect effect) {
            switch(effect) {
                case SouvenirEffect::FruitValue: return "souvenir.effect.fruitValue";
                case SouvenirEffect::Mana: return "souvenir.effect.mana";
                case SouvenirEffect::Variant: return "souvenir.effect.variant";
                case SouvenirEffect::SeedGain: return "souvenir.effect.seedGain";
                case SouvenirEffect::RouletteHit: return "souvenir.effect.rouletteHit";
                case SouvenirEffect::MagicDuration: return "souvenir.effect.magicDuration";
                case SouvenirEffect::OfflineHours: return "souvenir.effect.offlineHours";
                case SouvenirEffect::StartCoins: return "souvenir.effect.startCoins";
            }
            return "souvenir.effect.fruitValue";
        }

        //--------------------------------------------------------------
        //! 効果の量を文字にします（整数なら小数点を付けない）。
        //--------------------------------------------------------------
        std::wstring Amount(float value) {
            const float rounded = std::round(value);
            if(std::abs(value - rounded) < 0.01f)
                return std::to_wstring(static_cast<long long>(rounded));
            std::wstring text = std::to_wstring(std::round(value * 10.0f) / 10.0f);
            return text.substr(0, text.find(L'.') + 2);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 図鑑が開いていれば、要素の内容を更新します。
    //----------------------------------------------------------------------------
    void ZukanSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>() ||
           !registry.HasContext<CollectionConfig>())
            return;
        // 閉じている間の 3D は MenuSystem が隠す
        MenuState& menu = registry.GetContext<MenuState>();
        if(!menu.IsOpen(MenuKind::Zukan))
            return;

        //--------------------------------------------------------------
        // タブのクリックと、選んでいるタブの中身だけを見せる
        //--------------------------------------------------------------
        registry.View<ZukanElementComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, ZukanElementComponent& element, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                if(element.kind == ZukanElementKind::Tab && pointer.clicked)
                    menu.zukanTab = element.tab;
            });
        registry.View<ZukanElementComponent, Tsukino::BuiltIn::ECS::UIVisibilityComponent>().each(
            [&](Tsukino::ECS::Entity, ZukanElementComponent& element, Tsukino::BuiltIn::ECS::UIVisibilityComponent& visibility) {
                if(element.kind == ZukanElementKind::TabGroup)
                    visibility.visible = (element.tab == menu.zukanTab);
            });
        const bool fruitsShown    = menu.zukanTab == ZukanTab::Fruits;
        const bool souvenirsShown = menu.zukanTab == ZukanTab::Souvenirs;

        const GameState&        state      = registry.GetContext<GameState>();
        const UiConfig&         ui         = GetUiConfig(registry);
        const Texts&            texts      = GetTexts(registry);
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
        const int               totalEntries = static_cast<int>(catalog.Fruits().size() * collection.Variants().size());
        const SouvenirCatalog*  souvenirs    = registry.HasContext<SouvenirCatalog>() ? &registry.GetContext<SouvenirCatalog>() : nullptr;
        auto souvenirOf = [&](int index) -> const SouvenirDef* {
            return (souvenirs && index >= 0 && index < static_cast<int>(souvenirs->Souvenirs().size())) ? &souvenirs->Souvenirs()[index] : nullptr;
        };

        registry.View<ZukanElementComponent>().each([&](Tsukino::ECS::Entity entity, ZukanElementComponent& element) {
            //--------------------------------------------------------------
            // タブの色（選んでいるタブ・カーソルが重なっているタブ。強化画面のタブと同じ色）
            //--------------------------------------------------------------
            if(element.kind == ZukanElementKind::Tab) {
                auto* sprite  = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                auto* pointer = registry.try_get<Tsukino::BuiltIn::ECS::PointerTargetComponent>(entity);
                if(sprite)
                    sprite->tintColor = (element.tab == menu.zukanTab)   ? ui.upgradeTabSelectedColor
                                        : (pointer && pointer->hovered) ? ui.upgradeTabHoverColor
                                                                        : ui.upgradeTabColor;
                return;
            }

            //--------------------------------------------------------------
            // 枠の果物（3D）: 登録済みなら色付き、未登録なら黒いシルエットで、どちらもゆっくり回す。
            // 登録したかどうかが変わったときだけ作り直す（SetFruitIcon は同じ中身なら何もしない）。
            // 3D はタブの中身の親で隠れないので、果物のタブを選んでいる間だけ見せる
            //--------------------------------------------------------------
            if(element.kind == ZukanElementKind::Fruit) {
                const bool registered = CountOf(state, element.fruitIndex, element.variantIndex) > 0;
                SetFruitIcon(registry, entity, element.fruitIndex, element.variantIndex, !registered, ui.zukanFruitSize);
                SetFruitIconVisible(registry, entity, fruitsShown);
                if(auto* icon = registry.try_get<FruitIconComponent>(entity))
                    icon->spinSpeed = ui.zukanFruitSpinSpeed;
                return;
            }

            // おみやげの 3D: もらっていれば色付き、まだなら黒いシルエット。おみやげのタブを選んでいる間だけ見せる
            if(element.kind == ZukanElementKind::SouvenirIcon) {
                if(const SouvenirDef* def = souvenirOf(element.fruitIndex)) {
                    SetModelIcon(registry, entity, def->model, def->modelRotation, state.SouvenirLevelOf(def->id) <= 0, ui.souvenirIconSize);
                    SetFruitIconVisible(registry, entity, souvenirsShown);
                    if(auto* icon = registry.try_get<FruitIconComponent>(entity))
                        icon->spinSpeed = ui.zukanFruitSpinSpeed;
                }
                return;
            }

            //--------------------------------------------------------------
            // 文字
            //--------------------------------------------------------------
            auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
            if(!font)
                return;

            switch(element.kind) {
                case ZukanElementKind::RowName: {
                    if(element.fruitIndex < 0 || element.fruitIndex >= static_cast<int>(catalog.Fruits().size()))
                        break;
                    // ランク（小・中…）は出さない。行はランクの順に並ぶので、下ほど珍しいと分かる
                    const FruitDef& def = catalog.Fruits()[element.fruitIndex];
                    font->text          = texts.Format("zukan.rowName", {{"name", AnyRegistered(state, element.fruitIndex) ? def.name : texts.Get("zukan.unknown")}});
                    break;
                }

                case ZukanElementKind::Count: {
                    const int n = CountOf(state, element.fruitIndex, element.variantIndex);
                    font->text  = (n > 0) ? texts.Format("zukan.count", {{"n", std::to_wstring(n)}}) : texts.Get("zukan.unknown");
                    break;
                }

                case ZukanElementKind::SouvenirName:
                case ZukanElementKind::SouvenirLevel:
                case ZukanElementKind::SouvenirEffect: {
                    const SouvenirDef* def = souvenirOf(element.fruitIndex);
                    if(!def)
                        break;
                    const int level = state.SouvenirLevelOf(def->id);
                    if(element.kind == ZukanElementKind::SouvenirName) {
                        font->text = (level > 0) ? def->name : texts.Get("zukan.unknown");
                        if(!ui.souvenirRarityColors.empty())
                            font->color = ui.souvenirRarityColors[std::min(static_cast<size_t>(def->rarity), ui.souvenirRarityColors.size() - 1)];
                    } else if(element.kind == ZukanElementKind::SouvenirLevel) {
                        font->text = (level > 0) ? texts.Format("souvenir.level", {{"level", std::to_wstring(level)}, {"max", std::to_wstring(def->maxLevel)}})
                                                 : texts.Get("souvenir.notYet");
                    } else {
                        // まだのものも、何に効くかだけは見せる（1レベルの量）
                        const float amount = def->perLevel * static_cast<float>(std::max(1, level));
                        font->text         = texts.Format(EffectKey(def->effect), {{"n", Amount(amount)}});
                    }
                    break;
                }

                case ZukanElementKind::SouvenirFooter: {
                    const size_t total = souvenirs ? souvenirs->Souvenirs().size() : 0;
                    font->text         = texts.Format("souvenir.footer", {{"n", std::to_wstring(state.souvenirLevels.size())}, {"total", std::to_wstring(total)}});
                    break;
                }

                case ZukanElementKind::Footer: {
                    const int registered = state.RegisteredCount();
                    const int bonus      = static_cast<int>(std::lround(collection.ManaBonusPerEntry() * 100.0f * static_cast<float>(registered)));
                    font->text           = texts.Format("zukan.footer", {{"registered", std::to_wstring(registered)}, {"total", std::to_wstring(totalEntries)}, {"bonus", std::to_wstring(bonus)}});
                    break;
                }

                default:
                    break;
            }
        });
    }
}    // namespace FruitMagic::ECS

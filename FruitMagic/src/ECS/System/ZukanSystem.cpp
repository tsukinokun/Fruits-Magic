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
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
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
    }    // namespace

    //----------------------------------------------------------------------------
    //! 図鑑が開いていれば、要素の内容を更新します。
    //----------------------------------------------------------------------------
    void ZukanSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<MenuState>() || !registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>() ||
           !registry.HasContext<CollectionConfig>())
            return;
        //--------------------------------------------------------------
        // 閉じている間は、枠の果物（3D）を隠すだけ（画面スプライトと違い、MenuSystem はモデルを隠さない）
        //--------------------------------------------------------------
        if(!registry.GetContext<MenuState>().IsOpen(MenuKind::Zukan)) {
            registry.View<ZukanElementComponent>().each([&](Tsukino::ECS::Entity entity, ZukanElementComponent& element) {
                if(element.kind == ZukanElementKind::Fruit)
                    SetFruitIconVisible(registry, entity, false);
            });
            return;
        }

        const GameState&        state      = registry.GetContext<GameState>();
        const UiConfig&         ui         = GetUiConfig(registry);
        const Texts&            texts      = GetTexts(registry);
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
        const int               totalEntries = static_cast<int>(catalog.Fruits().size() * collection.Variants().size());

        registry.View<ZukanElementComponent>().each([&](Tsukino::ECS::Entity entity, ZukanElementComponent& element) {
            //--------------------------------------------------------------
            // 枠の果物（3D）: 登録済みなら色付き、未登録なら黒いシルエットで、どちらもゆっくり回す。
            // 登録したかどうかが変わったときだけ作り直す（SetFruitIcon は同じ中身なら何もしない）
            //--------------------------------------------------------------
            if(element.kind == ZukanElementKind::Fruit) {
                const bool registered = CountOf(state, element.fruitIndex, element.variantIndex) > 0;
                SetFruitIcon(registry, entity, element.fruitIndex, element.variantIndex, !registered, ui.zukanFruitSize);
                SetFruitIconVisible(registry, entity, true);
                if(auto* icon = registry.try_get<FruitIconComponent>(entity))
                    icon->spinSpeed = ui.zukanFruitSpinSpeed;
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

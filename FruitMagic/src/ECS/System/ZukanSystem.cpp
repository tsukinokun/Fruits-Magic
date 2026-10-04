//----------------------------------------------------------------------------
//! @file   ZukanSystem.cpp
//! @brief  図鑑画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ZukanSystem.hpp>

#include <FruitMagic/ECS/Component/ZukanElementComponent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MenuState.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cmath>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 未登録の色見本の色
        const hlslpp::float4 kUnregisteredColor = hlslpp::float4(0.22f, 0.2f, 0.26f, 1.0f);

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
        if(!registry.GetContext<MenuState>().IsOpen(MenuKind::Zukan))
            return;

        const GameState&        state      = registry.GetContext<GameState>();
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
        const int               totalEntries = static_cast<int>(catalog.Fruits().size() * collection.Variants().size());

        registry.View<ZukanElementComponent>().each([&](Tsukino::ECS::Entity entity, ZukanElementComponent& element) {
            //--------------------------------------------------------------
            // 色見本: 登録済みならその枠の色
            //--------------------------------------------------------------
            if(auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                if(element.kind != ZukanElementKind::Swatch)
                    return;
                const bool registered = CountOf(state, element.fruitIndex, element.variantIndex) > 0;
                if(registered && element.fruitIndex < static_cast<int>(catalog.Fruits().size()) &&
                   element.variantIndex < static_cast<int>(collection.Variants().size())) {
                    const hlslpp::float3 c = collection.Variants()[element.variantIndex].ColorOf(catalog.Fruits()[element.fruitIndex]);
                    sprite->tintColor      = hlslpp::float4(c.x, c.y, c.z, 1.0f);
                } else {
                    sprite->tintColor = kUnregisteredColor;
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
                    const FruitDef& def  = catalog.Fruits()[element.fruitIndex];
                    const auto&     rank = catalog.Ranks()[def.rankIndex];
                    font->text           = L"【" + rank.name + L"】" + (AnyRegistered(state, element.fruitIndex) ? def.name : std::wstring(L"？？？"));
                    break;
                }

                case ZukanElementKind::Count: {
                    const int n = CountOf(state, element.fruitIndex, element.variantIndex);
                    font->text  = (n > 0) ? L"×" + std::to_wstring(n) : std::wstring(L"？？？");
                    break;
                }

                case ZukanElementKind::Footer: {
                    const int registered = state.RegisteredCount();
                    const int bonus      = static_cast<int>(std::lround(collection.ManaBonusPerEntry() * 100.0f * static_cast<float>(registered)));
                    font->text           = L"登録 " + std::to_wstring(registered) + L" / " + std::to_wstring(totalEntries) + L"     マナ獲得 +" +
                                 std::to_wstring(bonus) + L"%";
                    break;
                }

                default:
                    break;
            }
        });
    }
}    // namespace FruitMagic::ECS

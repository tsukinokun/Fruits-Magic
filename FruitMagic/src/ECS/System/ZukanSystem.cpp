//----------------------------------------------------------------------------
//! @file   ZukanSystem.cpp
//! @brief  図鑑画面のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/ZukanSystem.hpp>

#include <FruitMagic/ECS/Component/ZukanElementComponent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/ZukanState.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>

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
    //! 開閉の入力を読み、図鑑の要素を更新します。
    //----------------------------------------------------------------------------
    void ZukanSystem::Update(Tsukino::ECS::Registry& registry, float /*deltaTime*/) {
        if(!registry.HasContext<ZukanState>() || !registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>() ||
           !registry.HasContext<CollectionConfig>())
            return;

        ZukanState&             zukan      = registry.GetContext<ZukanState>();
        const GameState&        state      = registry.GetContext<GameState>();
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();

        //--------------------------------------------------------------
        // 開閉（Tab キー、または開閉ボタンのクリック）
        //--------------------------------------------------------------
        bool toggle = false;
        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(ctx && ctx->inputSystem && ctx->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::Tab))
            toggle = true;

        registry.View<ZukanButtonComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, ZukanButtonComponent& button, Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                if(pointer.clicked)
                    toggle = true;
                if(button.label != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label))
                    registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label).text = zukan.open ? L"閉じる (Tab)" : L"図鑑 (Tab)";
            });
        if(toggle)
            zukan.open = !zukan.open;

        //--------------------------------------------------------------
        // 図鑑の要素の表示切替と内容の更新
        //--------------------------------------------------------------
        const int totalEntries = static_cast<int>(catalog.Fruits().size() * collection.Variants().size());

        registry.View<ZukanElementComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, ZukanElementComponent& element, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                //--------------------------------------------------------------
                // スプライト: 閉じている間はスケール 0（描画も当たり判定もされない）
                //--------------------------------------------------------------
                if(auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity)) {
                    transform.scale = zukan.open ? element.openScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                    transform.dirty = true;

                    if(element.kind == ZukanElementKind::Swatch && zukan.open) {
                        const bool registered = CountOf(state, element.fruitIndex, element.variantIndex) > 0;
                        if(registered && element.fruitIndex < static_cast<int>(catalog.Fruits().size()) &&
                           element.variantIndex < static_cast<int>(collection.Variants().size())) {
                            const hlslpp::float3 c = collection.Variants()[element.variantIndex].ColorOf(catalog.Fruits()[element.fruitIndex]);
                            sprite->tintColor      = hlslpp::float4(c.x, c.y, c.z, 1.0f);
                        } else {
                            sprite->tintColor = kUnregisteredColor;
                        }
                    }
                }

                //--------------------------------------------------------------
                // 文字: 閉じている間は空文字（描画されない）
                //--------------------------------------------------------------
                auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity);
                if(!font)
                    return;
                if(!zukan.open) {
                    font->text.clear();
                    return;
                }

                switch(element.kind) {
                    case ZukanElementKind::StaticText:
                        font->text = element.text;
                        break;

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

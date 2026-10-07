//----------------------------------------------------------------------------
//! @file   FruitIcon.cpp
//! @brief  画面に出す果物（UI の層に描く 3D の果物）を作る・入れ替える関数の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/FruitIcon.hpp>

#include <FruitMagic/ECS/Component/FruitIconComponent.hpp>
#include <FruitMagic/ECS/Component/FruitPartsComponent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>

#include <Tsukino/BuiltIn/ECS/Component/MaterialPropertyBlockComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScreenModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //--------------------------------------------------------------
        // 置き台を置くワールドの位置。UI の層に描くモデルはワールドに描かれないので、台から離しておくだけでよい
        //--------------------------------------------------------------
        const hlslpp::float3 kHolderPosition(0.0f, -1000.0f, 0.0f);

        //--------------------------------------------------------------
        //! 果物の見た目の、モデルを持つエンティティ（本体と、作り込んだモデルの子・飾りのパーツ）すべてに処理をします。
        //! @param  [in] registry レジストリ
        //! @param  [in] visual   果物の見た目
        //! @param  [in] fn       エンティティごとに呼ぶ処理
        //--------------------------------------------------------------
        template <class Fn>
        void ForEachModel(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity visual, Fn&& fn) {
            if(visual == entt::null || !registry.IsValid(visual))
                return;
            fn(visual);
            if(const auto* parts = registry.try_get<ECS::FruitPartsComponent>(visual)) {
                for(Tsukino::ECS::Entity part : parts->parts) {
                    if(registry.IsValid(part))
                        fn(part);
                }
            }
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 画面に出す果物の置き台を作ります。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity CreateFruitIconHolder(Tsukino::ECS::Registry& registry, const hlslpp::float2& screenPosition, int sortOrder, Tsukino::ECS::Entity anchor) {
        Tsukino::ECS::Entity                       holder    = registry.CreateEntity();
        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(holder);
        transform.position                                   = kHolderPosition;
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ScreenModelComponent& screen = registry.AddComponent<Tsukino::BuiltIn::ECS::ScreenModelComponent>(holder);
        screen.screenPosition                               = screenPosition;
        screen.sortOrder                                    = sortOrder;
        screen.anchor                                       = anchor;

        // 回さない置き台も、上から少し見下ろす向きにしておく（回す置き台は FruitIconSystem が向きを決める）
        const ECS::FruitIconComponent& icon = registry.AddComponent<ECS::FruitIconComponent>(holder);
        transform.rotation                  = PrizeFactory::EulerDegrees(hlslpp::float3(icon.tilt, 0.0f, 0.0f));
        return holder;
    }

    //----------------------------------------------------------------------------
    //! 置き台に出す果物を設定します。
    //----------------------------------------------------------------------------
    void SetFruitIcon(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity holder, int fruitIndex, int variantIndex, bool silhouette, float sizePixels) {
        auto* icon = registry.try_get<ECS::FruitIconComponent>(holder);
        if(!icon)
            return;
        if(icon->fruitIndex == fruitIndex && icon->variantIndex == variantIndex && icon->silhouette == silhouette && icon->sizePixels == sizePixels
           && (fruitIndex < 0 || icon->visual != entt::null))
            return;

        // 今の見た目を消す（飾りのパーツも一緒に）
        if(icon->visual != entt::null && registry.IsValid(icon->visual))
            PrizeFactory::DestroyPrize(registry, icon->visual);
        icon->visual       = entt::null;
        icon->fruitIndex   = fruitIndex;
        icon->variantIndex = variantIndex;
        icon->silhouette   = silhouette;
        icon->sizePixels   = sizePixels;

        if(fruitIndex < 0 || !registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>() || !registry.HasContext<PrizeFactory>())
            return;
        const auto& fruits   = registry.GetContext<FruitCatalog>().Fruits();
        const auto& variants = registry.GetContext<CollectionConfig>().Variants();
        if(fruitIndex >= static_cast<int>(fruits.size()) || variantIndex < 0 || variantIndex >= static_cast<int>(variants.size()))
            return;

        //--------------------------------------------------------------
        // 台の上の果物と同じ見た目を作り、置き台の子にする（置き台の ScreenModelComponent で UI の層に描かれる）
        //--------------------------------------------------------------
        const FruitDef&      def     = fruits[fruitIndex];
        const VariantDef&    variant = variants[variantIndex];
        Tsukino::ECS::Entity visual  = registry.GetContext<PrizeFactory>().CreateFruitVisual(registry, def, variant.ColorOf(def), variant.glow, hlslpp::float3(0.0f, 0.0f, 0.0f));
        auto&                transform = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(visual);
        transform.parent                = holder;
        transform.dirty                 = true;
        icon->visual                    = visual;

        // 大きさ: 果物の外形のいちばん長い向きを sizePixels に合わせる
        const hlslpp::float3 half    = PrizeFactory::FruitHalfExtent(def);
        const float          longest = std::max({float(half.x), float(half.y), float(half.z)}) * 2.0f;
        if(auto* screen = registry.try_get<Tsukino::BuiltIn::ECS::ScreenModelComponent>(holder))
            screen->pixelsPerUnit = sizePixels / std::max(longest, 0.01f);

        //--------------------------------------------------------------
        // シルエット: 形だけが分かるよう、すべての部品を黒くし、輪郭の光を消す
        //--------------------------------------------------------------
        const bool visible = icon->visible;
        ForEachModel(registry, visual, [&](Tsukino::ECS::Entity e) {
            if(silhouette) {
                if(registry.HasComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e)) {
                    auto& block     = registry.HasComponent<Tsukino::BuiltIn::ECS::MaterialPropertyBlockComponent>(e)
                                          ? registry.GetComponent<Tsukino::BuiltIn::ECS::MaterialPropertyBlockComponent>(e)
                                          : registry.AddComponent<Tsukino::BuiltIn::ECS::MaterialPropertyBlockComponent>(e);
                    block.baseColor = hlslpp::float4(0.03f, 0.02f, 0.05f, 1.0f);
                    block.emissive  = hlslpp::float3(0.0f, 0.0f, 0.0f);
                    block.metallic  = 0.0f;
                    block.roughness = 1.0f;
                }
                if(auto* rim = registry.try_get<Tsukino::BuiltIn::ECS::RimGlowComponent>(e))
                    rim->active = false;
            }
            if(auto* model = registry.try_get<Tsukino::BuiltIn::ECS::ModelComponent>(e))
                model->visible = visible;
        });
    }

    //----------------------------------------------------------------------------
    //! 置き台の果物を見せるか隠すかを切り替えます。
    //----------------------------------------------------------------------------
    void SetFruitIconVisible(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity holder, bool visible) {
        auto* icon = registry.try_get<ECS::FruitIconComponent>(holder);
        if(!icon || icon->visible == visible)
            return;
        icon->visible = visible;
        ForEachModel(registry, icon->visual, [&](Tsukino::ECS::Entity e) {
            if(auto* model = registry.try_get<Tsukino::BuiltIn::ECS::ModelComponent>(e))
                model->visible = visible;
        });
    }
}    // namespace FruitMagic

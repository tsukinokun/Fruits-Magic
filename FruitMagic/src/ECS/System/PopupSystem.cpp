//----------------------------------------------------------------------------
//! @file   PopupSystem.cpp
//! @brief  落ちたときのポップのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/PopupSystem.hpp>

#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/EffectsConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiFonts.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/WorldAnchorComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <string>
#include <utility>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    PopupSystem::PopupSystem(Tsukino::ECS::EventBus& eventBus) {
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.zone == DropZone::Payout)
                m_pending.push_back(Drop{e.kind == PrizeKind::Fruit, e.fruitIndex, e.variantIndex, e.x});
        });
    }

    //----------------------------------------------------------------------------
    //! ポップを出して動かします。
    //----------------------------------------------------------------------------
    void PopupSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        // ポップは台の手前の縁の上に出す
        const float      popupZ = GetTableLayout(registry).fieldFrontZ;
        const PopupStyle style  = registry.HasContext<EffectsConfig>() ? registry.GetContext<EffectsConfig>().popup : PopupStyle{};

        //--------------------------------------------------------------
        // ポップを1つ作る（ワールドの一点に追従する画面の文字）
        //--------------------------------------------------------------
        auto spawn = [&](const std::wstring& text, float x, const hlslpp::float4& color, float scale, float life) {
            Tsukino::ECS::Entity                       e = registry.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.scale                                      = hlslpp::float3(scale, scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.fontHandle                            = GetUiFont(registry, true);    // ポップは太字
            font.text                                  = text;
            font.color                                 = color;
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = hlslpp::float4(style.outlineColor, 1.0f);
            font.outlineWidth                          = style.outlineWidth;
            font.sortOrder                             = 15;    // 魔法ボタン（10・11）より手前

            Tsukino::BuiltIn::ECS::WorldAnchorComponent& anchor = registry.AddComponent<Tsukino::BuiltIn::ECS::WorldAnchorComponent>(e);
            anchor.useFixedWorldPosition                         = true;
            anchor.fixedWorldPosition                            = hlslpp::float3(x, style.y, popupZ);

            PopupComponent& popup = registry.AddComponent<PopupComponent>(e);
            popup.color           = color;
            popup.life            = life;
            popup.maxLife         = life;
            popup.rise            = style.rise;
        };

        //--------------------------------------------------------------
        // 落ちた物をポップにする。果物はすぐ、コインは少しまとめてから
        //--------------------------------------------------------------
        for(const Drop& drop : m_pending) {
            if(!drop.fruit) {
                if(m_coinCount == 0)
                    m_coinTimer = style.coinGatherSeconds;
                m_coinCount += 1;
                m_coinXSum += drop.x;
                continue;
            }
            if(!registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>())
                continue;
            const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
            if(drop.fruitIndex < 0 || drop.fruitIndex >= static_cast<int>(fruits.size()))
                continue;

            // 名前はバリエーション付き、色はそのバリエーションの色（読みやすいよう明るめに）
            const CollectionConfig& collection = registry.GetContext<CollectionConfig>();
            const int               v          = std::clamp(drop.variantIndex, 0, static_cast<int>(collection.Variants().size()) - 1);
            const hlslpp::float3    c          = collection.Variants()[v].ColorOf(fruits[drop.fruitIndex]);
            const hlslpp::float3    light      = c + (hlslpp::float3(1.0f, 1.0f, 1.0f) - c) * style.fruitLighten;
            spawn(GetTexts(registry).Format("popup.fruit", {{"name", collection.DisplayName(fruits[drop.fruitIndex], drop.variantIndex)}}), drop.x, hlslpp::float4(light, 1.0f), (v > 0) ? style.variantScale : style.fruitScale, style.fruitLife);
        }
        m_pending.clear();

        if(m_coinCount > 0) {
            m_coinTimer -= deltaTime;
            if(m_coinTimer <= 0.0f) {
                const float scale = 1.0f + std::min(style.coinScaleMaxBonus, style.coinScaleStep * static_cast<float>(m_coinCount));    // たくさん落ちたほど大きく
                spawn(GetTexts(registry).Format("popup.coins", {{"n", std::to_wstring(m_coinCount)}}), m_coinXSum / static_cast<float>(m_coinCount), style.coinColor, scale, style.coinLife);
                m_coinCount = 0;
                m_coinXSum  = 0.0f;
            }
        }

        //--------------------------------------------------------------
        // 浮かせて薄くし、寿命で消す。多すぎるときは古いもの（残りの寿命が短いもの）から消す
        //--------------------------------------------------------------
        std::vector<std::pair<float, Tsukino::ECS::Entity>> alive;
        std::vector<Tsukino::ECS::Entity>                   dead;
        registry.View<PopupComponent, Tsukino::BuiltIn::ECS::FontComponent, Tsukino::BuiltIn::ECS::WorldAnchorComponent>().each(
            [&](Tsukino::ECS::Entity entity, PopupComponent& popup, Tsukino::BuiltIn::ECS::FontComponent& font,
                Tsukino::BuiltIn::ECS::WorldAnchorComponent& anchor) {
                popup.life -= deltaTime;
                if(popup.life <= 0.0f) {
                    dead.push_back(entity);
                    return;
                }
                const float k      = popup.life / popup.maxLife;    // 1 → 0
                const float alpha  = std::min(1.0f, k * style.fadeStart);    // 既定（2.5）なら最後の4割で消えていく
                anchor.worldOffset = hlslpp::float3(0.0f, popup.rise * (1.0f - k), 0.0f);
                font.color         = hlslpp::float4(popup.color.xyz, alpha);
                font.outlineColor  = hlslpp::float4(style.outlineColor, alpha);
                alive.emplace_back(popup.life, entity);
            });

        if(static_cast<int>(alive.size()) > style.maxPopups) {
            std::sort(alive.begin(), alive.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            for(size_t i = 0; i < alive.size() - static_cast<size_t>(style.maxPopups); ++i)
                dead.push_back(alive[i].second);
        }
        for(Tsukino::ECS::Entity entity : dead)
            registry.QueueDestroy(entity);
    }
}    // namespace FruitMagic::ECS

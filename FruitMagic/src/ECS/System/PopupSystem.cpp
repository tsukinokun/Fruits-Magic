//----------------------------------------------------------------------------
//! @file   PopupSystem.cpp
//! @brief  落ちたときのポップのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/PopupSystem.hpp>

#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>

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
    namespace {
        //! @brief コインをまとめる時間（秒）。この間に落ちた分を1つの「+N」にする
        constexpr float kCoinGatherSeconds = 0.25f;

        //! @brief ポップの出る位置（台の手前の縁の少し上。これより下は画面下の魔法ボタンに隠れる）
        constexpr float kPopupY = 6.0f;
        constexpr float kPopupZ = Layout::kFieldFrontZ;

        //! @brief 同時に出しておくポップの上限（多すぎると読めないので、超えたら古いものから消す）
        constexpr int kMaxPopups = 10;

        //! @brief コインのポップの色
        const hlslpp::float4 kCoinColor = hlslpp::float4(1.0f, 0.9f, 0.35f, 1.0f);

        //! @brief 文字の縁の色
        const hlslpp::float3 kOutlineColor = hlslpp::float3(0.2f, 0.08f, 0.12f);
    }    // namespace

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
        //--------------------------------------------------------------
        // ポップを1つ作る（ワールドの一点に追従する画面の文字）
        //--------------------------------------------------------------
        auto spawn = [&](const std::wstring& text, float x, const hlslpp::float4& color, float scale, float life) {
            Tsukino::ECS::Entity                       e = registry.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.scale                                      = hlslpp::float3(scale, scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.text                                  = text;
            font.color                                 = color;
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = hlslpp::float4(kOutlineColor, 1.0f);
            font.outlineWidth                          = 2.0f;
            font.sortOrder                             = 15;    // 魔法ボタン（10・11）より手前

            Tsukino::BuiltIn::ECS::WorldAnchorComponent& anchor = registry.AddComponent<Tsukino::BuiltIn::ECS::WorldAnchorComponent>(e);
            anchor.useFixedWorldPosition                         = true;
            anchor.fixedWorldPosition                            = hlslpp::float3(x, kPopupY, kPopupZ);

            PopupComponent& popup = registry.AddComponent<PopupComponent>(e);
            popup.color           = color;
            popup.life            = life;
            popup.maxLife         = life;
        };

        //--------------------------------------------------------------
        // 落ちた物をポップにする。果物はすぐ、コインは少しまとめてから
        //--------------------------------------------------------------
        for(const Drop& drop : m_pending) {
            if(!drop.fruit) {
                if(m_coinCount == 0)
                    m_coinTimer = kCoinGatherSeconds;
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
            const hlslpp::float3    light      = c + (hlslpp::float3(1.0f, 1.0f, 1.0f) - c) * 0.35f;
            spawn(collection.DisplayName(fruits[drop.fruitIndex], drop.variantIndex) + L"！", drop.x, hlslpp::float4(light, 1.0f), (v > 0) ? 1.1f : 0.95f, 1.6f);
        }
        m_pending.clear();

        if(m_coinCount > 0) {
            m_coinTimer -= deltaTime;
            if(m_coinTimer <= 0.0f) {
                const float scale = 1.0f + std::min(0.5f, 0.05f * static_cast<float>(m_coinCount));    // たくさん落ちたほど大きく
                spawn(L"+" + std::to_wstring(m_coinCount), m_coinXSum / static_cast<float>(m_coinCount), kCoinColor, scale, 1.0f);
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
                const float alpha  = std::min(1.0f, k * 2.5f);      // 最後の4割で消えていく
                anchor.worldOffset = hlslpp::float3(0.0f, popup.rise * (1.0f - k), 0.0f);
                font.color         = hlslpp::float4(popup.color.xyz, alpha);
                font.outlineColor  = hlslpp::float4(kOutlineColor, alpha);
                alive.emplace_back(popup.life, entity);
            });

        if(static_cast<int>(alive.size()) > kMaxPopups) {
            std::sort(alive.begin(), alive.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            for(size_t i = 0; i < alive.size() - kMaxPopups; ++i)
                dead.push_back(alive[i].second);
        }
        for(Tsukino::ECS::Entity entity : dead)
            registry.QueueDestroy(entity);
    }
}    // namespace FruitMagic::ECS

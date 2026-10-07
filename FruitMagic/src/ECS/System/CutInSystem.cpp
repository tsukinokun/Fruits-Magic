//----------------------------------------------------------------------------
//! @file   CutInSystem.cpp
//! @brief  果物が取れたときのカットイン（帯・果物の 3D モデル・名前）を出すシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/CutInSystem.hpp>

#include <FruitMagic/ECS/Component/CutInElementComponent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScreenModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //--------------------------------------------------------------
        //! 出だしが速く、終わりがゆっくりな動き（0〜1 → 0〜1）を返します。
        //! @param  [in] t 進み具合（0〜1）
        //! @return 動きの進み具合
        //--------------------------------------------------------------
        float EaseOut(float t) {
            const float u = 1.0f - std::clamp(t, 0.0f, 1.0f);
            return 1.0f - u * u * u;
        }

        //--------------------------------------------------------------
        //! 出だしがゆっくりで、終わりが速い動き（0〜1 → 0〜1）を返します。
        //! @param  [in] t 進み具合（0〜1）
        //! @return 動きの進み具合
        //--------------------------------------------------------------
        float EaseIn(float t) {
            const float u = std::clamp(t, 0.0f, 1.0f);
            return u * u * u;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    CutInSystem::CutInSystem(Tsukino::ECS::EventBus& eventBus) {
        // 色違い・金色が払い出し口に落ちたとき（左右の溝に落ちたものは取れていないので出さない）
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Fruit && e.zone == DropZone::Payout && e.fruitIndex >= 0 && e.variantIndex > 0)
                m_incoming.push_back(Request{e.fruitIndex, e.variantIndex, false});
        });

        // 図鑑に初めて載ったとき（ふつうの果物も含む）
        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent& e) {
            if(e.fruitIndex >= 0)
                m_incoming.push_back(Request{e.fruitIndex, e.variantIndex, true});
        });
    }

    //----------------------------------------------------------------------------
    //! 順番待ちを進め、カットインの部品を動かします。
    //----------------------------------------------------------------------------
    void CutInSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        const UiConfig& ui      = GetUiConfig(registry);
        const bool      enabled = !registry.HasContext<Settings>() || registry.GetContext<Settings>().showCutIn;

        //--------------------------------------------------------------
        // 届いた分を順番待ちへ。同じ果物の「色違いが落ちた」と「図鑑に登録」は1回にまとめ、見出しは図鑑を優先する
        //--------------------------------------------------------------
        std::vector<Request> merged;
        for(const Request& request : m_incoming) {
            auto it = std::find_if(merged.begin(), merged.end(), [&](const Request& r) {
                return r.fruitIndex == request.fruitIndex && r.variantIndex == request.variantIndex;
            });
            if(it == merged.end())
                merged.push_back(request);
            else
                it->registered = it->registered || request.registered;
        }
        m_incoming.clear();
        if(enabled) {
            for(const Request& request : merged) {
                if(static_cast<int>(m_queue.size()) < ui.cutInMaxQueue)
                    m_queue.push_back(request);
            }
        }

        //--------------------------------------------------------------
        // 進める。終わったら次の順番待ちを始める
        //--------------------------------------------------------------
        const float total = ui.cutInInSeconds + ui.cutInHoldSeconds + ui.cutInOutSeconds;
        if(m_playing) {
            m_time += deltaTime;
            m_spin = std::fmod(m_spin + ui.cutInSpinSpeed * deltaTime, 360.0f);
            if(m_time >= total)
                Finish(registry);
        }
        while(!m_playing && !m_queue.empty()) {
            const Request request = m_queue.front();
            m_queue.pop_front();
            Start(registry, request);
        }

        //--------------------------------------------------------------
        // 部品の動き。入るときは左から滑り込み、止まって見せ、右へ抜けて消える。
        // 果物はぽんと一瞬大きくなってから落ち着き、消えるときは縮む
        //--------------------------------------------------------------
        const float inRate  = m_playing ? std::clamp(m_time / ui.cutInInSeconds, 0.0f, 1.0f) : 0.0f;
        const float outRate = m_playing ? std::clamp((m_time - ui.cutInInSeconds - ui.cutInHoldSeconds) / ui.cutInOutSeconds, 0.0f, 1.0f) : 1.0f;
        const float slideX  = (EaseOut(inRate) - 1.0f + EaseIn(outRate)) * ui.screenWidth;    // 帯をずらす量（左の画面外 → 0 → 右の画面外）
        const float opacity = std::min(inRate, 1.0f - outRate);                               // 文字の濃さ

        constexpr float kPopPeak = 0.6f;    // 入る時間のうち、一番大きくなるまでの割合
        float           pop      = (inRate < kPopPeak) ? ui.cutInPopScale * EaseOut(inRate / kPopPeak)
                                                       : ui.cutInPopScale + (1.0f - ui.cutInPopScale) * EaseOut((inRate - kPopPeak) / (1.0f - kPopPeak));
        pop *= 1.0f - EaseIn(outRate);

        registry.View<CutInElementComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, CutInElementComponent& element, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                switch(element.part) {
                    case CutInPart::Band:
                    case CutInPart::Edge:
                        transform.position = hlslpp::float3(float(element.basePosition.x) + slideX, float(element.basePosition.y), 0.0f);
                        transform.scale    = m_playing ? element.shownScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                        break;

                    case CutInPart::Title:
                    case CutInPart::Name:
                        if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity)) {
                            if(!m_playing)
                                font->text.clear();
                            font->color = hlslpp::float4(hlslpp::float3(font->color.xyz), opacity);
                        }
                        break;

                    case CutInPart::Fruit:
                        transform.scale    = hlslpp::float3(pop, pop, pop);
                        transform.rotation = PrizeFactory::EulerDegrees(hlslpp::float3(ui.cutInFruitTilt, m_spin, 0.0f));
                        break;
                }
                transform.dirty = true;
            });
    }

    //----------------------------------------------------------------------------
    //! カットインを1つ始めます。
    //----------------------------------------------------------------------------
    bool CutInSystem::Start(Tsukino::ECS::Registry& registry, const Request& request) {
        if(!registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>() || !registry.HasContext<PrizeFactory>())
            return false;
        const auto& fruits   = registry.GetContext<FruitCatalog>().Fruits();
        const auto& variants = registry.GetContext<CollectionConfig>().Variants();
        if(request.fruitIndex < 0 || request.fruitIndex >= static_cast<int>(fruits.size()) || request.variantIndex < 0
           || request.variantIndex >= static_cast<int>(variants.size()))
            return false;

        const UiConfig&   ui      = GetUiConfig(registry);
        const Texts&      texts   = GetTexts(registry);
        const FruitDef&   def     = fruits[request.fruitIndex];
        const VariantDef& variant = variants[request.variantIndex];
        const hlslpp::float3 color = variant.ColorOf(def);

        //--------------------------------------------------------------
        // 果物の見た目を置き台の子にする（置き台の ScreenModelComponent で UI の層に描かれる）。
        // 大きさは、果物の外形のいちばん長い向きが cutInFruitSize ピクセルになるようにする
        //--------------------------------------------------------------
        Tsukino::ECS::Entity holder = entt::null;
        registry.View<CutInElementComponent>().each([&](Tsukino::ECS::Entity entity, CutInElementComponent& element) {
            if(element.part == CutInPart::Fruit)
                holder = entity;
        });
        if(holder != entt::null) {
            m_fruit = registry.GetContext<PrizeFactory>().CreateFruitVisual(registry, def, color, variant.glow, hlslpp::float3(0.0f, 0.0f, 0.0f));
            auto& transform  = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_fruit);
            transform.parent = holder;
            transform.dirty  = true;

            const hlslpp::float3 half    = PrizeFactory::FruitHalfExtent(def);
            const float          longest = std::max({float(half.x), float(half.y), float(half.z)}) * 2.0f;
            if(auto* screen = registry.try_get<Tsukino::BuiltIn::ECS::ScreenModelComponent>(holder))
                screen->pixelsPerUnit = ui.cutInFruitSize / std::max(longest, 0.01f);
        }

        //--------------------------------------------------------------
        // 文字と、帯の縁の色（果物の色）
        //--------------------------------------------------------------
        const std::wstring title = request.registered ? texts.Get("cutin.registered") : texts.Format("cutin.variant", {{"variant", variant.name}});
        const std::wstring name  = registry.GetContext<CollectionConfig>().DisplayName(def, request.variantIndex);
        registry.View<CutInElementComponent>().each([&](Tsukino::ECS::Entity entity, CutInElementComponent& element) {
            if(element.part == CutInPart::Edge) {
                if(auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity))
                    sprite->tintColor = hlslpp::float4(color, 1.0f);
            } else if(element.part == CutInPart::Title || element.part == CutInPart::Name) {
                if(auto* font = registry.try_get<Tsukino::BuiltIn::ECS::FontComponent>(entity))
                    font->text = (element.part == CutInPart::Title) ? title : name;
            }
        });

        m_playing = true;
        m_time    = 0.0f;
        m_spin    = 0.0f;
        return true;
    }

    //----------------------------------------------------------------------------
    //! 今のカットインを終えます。
    //----------------------------------------------------------------------------
    void CutInSystem::Finish(Tsukino::ECS::Registry& registry) {
        if(m_fruit != entt::null && registry.IsValid(m_fruit))
            PrizeFactory::DestroyPrize(registry, m_fruit);
        m_fruit   = entt::null;
        m_playing = false;
    }
}    // namespace FruitMagic::ECS

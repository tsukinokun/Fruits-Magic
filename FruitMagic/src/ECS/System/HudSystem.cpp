//----------------------------------------------------------------------------
//! @file   HudSystem.cpp
//! @brief  HUD（手持ち枚数・払い出し・収穫・ルーレット）を更新するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HudSystem.hpp>

#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Component/ManaGaugeComponent.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/RouletteState.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <numeric>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 払い出し表示を出しておく時間（秒）。続けて落ちたら延長して合算する
        constexpr float kPopupDuration = 1.2f;

        //! @brief 収穫表示を出しておく時間（秒）
        constexpr float kHarvestPopupDuration = 2.0f;

        //--------------------------------------------------------------
        //! 果物の表示名を返します。
        //! @param  [in] registry   レジストリ（FruitCatalog を参照する）
        //! @param  [in] fruitIndex 果物の添字
        //! @return 表示名。範囲外なら空文字列
        //--------------------------------------------------------------
        std::wstring FruitName(Tsukino::ECS::Registry& registry, int fruitIndex) {
            if(!registry.HasContext<FruitCatalog>())
                return std::wstring();
            const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
            return (fruitIndex >= 0 && fruitIndex < static_cast<int>(fruits.size())) ? fruits[fruitIndex].name : std::wstring();
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    HudSystem::HudSystem(Tsukino::ECS::EventBus& eventBus) {
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Fruit) {
                // 果物は払い出し口に落ちたときだけ「ゲット」を出す
                if(e.zone == DropZone::Payout) {
                    m_harvestFruit = e.fruitIndex;
                    m_harvestTimer = kHarvestPopupDuration;
                }
                return;
            }

            if(e.zone == DropZone::Payout) {
                m_recentPayout += e.value;
            } else {
                m_recentGutter += 1;
            }
            m_popupTimer = kPopupDuration;
        });
    }

    //----------------------------------------------------------------------------
    //! HUD のテキストを更新します。
    //----------------------------------------------------------------------------
    void HudSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        //--------------------------------------------------------------
        // 一時表示の残り時間。切れたら内容をリセット
        //--------------------------------------------------------------
        if(m_popupTimer > 0.0f) {
            m_popupTimer -= deltaTime;
            if(m_popupTimer <= 0.0f) {
                m_recentPayout = 0;
                m_recentGutter = 0;
            }
        }
        if(m_harvestTimer > 0.0f) {
            m_harvestTimer -= deltaTime;
            if(m_harvestTimer <= 0.0f)
                m_harvestFruit = -1;
        }

        const GameState     state    = registry.HasContext<GameState>() ? registry.GetContext<GameState>() : GameState{};
        const RouletteState roulette = registry.HasContext<RouletteState>() ? registry.GetContext<RouletteState>() : RouletteState{};
        const int           harvestTotal = std::accumulate(state.harvestCounts.begin(), state.harvestCounts.end(), 0);

        //--------------------------------------------------------------
        // 各テキストの更新
        //--------------------------------------------------------------
        auto view = registry.View<HudTextComponent, Tsukino::BuiltIn::ECS::FontComponent>();
        view.each([&](Tsukino::ECS::Entity, HudTextComponent& hud, Tsukino::BuiltIn::ECS::FontComponent& font) {
            switch(hud.kind) {
                case HudTextKind::Coins:
                    font.text = L"コイン: " + std::to_wstring(state.coins);
                    break;

                case HudTextKind::Mana:
                    font.text = L"マナ " + std::to_wstring(state.mana) + L" / " + std::to_wstring(state.maxMana);
                    break;

                case HudTextKind::DropPopup: {
                    std::wstring text;
                    if(m_recentPayout > 0)
                        text += L"+" + std::to_wstring(m_recentPayout) + L"  ";
                    if(m_recentGutter > 0)
                        text += L"溝 " + std::to_wstring(m_recentGutter);
                    font.text = text;
                    break;
                }

                case HudTextKind::HarvestTotal:
                    font.text = L"収穫: " + std::to_wstring(harvestTotal);
                    break;

                case HudTextKind::HarvestPopup:
                    font.text = (m_harvestFruit >= 0) ? FruitName(registry, m_harvestFruit) + L" ゲット！" : std::wstring();
                    break;

                case HudTextKind::Roulette: {
                    std::wstring text;
                    switch(roulette.phase) {
                        case RoulettePhase::Spinning:
                            text = L"ルーレット ▶ " + ((roulette.displayFruit >= 0) ? FruitName(registry, roulette.displayFruit) : std::wstring(L"ハズレ"));
                            break;
                        case RoulettePhase::Result:
                            text = roulette.resultHit ? L"当たり！ " + FruitName(registry, roulette.displayFruit) + L" が出た！" : std::wstring(L"ハズレ…");
                            break;
                        case RoulettePhase::Idle:
                        default:
                            break;
                    }
                    if(roulette.stock > 0)
                        text += L"   (のこり " + std::to_wstring(roulette.stock) + L")";
                    font.text = text;
                    break;
                }

                case HudTextKind::ControlsHint:
#ifdef _DEBUG
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入   F5: コリジョン表示   F2: 果樹Lv " + std::to_wstring(state.treeLevel);
#else
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入";
#endif
                    break;
            }
        });

        UpdateManaGauge(registry, state);
        UpdateMagicButtons(registry, state);
    }

    //----------------------------------------------------------------------------
    //! マナゲージの中身のバーの幅を、今のマナに合わせます。
    //----------------------------------------------------------------------------
    void HudSystem::UpdateManaGauge(Tsukino::ECS::Registry& registry, const GameState& state) {
        const float ratio = (state.maxMana > 0) ? std::clamp(static_cast<float>(state.mana) / static_cast<float>(state.maxMana), 0.0f, 1.0f) : 0.0f;

        registry.View<ManaGaugeComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity, ManaGaugeComponent& gauge, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                // 画面スプライトの位置は中心なので、左端を固定するには幅の半分だけずらす
                const float width  = gauge.fullWidth * ratio;
                transform.position = hlslpp::float3(gauge.left + width * 0.5f, float(transform.position.y), 0.0f);
                transform.scale    = hlslpp::float3(width / gauge.textureSize, gauge.height / gauge.textureSize, 1.0f);
                transform.dirty    = true;
            });
    }

    //----------------------------------------------------------------------------
    //! 魔法ボタンの色と文字を、解放状態・マナ・効果中かに合わせます。
    //----------------------------------------------------------------------------
    void HudSystem::UpdateMagicButtons(Tsukino::ECS::Registry& registry, const GameState& state) {
        if(!registry.HasContext<MagicCatalog>())
            return;

        const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();
        const bool          busy    = registry.HasContext<MagicState>() && registry.GetContext<MagicState>().activeMagic >= 0;

        registry.View<MagicButtonComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, MagicButtonComponent& button, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                const int       index = catalog.FindBySlot(button.slot);
                const MagicDef* def   = (index >= 0) ? &catalog.Magics()[index] : nullptr;

                std::wstring   text;
                hlslpp::float4 color;
                if(!def || !def->unlocked) {
                    // 割り当てが無い・未解放の枠
                    text  = std::to_wstring(button.slot) + L"  ？";
                    color = hlslpp::float4(0.35f, 0.35f, 0.4f, 0.8f);
                } else {
                    text = std::to_wstring(button.slot) + L" " + def->name + L"  " + std::to_wstring(def->cost);
                    if(busy || state.mana < def->cost) {
                        // マナ不足・ほかの魔法の効果中は暗く
                        color = hlslpp::float4(0.3f, 0.2f, 0.45f, 0.85f);
                    } else {
                        // 撃てる。カーソルが重なっていたら少し明るく
                        color = pointer.hovered ? hlslpp::float4(1.0f, 0.7f, 1.0f, 1.0f) : hlslpp::float4(0.85f, 0.45f, 0.95f, 1.0f);
                    }
                }
                sprite.tintColor = color;

                if(button.label != entt::null && registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label)) {
                    registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(button.label).text = text;
                }
            });
    }
}    // namespace FruitMagic::ECS

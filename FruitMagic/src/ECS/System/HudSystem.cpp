//----------------------------------------------------------------------------
//! @file   HudSystem.cpp
//! @brief  HUD（手持ち枚数・払い出し・収穫・ルーレット）を更新するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HudSystem.hpp>

#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Component/ManaGaugeComponent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
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
#include <cmath>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 払い出し表示を出しておく時間（秒）。続けて落ちたら延長して合算する
        constexpr float kPopupDuration = 1.2f;

        //! @brief 収穫表示を出しておく時間（秒）
        constexpr float kHarvestPopupDuration = 2.0f;

        //! @brief 「図鑑に登録！」を出しておく時間（秒）。通常の収穫表示より長く
        constexpr float kRegisteredPopupDuration = 3.5f;

        //--------------------------------------------------------------
        //! 果物の表示名（バリエーション名付き）を返します。
        //! @param  [in] registry     レジストリ（FruitCatalog・CollectionConfig を参照する）
        //! @param  [in] fruitIndex   果物の添字
        //! @param  [in] variantIndex バリエーションの添字
        //! @return 表示名。範囲外なら空文字列
        //--------------------------------------------------------------
        std::wstring FruitName(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex = 0) {
            if(!registry.HasContext<FruitCatalog>())
                return std::wstring();
            const auto& fruits = registry.GetContext<FruitCatalog>().Fruits();
            if(fruitIndex < 0 || fruitIndex >= static_cast<int>(fruits.size()))
                return std::wstring();
            if(!registry.HasContext<CollectionConfig>())
                return fruits[fruitIndex].name;
            return registry.GetContext<CollectionConfig>().DisplayName(fruits[fruitIndex], variantIndex);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    HudSystem::HudSystem(Tsukino::ECS::EventBus& eventBus) {
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind == PrizeKind::Fruit) {
                // 果物は払い出し口に落ちたときだけ「ゲット」を出す
                // （初めての枠なら、この後の ZukanRegisteredEvent で「図鑑に登録！」に差し替わる）
                if(e.zone == DropZone::Payout) {
                    m_harvestFruit   = e.fruitIndex;
                    m_harvestVariant = e.variantIndex;
                    m_harvestIsNew   = false;
                    m_harvestTimer   = kHarvestPopupDuration;
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

        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent& e) {
            m_harvestFruit   = e.fruitIndex;
            m_harvestVariant = e.variantIndex;
            m_harvestIsNew   = true;
            m_harvestTimer   = kRegisteredPopupDuration;
        });

        m_noticeConnection = eventBus.Subscribe<NoticeEvent>([this](const NoticeEvent& e) {
            m_noticeText  = e.text;
            m_noticeTimer = e.seconds;
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
        if(m_noticeTimer > 0.0f) {
            m_noticeTimer -= deltaTime;
            if(m_noticeTimer <= 0.0f)
                m_noticeText.clear();
        }

        const GameState     state    = registry.HasContext<GameState>() ? registry.GetContext<GameState>() : GameState{};
        const RouletteState roulette = registry.HasContext<RouletteState>() ? registry.GetContext<RouletteState>() : RouletteState{};
        const int           harvestTotal = state.HarvestTotal();

        CheckMagicUnlocks(registry, state);

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
                    font.text = L"果実: " + std::to_wstring(state.fruitPoints) + L"   (収穫 " + std::to_wstring(harvestTotal) + L")";
                    break;

                case HudTextKind::HarvestPopup:
                    if(m_harvestFruit < 0)
                        font.text.clear();
                    else if(m_harvestIsNew)
                        font.text = L"図鑑に登録！ " + FruitName(registry, m_harvestFruit, m_harvestVariant);
                    else
                        font.text = FruitName(registry, m_harvestFruit, m_harvestVariant) + L" ゲット！";
                    break;

                case HudTextKind::Roulette: {
                    std::wstring text;
                    switch(roulette.phase) {
                        case RoulettePhase::Spinning:
                            text = L"ルーレット ▶ " + ((roulette.displayFruit >= 0) ? FruitName(registry, roulette.displayFruit) : std::wstring(L"ハズレ"));
                            break;
                        case RoulettePhase::Result:
                            text = roulette.resultHit ? L"当たり！ " + FruitName(registry, roulette.displayFruit, roulette.displayVariant) + L" が出た！"
                                                      : std::wstring(L"ハズレ…");
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

                case HudTextKind::Notice:
                    font.text = m_noticeText;
                    break;

                case HudTextKind::ControlsHint:
#ifdef _DEBUG
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入   Tab: 図鑑   U: 強化   F5: コリジョン表示   F2: コイン・果実 +100・マナ満タン";
#else
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入   Tab: 図鑑   U: 強化";
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

        const MagicCatalog& catalog    = registry.GetContext<MagicCatalog>();
        const MagicState    magic      = registry.HasContext<MagicState>() ? registry.GetContext<MagicState>() : MagicState{};
        const int           registered = state.RegisteredCount();

        registry.View<MagicButtonComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, MagicButtonComponent& button, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                const int       index = catalog.FindBySlot(button.slot);
                const MagicDef* def   = (index >= 0) ? &catalog.Magics()[index] : nullptr;

                std::wstring   text;
                hlslpp::float4 color;
                if(!def) {
                    // 割り当てが無い枠
                    text  = std::to_wstring(button.slot) + L"  ？";
                    color = hlslpp::float4(0.35f, 0.35f, 0.4f, 0.8f);
                } else if(!catalog.IsUnlocked(index, registered)) {
                    // 未解放。あと何枠で覚えるかの目安に、必要な図鑑の登録数を出す
                    text  = std::to_wstring(button.slot) + L"  ？  図鑑" + std::to_wstring(def->unlockZukan);
                    color = hlslpp::float4(0.35f, 0.35f, 0.4f, 0.8f);
                } else if(magic.IsActive(index)) {
                    // 効果中は残り秒数（切り上げ）
                    const int seconds = static_cast<int>(std::ceil(magic.remaining[index]));
                    text              = std::to_wstring(button.slot) + L" " + def->name + L"  " + std::to_wstring(seconds) + L"秒";
                    color             = hlslpp::float4(0.55f, 0.35f, 0.75f, 0.9f);
                } else {
                    text = std::to_wstring(button.slot) + L" " + def->name + L"  " + std::to_wstring(def->cost);
                    if(state.mana < def->cost) {
                        // マナ不足は暗く
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

    //----------------------------------------------------------------------------
    //! 図鑑の登録数で新しく解放された魔法があれば、お知らせを出します。
    //----------------------------------------------------------------------------
    void HudSystem::CheckMagicUnlocks(Tsukino::ECS::Registry& registry, const GameState& state) {
        if(!registry.HasContext<MagicCatalog>())
            return;

        const MagicCatalog& catalog    = registry.GetContext<MagicCatalog>();
        const int           registered = state.RegisteredCount();

        int          unlocked = 0;
        std::wstring newest;    // 解放に必要な登録数がいちばん多い（＝今回増えた）魔法の名前
        int          newestRequirement = -1;
        for(int i = 0; i < static_cast<int>(catalog.Magics().size()); ++i) {
            if(!catalog.IsUnlocked(i, registered))
                continue;
            ++unlocked;
            if(catalog.Magics()[i].unlockZukan > newestRequirement) {
                newestRequirement = catalog.Magics()[i].unlockZukan;
                newest            = catalog.Magics()[i].name;
            }
        }

        // 起動直後（セーブを読んだ分）は出さず、プレイ中に増えたときだけ
        if(m_lastUnlocked >= 0 && unlocked > m_lastUnlocked) {
            m_noticeText  = L"新しい魔法「" + newest + L"」を覚えた！";
            m_noticeTimer = 4.0f;
        }
        m_lastUnlocked = unlocked;
    }
}    // namespace FruitMagic::ECS

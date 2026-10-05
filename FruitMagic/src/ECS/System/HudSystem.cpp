//----------------------------------------------------------------------------
//! @file   HudSystem.cpp
//! @brief  HUD（手持ち枚数・払い出し・収穫・ルーレット）を更新するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HudSystem.hpp>

#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/ECS/Component/MagicButtonComponent.hpp>
#include <FruitMagic/ECS/Component/ManaGaugeComponent.hpp>
#include <FruitMagic/ECS/Component/ReliefGaugeComponent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/EconomyConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/ReliefState.hpp>
#include <FruitMagic/Game/RouletteState.hpp>
#include <FruitMagic/Game/Settings.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

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
                    m_harvestTimer   = m_harvestPopupSeconds;
                }
                return;
            }

            if(e.zone == DropZone::Payout) {
                m_recentPayout += e.value;
            } else {
                m_recentGutter += 1;
            }
            m_popupTimer = m_payoutPopupSeconds;
        });

        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent& e) {
            m_harvestFruit   = e.fruitIndex;
            m_harvestVariant = e.variantIndex;
            m_harvestIsNew   = true;
            m_harvestTimer   = m_registeredPopupSeconds;
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
        const UiConfig& ui    = GetUiConfig(registry);
        const Texts&    texts = GetTexts(registry);

        // イベントのハンドラ（Update の外）で使う表示時間を覚えておく
        m_payoutPopupSeconds     = ui.payoutPopupSeconds;
        m_harvestPopupSeconds    = ui.harvestPopupSeconds;
        m_registeredPopupSeconds = ui.registeredPopupSeconds;

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
                    font.text = texts.Format("hud.coins", {{"n", std::to_wstring(state.coins)}});
                    break;

                case HudTextKind::Mana:
                    font.text = texts.Format("hud.mana", {{"mana", std::to_wstring(state.mana)}, {"max", std::to_wstring(state.maxMana)}});
                    break;

                case HudTextKind::DropPopup: {
                    std::wstring text;
                    if(m_recentPayout > 0)
                        text += texts.Format("hud.payout", {{"n", std::to_wstring(m_recentPayout)}});
                    if(m_recentGutter > 0)
                        text += texts.Format("hud.gutter", {{"n", std::to_wstring(m_recentGutter)}});
                    font.text = text;
                    break;
                }

                case HudTextKind::HarvestTotal:
                    font.text = texts.Format("hud.harvestTotal", {{"fruit", std::to_wstring(state.fruitPoints)}, {"harvest", std::to_wstring(harvestTotal)}});
                    break;

                case HudTextKind::HarvestPopup:
                    if(m_harvestFruit < 0)
                        font.text.clear();
                    else if(m_harvestIsNew)
                        font.text = texts.Format("hud.registered", {{"name", FruitName(registry, m_harvestFruit, m_harvestVariant)}});
                    else
                        font.text = texts.Format("hud.harvested", {{"name", FruitName(registry, m_harvestFruit, m_harvestVariant)}});
                    break;

                case HudTextKind::Roulette: {
                    std::wstring text;
                    switch(roulette.phase) {
                        case RoulettePhase::Spinning:
                            text = texts.Format("roulette.spinning", {{"name", (roulette.displayFruit >= 0) ? FruitName(registry, roulette.displayFruit) : texts.Get("roulette.miss")}});
                            break;
                        case RoulettePhase::Result:
                            if(roulette.resultHit)
                                text = texts.Format("roulette.hit", {{"name", FruitName(registry, roulette.displayFruit, roulette.displayVariant)}});
                            else if(roulette.resultCoins > 0)
                                text = texts.Format("roulette.coins", {{"n", std::to_wstring(roulette.resultCoins)}});
                            else
                                text = texts.Get("roulette.missResult");
                            break;
                        case RoulettePhase::JackpotSpin:
                            text = texts.Format("roulette.jackpotSpin", {{"display", roulette.jackpotDisplay ? texts.Get("roulette.jackpot") : texts.Get("roulette.miss")}});
                            break;
                        case RoulettePhase::JackpotResult:
                            text = roulette.jackpotWin ? texts.Get("roulette.jackpotWin") : texts.Get("roulette.jackpotLose");
                            break;
                        case RoulettePhase::Idle:
                        default:
                            break;
                    }
                    if(roulette.stock > 0)
                        text += texts.Format("roulette.stock", {{"n", std::to_wstring(roulette.stock)}});
                    font.text = text;

                    // ジャックポットチャンスの間は金色に
                    const bool jackpot = roulette.phase == RoulettePhase::JackpotSpin || roulette.phase == RoulettePhase::JackpotResult;
                    font.color         = jackpot ? ui.rouletteJackpotColor : ui.HudText("roulette").color;
                    break;
                }

                case HudTextKind::Relief: {
                    // 手持ちが少ない間だけ、妖精のおすそわけを待っていることと次の1枚までの秒数を出す
                    const ReliefState* relief = registry.HasContext<ReliefState>() ? &registry.GetContext<ReliefState>() : nullptr;
                    if(!relief || !relief->waiting) {
                        font.text.clear();
                        break;
                    }
                    const int seconds = std::max(1, static_cast<int>(std::ceil(relief->secondsRemaining)));
                    font.text         = texts.Format("hud.relief", {{"n", std::to_wstring(seconds)}});
                    break;
                }

                case HudTextKind::Notice:
                    font.text = m_noticeText;
                    break;

                case HudTextKind::ControlsHint:
                    // オプションで隠せる（操作説明はオプション画面にもある）
                    if(registry.HasContext<Settings>() && !registry.GetContext<Settings>().showControlsHint) {
                        font.text.clear();
                        break;
                    }
#ifdef _DEBUG
                    font.text = texts.Get("hud.controlsDebug");
#else
                    font.text = texts.Get("hud.controls");
#endif
                    break;
            }
        });

        UpdateManaGauge(registry, state);
        UpdateReliefGauge(registry);
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
    //! おすそわけ待ちのリングを、待っている間だけ出し、次の1枚までの進み具合で塗ります。
    //----------------------------------------------------------------------------
    void HudSystem::UpdateReliefGauge(Tsukino::ECS::Registry& registry) {
        const bool  waiting  = registry.HasContext<ReliefState>() && registry.GetContext<ReliefState>().waiting;
        const float interval = registry.HasContext<EconomyConfig>() ? registry.GetContext<EconomyConfig>().reliefSeconds : 0.0f;
        const float progress = (waiting && interval > 0.0f) ? std::clamp(1.0f - registry.GetContext<ReliefState>().secondsRemaining / interval, 0.0f, 1.0f)
                                                            : 0.0f;

        registry.View<ReliefGaugeComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity, ReliefGaugeComponent& gauge, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                // 待っていない間は拡大率 0 で隠す（拡大率 0 のスプライトは描かれない）
                const float scale = waiting ? gauge.shownScale : 0.0f;
                transform.scale   = hlslpp::float3(scale, scale, 1.0f);
                transform.dirty   = true;

                if(gauge.part == ReliefGaugePart::Fill)
                    sprite.fillAmount = progress;
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
        const UiConfig&     ui         = GetUiConfig(registry);
        const Texts&        texts      = GetTexts(registry);

        registry.View<MagicButtonComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::PointerTargetComponent>().each(
            [&](Tsukino::ECS::Entity, MagicButtonComponent& button, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                const int       index = catalog.FindBySlot(button.slot);
                const MagicDef* def   = (index >= 0) ? &catalog.Magics()[index] : nullptr;

                std::wstring   text;
                hlslpp::float4 color;
                if(!def) {
                    // 割り当てが無い枠
                    text  = texts.Format("magicButton.empty", {{"slot", std::to_wstring(button.slot)}});
                    color = ui.magicLockedColor;
                } else if(!catalog.IsUnlocked(index, registered)) {
                    // 未解放。あと何枠で覚えるかの目安に、必要な図鑑の登録数を出す
                    text  = texts.Format("magicButton.locked", {{"slot", std::to_wstring(button.slot)}, {"zukan", std::to_wstring(def->unlockZukan)}});
                    color = ui.magicLockedColor;
                } else if(magic.IsActive(index)) {
                    // 効果中は残り秒数（切り上げ）
                    const int seconds = static_cast<int>(std::ceil(magic.remaining[index]));
                    text              = texts.Format("magicButton.active", {{"slot", std::to_wstring(button.slot)}, {"name", def->name}, {"n", std::to_wstring(seconds)}});
                    color             = ui.magicActiveColor;
                } else {
                    text = texts.Format("magicButton.ready", {{"slot", std::to_wstring(button.slot)}, {"name", def->name}, {"cost", std::to_wstring(def->cost)}});
                    if(state.mana < def->cost) {
                        // マナ不足は暗く
                        color = ui.magicNoManaColor;
                    } else {
                        // 撃てる。カーソルが重なっていたら少し明るく
                        color = pointer.hovered ? ui.magicHoverColor : ui.magicReadyColor;
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
            m_noticeText  = GetTexts(registry).Format("notice.magicLearned", {{"name", newest}});
            m_noticeTimer = GetUiConfig(registry).magicLearnedSeconds;
        }
        m_lastUnlocked = unlocked;
    }
}    // namespace FruitMagic::ECS

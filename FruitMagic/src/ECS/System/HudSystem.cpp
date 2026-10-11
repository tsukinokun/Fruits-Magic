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
#include <FruitMagic/Game/FruitIcon.hpp>
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
#include <Tsukino/Core/Math/Easing.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

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
                    m_harvestStarted = true;
                }
                return;
            }

            if(e.zone == DropZone::Payout) {
                m_recentPayout += e.value;
            } else {
                m_recentGutter += 1;
            }
            m_popupTimer    = m_payoutPopupSeconds;
            m_payoutStarted = true;
        });

        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent& e) {
            m_harvestFruit   = e.fruitIndex;
            m_harvestVariant = e.variantIndex;
            m_harvestIsNew   = true;
            m_harvestTimer   = m_registeredPopupSeconds;
            m_harvestStarted = true;
        });

        m_noticeConnection = eventBus.Subscribe<NoticeEvent>([this](const NoticeEvent& e) {
            m_noticeText    = e.text;
            m_noticeTimer   = e.seconds;
            m_noticeStarted = true;
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

        CheckMagicUnlocks(registry, state);

        //--------------------------------------------------------------
        // UI の動きのきっかけ。一時表示は出し直したらポップさせ、消える少し前から薄くする。
        // コイン・FP・マナの数は実際の数へ追いつかせ（カウントアップ）、増えたら文字とマークを弾ませる
        //--------------------------------------------------------------
        const UiMotionStyle& motion         = ui.motion;
        const bool           payoutStarted  = std::exchange(m_payoutStarted, false);
        const bool           harvestStarted = std::exchange(m_harvestStarted, false);
        const bool           noticeStarted  = std::exchange(m_noticeStarted, false);

        // 一時表示の動き（出し直したらポップ、残り時間が短くなったらフェードアウト）
        auto animateTimed = [&](Tsukino::ECS::Entity entity, bool started, float timer) {
            if(started)
                PlayPopIn(registry, entity);
            else if(timer > 0.0f && timer < motion.fadeOutSeconds)
                PlayFadeOut(registry, entity, timer);
        };

        const long long shownCoins = m_coinsShown.Update(state.coins, motion.countUpSpeed, deltaTime);
        const long long shownFruit = m_fruitShown.Update(state.fruitPoints, motion.countUpSpeed, deltaTime);
        const long long shownMana  = m_manaShown.Update(state.mana, motion.countUpSpeed, deltaTime);
        registry.View<MoneyIconComponent>().each([&](Tsukino::ECS::Entity entity, MoneyIconComponent& icon) {
            if((icon.kind == HudTextKind::Coins && m_coinsShown.Increased()) || (icon.kind == HudTextKind::FruitPoints && m_fruitShown.Increased()))
                PlayPunch(registry, entity);
        });

        //--------------------------------------------------------------
        // 「〇〇 ゲット！」の左の果物（3D）。収穫の表示が出ている間だけ見せる
        //--------------------------------------------------------------
        registry.View<HarvestIconComponent>().each([&](Tsukino::ECS::Entity entity, HarvestIconComponent& icon) {
            if(m_harvestFruit >= 0)
                SetFruitIcon(registry, entity, m_harvestFruit, m_harvestVariant, false, icon.sizePixels);
            SetFruitIconVisible(registry, entity, m_harvestFruit >= 0);
            animateTimed(entity, harvestStarted, m_harvestTimer);
        });

        //--------------------------------------------------------------
        // 各テキストの更新
        //--------------------------------------------------------------
        auto view = registry.View<HudTextComponent, Tsukino::BuiltIn::ECS::FontComponent>();
        view.each([&](Tsukino::ECS::Entity entity, HudTextComponent& hud, Tsukino::BuiltIn::ECS::FontComponent& font) {
            const bool wasEmpty = font.text.empty();
            switch(hud.kind) {
                case HudTextKind::Coins:
                    font.text = texts.Format("hud.coins", {{"n", std::to_wstring(shownCoins)}});
                    if(m_coinsShown.Increased())
                        PlayPunch(registry, entity);
                    break;

                case HudTextKind::Mana:
                    font.text = texts.Format("hud.mana", {{"mana", std::to_wstring(shownMana)}, {"max", std::to_wstring(state.maxMana)}});
                    break;

                case HudTextKind::DropPopup: {
                    std::wstring text;
                    if(m_recentPayout > 0)
                        text += texts.Format("hud.payout", {{"n", std::to_wstring(m_recentPayout)}});
                    if(m_recentGutter > 0)
                        text += texts.Format("hud.gutter", {{"n", std::to_wstring(m_recentGutter)}});
                    font.text = text;
                    animateTimed(entity, payoutStarted, m_popupTimer);
                    break;
                }

                case HudTextKind::FruitPoints:
                    font.text = texts.Format("hud.fruitPoints", {{"n", std::to_wstring(shownFruit)}});
                    if(m_fruitShown.Increased())
                        PlayPunch(registry, entity);
                    break;

                case HudTextKind::HarvestPopup:
                    if(m_harvestFruit < 0)
                        font.text.clear();
                    else if(m_harvestIsNew)
                        font.text = texts.Format("hud.registered", {{"name", FruitName(registry, m_harvestFruit, m_harvestVariant)}});
                    else
                        font.text = texts.Format("hud.harvested", {{"name", FruitName(registry, m_harvestFruit, m_harvestVariant)}});
                    animateTimed(entity, harvestStarted, m_harvestTimer);
                    break;

                case HudTextKind::Roulette: {
                    std::wstring text;
                    switch(roulette.phase) {
                        case RoulettePhase::Spinning:
                            // リールはスロットが見せるので、文字はリーチのときだけ
                            if(roulette.reach && roulette.reelsStopped >= kReelCount - 1)
                                text = texts.Get("roulette.reach");
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
                            text = texts.Get("roulette.jackpotSpin");
                            break;
                        case RoulettePhase::JackpotResult:
                            text = roulette.jackpotWin ? texts.Get("roulette.jackpotWin") : texts.Get("roulette.jackpotLose");
                            break;
                        case RoulettePhase::Idle:
                        default:
                            break;
                    }
                    // ためている回数はスロットの下の玉で見せる。文字が変わったらポップさせる
                    if(!text.empty() && text != font.text)
                        PlayPopIn(registry, entity);
                    font.text = text;

                    // リーチの間は呼吸させて盛り上げる
                    const bool reach = roulette.phase == RoulettePhase::Spinning && roulette.reach && roulette.reelsStopped >= kReelCount - 1;
                    SetPulse(registry, entity, reach);

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
                    // 秒数は毎秒変わるので、出始めたときだけポップさせる
                    if(wasEmpty)
                        PlayPopIn(registry, entity);
                    break;
                }

                case HudTextKind::Notice:
                    font.text = m_noticeText;
                    animateTimed(entity, noticeStarted, m_noticeTimer);
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

        UpdateManaGauge(registry, state, deltaTime);
        UpdateReliefGauge(registry);
        UpdateMagicButtons(registry, state);
    }

    //----------------------------------------------------------------------------
    //! マナゲージの中身のバーの幅を、今のマナに合わせます。
    //----------------------------------------------------------------------------
    void HudSystem::UpdateManaGauge(Tsukino::ECS::Registry& registry, const GameState& state, float deltaTime) {
        const float target = (state.maxMana > 0) ? std::clamp(static_cast<float>(state.mana) / static_cast<float>(state.maxMana), 0.0f, 1.0f) : 0.0f;

        // 実際の割合へなめらかに追いつかせる（最初は実際の割合から）
        m_manaRatio = (m_manaRatio < 0.0f) ? target : Tsukino::Core::Math::Easing::Damp(m_manaRatio, target, GetUiConfig(registry).motion.gaugeSpeed, deltaTime);
        if(std::abs(m_manaRatio - target) < 0.001f)
            m_manaRatio = target;
        const float ratio = m_manaRatio;

        // 満タンになった瞬間に弾ませる
        const bool full      = state.maxMana > 0 && state.mana >= state.maxMana;
        const bool becameFull = full && !m_manaFull;
        m_manaFull            = full;

        registry.View<ManaGaugeComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, ManaGaugeComponent& gauge, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(becameFull)
                    PlayPunch(registry, entity);
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

        // 待ち始めた瞬間にポップさせる
        const bool startedWaiting = waiting && !m_wasWaiting;
        m_wasWaiting              = waiting;

        registry.View<ReliefGaugeComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, ReliefGaugeComponent& gauge, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                if(startedWaiting)
                    PlayPopIn(registry, entity);
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
            [&](Tsukino::ECS::Entity entity, MagicButtonComponent& button, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::PointerTargetComponent& pointer) {
                const int       index = catalog.FindBySlot(button.slot);
                const MagicDef* def   = (index >= 0) ? &catalog.Magics()[index] : nullptr;

                // 撃てる間だけ手ごたえを出し、呼吸させて撃てることを知らせる
                const bool castable = def && catalog.IsUnlocked(index, registered) && !magic.IsActive(index) && state.mana >= def->cost;
                SetButtonInteractable(registry, entity, castable);
                SetPulse(registry, entity, castable);

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
            m_noticeText    = GetTexts(registry).Format("notice.magicLearned", {{"name", newest}});
            m_noticeTimer   = GetUiConfig(registry).magicLearnedSeconds;
            m_noticeStarted = true;
        }
        m_lastUnlocked = unlocked;
    }
}    // namespace FruitMagic::ECS

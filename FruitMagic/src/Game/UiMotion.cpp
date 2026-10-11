//----------------------------------------------------------------------------
//! @file   UiMotion.cpp
//! @brief  画面の UI の動きを付ける関数の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/UiMotion.hpp>

#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/UIButtonFeedbackComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UIMotionComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UITweenComponent.hpp>
#include <Tsukino/BuiltIn/ECS/UI/UITweenUtility.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Math/Easing.hpp>

#include <cmath>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        using Tsukino::BuiltIn::ECS::UITween;
        using Tsukino::BuiltIn::ECS::UITweenLoop;
        using Tsukino::BuiltIn::ECS::UITweenProperty;
        using Tsukino::Core::Math::Easing::EaseType;
        namespace TweenUtility = Tsukino::BuiltIn::ECS::UITweenUtility;

        //--------------------------------------------------------------
        //! トゥイーンを1本作ります。
        //! @param  [in] property 動かす値
        //! @param  [in] from     始めの値
        //! @param  [in] to       終わりの値
        //! @param  [in] seconds  長さ（秒）
        //! @param  [in] ease     緩急の付け方
        //! @param  [in] tag      種類の目印
        //! @return トゥイーン
        //--------------------------------------------------------------
        UITween MakeTween(UITweenProperty property, float from, float to, float seconds, EaseType ease, std::uint32_t tag) {
            UITween tween;
            tween.property = property;
            tween.from     = from;
            tween.to       = to;
            tween.duration = seconds;
            tween.ease     = ease;
            tween.tag      = tag;
            return tween;
        }

        //--------------------------------------------------------------
        //! 動きの調整値を返します。
        //! @param  [in] registry レジストリ
        //! @return 動きの調整値
        //--------------------------------------------------------------
        const UiMotionStyle& Style(Tsukino::ECS::Registry& registry) {
            return GetUiConfig(registry).motion;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! ボタンに手ごたえを付けます。
    //----------------------------------------------------------------------------
    void AddButtonMotion(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button, Tsukino::ECS::Entity label) {
        const UiMotionStyle& style = Style(registry);

        Tsukino::BuiltIn::ECS::UIButtonFeedbackComponent& feedback = registry.HasComponent<Tsukino::BuiltIn::ECS::UIButtonFeedbackComponent>(button)
                                                                         ? registry.GetComponent<Tsukino::BuiltIn::ECS::UIButtonFeedbackComponent>(button)
                                                                         : registry.AddComponent<Tsukino::BuiltIn::ECS::UIButtonFeedbackComponent>(button);
        feedback.hoverScale                                        = style.buttonHoverScale;
        feedback.pressScale                                        = style.buttonPressScale;
        feedback.clickPunch                                        = style.buttonClickPunch;
        feedback.speed                                             = style.buttonSpeed;

        if(label != entt::null)
            FollowUiMotion(registry, label, button);
    }

    //----------------------------------------------------------------------------
    //! ボタンが押せる状態かを設定します。
    //----------------------------------------------------------------------------
    void SetButtonInteractable(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button, bool interactable) {
        if(auto* feedback = registry.try_get<Tsukino::BuiltIn::ECS::UIButtonFeedbackComponent>(button))
            feedback->interactable = interactable;
    }

    //----------------------------------------------------------------------------
    //! 別のエンティティの動きを写すようにします。
    //----------------------------------------------------------------------------
    void FollowUiMotion(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, Tsukino::ECS::Entity source) {
        Tsukino::BuiltIn::ECS::UIMotionComponent& motion = registry.HasComponent<Tsukino::BuiltIn::ECS::UIMotionComponent>(entity)
                                                               ? registry.GetComponent<Tsukino::BuiltIn::ECS::UIMotionComponent>(entity)
                                                               : registry.AddComponent<Tsukino::BuiltIn::ECS::UIMotionComponent>(entity);
        motion.source                                    = source;
    }

    //----------------------------------------------------------------------------
    //! ポップインを再生します。
    //----------------------------------------------------------------------------
    void PlayPopIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        const UiMotionStyle& style = Style(registry);
        StopFadeOut(registry, entity);
        TweenUtility::Play(
            registry, entity, MakeTween(UITweenProperty::Scale, style.popFromScale, 1.0f, style.popSeconds, EaseType::OutBack, UiMotionTag::kPop));
    }

    //----------------------------------------------------------------------------
    //! 画面を開くときのポップとフェードインを再生します。
    //----------------------------------------------------------------------------
    void PlayPanelOpen(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity panel) {
        const UiMotionStyle& style = Style(registry);
        TweenUtility::Play(
            registry, panel, MakeTween(UITweenProperty::Scale, style.panelFromScale, 1.0f, style.panelSeconds, EaseType::OutBack, UiMotionTag::kPanel));
        TweenUtility::Play(registry, panel, MakeTween(UITweenProperty::Alpha, 0.0f, 1.0f, style.panelSeconds * 0.6f, EaseType::OutCubic, UiMotionTag::kFadeIn));
    }

    //----------------------------------------------------------------------------
    //! フェードインを再生します。
    //----------------------------------------------------------------------------
    void PlayFadeIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        const UiMotionStyle& style = Style(registry);
        TweenUtility::Play(
            registry, entity, MakeTween(UITweenProperty::Alpha, 0.0f, 1.0f, style.panelSeconds * 0.6f, EaseType::OutCubic, UiMotionTag::kFadeIn));
    }

    //----------------------------------------------------------------------------
    //! スライドインを再生します。
    //----------------------------------------------------------------------------
    void PlaySlideIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, float direction, float delay) {
        const UiMotionStyle& style = Style(registry);

        UITween slide = MakeTween(UITweenProperty::OffsetX, style.slideDistance * direction, 0.0f, style.slideSeconds, EaseType::OutCubic, UiMotionTag::kSlide);
        slide.delay   = delay;
        TweenUtility::Play(registry, entity, slide);

        UITween fade = MakeTween(UITweenProperty::Alpha, 0.0f, 1.0f, style.slideSeconds * 0.7f, EaseType::OutQuad, UiMotionTag::kFadeIn);
        fade.delay   = delay;
        TweenUtility::Play(registry, entity, fade);
    }

    //----------------------------------------------------------------------------
    //! 押せる物の呼吸を出すか止めるかを切り替えます。
    //----------------------------------------------------------------------------
    void SetPulse(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, bool active) {
        const bool playing = TweenUtility::IsPlaying(registry, entity, UiMotionTag::kPulse);
        if(active == playing)
            return;
        if(!active) {
            TweenUtility::Stop(registry, entity, UiMotionTag::kPulse);
            return;
        }

        const UiMotionStyle& style = Style(registry);
        UITween              pulse = MakeTween(UITweenProperty::Scale, 1.0f, style.pulseScale, style.pulseSeconds, EaseType::InOutSine, UiMotionTag::kPulse);
        pulse.loop                 = UITweenLoop::PingPong;
        TweenUtility::Play(registry, entity, pulse);
    }

    //----------------------------------------------------------------------------
    //! 弾む動きを再生します。
    //----------------------------------------------------------------------------
    void PlayPunch(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        const UiMotionStyle& style = Style(registry);
        TweenUtility::Play(
            registry, entity, MakeTween(UITweenProperty::Scale, 1.0f, 1.0f + style.punchScale, style.punchSeconds, EaseType::Punch, UiMotionTag::kPunch));
    }

    //----------------------------------------------------------------------------
    //! フェードアウトを再生します。
    //----------------------------------------------------------------------------
    void PlayFadeOut(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, float seconds) {
        if(TweenUtility::IsPlaying(registry, entity, UiMotionTag::kFadeOut))
            return;
        UITween fade   = MakeTween(UITweenProperty::Alpha, 1.0f, 0.0f, seconds, EaseType::InQuad, UiMotionTag::kFadeOut);
        fade.holdAtEnd = true;
        TweenUtility::Play(registry, entity, fade);
    }

    //----------------------------------------------------------------------------
    //! フェードアウトを取り消します。
    //----------------------------------------------------------------------------
    void StopFadeOut(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        TweenUtility::Stop(registry, entity, UiMotionTag::kFadeOut);
    }

    //----------------------------------------------------------------------------
    //! ときどき裏返る動きを繰り返します。
    //----------------------------------------------------------------------------
    void PlayIconFlip(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        const UiMotionStyle& style = Style(registry);
        // 横の拡大率を 1 → 0 → 1 にすると、薄くなってまた開くので裏返ったように見える
        // （負の拡大率は画面スプライトの裏面になるので使わない）
        UITween flip     = MakeTween(UITweenProperty::ScaleX, 1.0f, 0.0f, style.iconFlipSeconds * 0.5f, EaseType::InOutSine, UiMotionTag::kFlip);
        flip.loop        = UITweenLoop::PingPong;
        flip.repeatDelay = style.iconFlipInterval;
        TweenUtility::Play(registry, entity, flip);
    }

    //----------------------------------------------------------------------------
    //! 表示する数を実際の数へ近づけます。
    //----------------------------------------------------------------------------
    long long CountUpValue::Update(long long target, float speed, float deltaTime) {
        if(!m_initialized) {
            Snap(target);
            m_initialized = true;
            m_increased   = false;
            return target;
        }

        m_increased  = target > m_lastTarget;
        m_lastTarget = target;

        // 0.5 より近づいたら揃える（いつまでも 1 だけずれて見えないように）
        m_shown = Tsukino::Core::Math::Easing::Damp(m_shown, static_cast<double>(target), speed, deltaTime);
        if(std::abs(m_shown - static_cast<double>(target)) < 0.5)
            m_shown = static_cast<double>(target);
        return std::llround(m_shown);
    }

    //----------------------------------------------------------------------------
    //! 表示する数を実際の数にそろえます。
    //----------------------------------------------------------------------------
    void CountUpValue::Snap(long long target) {
        m_shown      = static_cast<double>(target);
        m_lastTarget = target;
    }
}    // namespace FruitMagic

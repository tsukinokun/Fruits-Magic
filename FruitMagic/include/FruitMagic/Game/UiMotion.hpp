//----------------------------------------------------------------------------
//! @file   UiMotion.hpp
//! @brief  画面の UI の動き（ボタンの手ごたえ・ポップ・スライド・呼吸・弾み・フェード）を付ける関数
//! @detail エンジンの UITween・UIMotion・UICanvasGroup・UIButtonFeedback を、Ui.json の "motion" の値で付けます。
//!         同じ種類の動きは同じ tag で再生するので、出し直すと前の動きは置き換わります。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <cstdint>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 動きの種類の目印（UITween::tag）です。
    namespace UiMotionTag {
        inline constexpr std::uint32_t kPop     = 1;    // ポップイン
        inline constexpr std::uint32_t kPanel   = 2;    // 画面を開くときのポップ（拡大）
        inline constexpr std::uint32_t kFadeIn  = 3;    // フェードイン
        inline constexpr std::uint32_t kSlide   = 4;    // スライドイン
        inline constexpr std::uint32_t kPulse   = 5;    // 押せる物の呼吸
        inline constexpr std::uint32_t kPunch   = 6;    // 弾み
        inline constexpr std::uint32_t kFadeOut = 7;    // フェードアウト
        inline constexpr std::uint32_t kFlip    = 8;    // くるっと裏返る
    }    // namespace UiMotionTag

    //! ボタンに手ごたえ（重なると大きく・押すと縮む・クリックで弾む）を付けます。
    //! @param  [in] registry レジストリ
    //! @param  [in] button   ボタン（PointerTargetComponent を持つスプライト）
    //! @param  [in] label    ボタンの上の文字（一緒に動かす。無ければ entt::null）
    void AddButtonMotion(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button, Tsukino::ECS::Entity label);

    //! ボタンが押せる状態かを設定します（押せない間は手ごたえが無い）。
    //! @param  [in] registry     レジストリ
    //! @param  [in] button       ボタン
    //! @param  [in] interactable 押せるなら true
    void SetButtonInteractable(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button, bool interactable);

    //! 別のエンティティの動きを写すようにします（パネルの部品・ボタンの上の文字など）。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    //! @param  [in] source   写し元
    void FollowUiMotion(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, Tsukino::ECS::Entity source);

    //! 小さい所から少し行き過ぎて元の大きさになるポップインを再生します（フェードアウト中なら取り消す）。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    void PlayPopIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

    //! 画面を開くときのポップ（少し小さい所から拡大）とフェードインを再生します。
    //! @param  [in] registry レジストリ
    //! @param  [in] panel    画面の板（ほかの部品は FollowUiMotion でこれを写す）
    void PlayPanelOpen(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity panel);

    //! フェードインを再生します。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    void PlayFadeIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

    //! 横からすべり込むスライドイン（とフェードイン）を再生します。
    //! @param  [in] registry  レジストリ
    //! @param  [in] entity    動かすエンティティ
    //! @param  [in] direction 来る向き（-1 なら左から、+1 なら右から）
    //! @param  [in] delay     始めるまでの待ち（秒。行ごとにずらす）
    void PlaySlideIn(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, float direction, float delay);

    //! 押せる物の呼吸（ゆっくり大きくなったり戻ったり）を出すか止めるかを切り替えます。変わったときだけ再生し直します。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    //! @param  [in] active   呼吸させるなら true
    void SetPulse(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, bool active);

    //! ぽんと弾む動きを再生します（数字が増えたとき・ゲージが満タンになったとき）。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    void PlayPunch(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

    //! フェードアウトを再生します（終わったら透明のまま。PlayPopIn で取り消す）。すでにフェードアウト中なら何もしません。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    //! @param  [in] seconds  薄くなる秒数
    void PlayFadeOut(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, float seconds);

    //! フェードアウトを取り消します（透明のままにしない）。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   エンティティ
    void StopFadeOut(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

    //! ときどきくるっと裏返る動きを繰り返します（お金のマーク）。
    //! @param  [in] registry レジストリ
    //! @param  [in] entity   動かすエンティティ
    void PlayIconFlip(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

    //! 画面に出す数を、実際の数へなめらかに追いつかせる（カウントアップ）表示用の値です。
    class CountUpValue {
    public:
        //! 表示する数を実際の数へ近づけます。最初の1回は実際の数にそろえます。
        //! @param  [in] target    実際の数
        //! @param  [in] speed     近づく速さ（1秒あたりの減衰率）
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        //! @return 表示する数
        long long Update(long long target, float speed, float deltaTime);

        //! 表示する数を実際の数にそろえます（画面を開き直したときなど）。
        //! @param  [in] target 実際の数
        void Snap(long long target);

        //! 前のフレームより実際の数が増えたかを返します（Update の後に呼ぶ）。
        //! @return 増えたら true
        bool Increased() const { return m_increased; }

    private:
        double    m_shown       = 0.0;      // 表示している数（小数で持って少しずつ近づける）
        long long m_lastTarget  = 0;        // 前のフレームの実際の数
        bool      m_initialized = false;    // 一度でも Update したか
        bool      m_increased   = false;    // 今回の Update で実際の数が増えたか
    };
}    // namespace FruitMagic

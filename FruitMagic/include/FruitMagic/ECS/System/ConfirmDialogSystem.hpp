//----------------------------------------------------------------------------
//! @file   ConfirmDialogSystem.hpp
//! @brief  確認ウィンドウ（データ消去・おひっこし）のシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 確認ウィンドウのシステムです。各画面のシステムが ConfirmState::Open で出した確認を表示し、
    //! 「はい」を最後の回まで選ぶと SceneRequest を立てます（データ消去は restart、おひっこしは move）。
    //! 文言は Texts.json の、データ消去は "options.confirm"、おひっこしは "move.confirm" の下
    //! （message<n>・note<n>・yes<n>・no。{seeds} はおひっこしでもらえる星のたね）。
    //! 出した画面が閉じたら取り消します。出てすぐの間は「はい」を受け付けません（連打で通らないように）。
    class ConfirmDialogSystem : public Tsukino::ECS::ISystem {
    public:

        //! ボタンの入力を読み、確認ウィンドウを更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        int   m_shownStep = 0;       // 前のフレームに出していた確認の回（変わったら待ち時間を数え直す）
        float m_stepTimer = 0.0f;    // 今の回を出してからの時間（秒）
    };
}    // namespace FruitMagic::ECS

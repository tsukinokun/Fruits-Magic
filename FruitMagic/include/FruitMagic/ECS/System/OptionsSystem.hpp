//----------------------------------------------------------------------------
//! @file   OptionsSystem.hpp
//! @brief  オプション画面（音量・消音・操作説明の表示・終了・データの消去）のシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! オプション画面のシステムです。オプション画面が開いている間（開閉は MenuSystem）、
    //! ボタンのクリックで設定（Settings）を変えて保存し、ボタンの文字・色と音量の表示を更新します。
    //! 「データを消して最初から」は確認ウィンドウ（OptionsDialogComponent）を出し、3回「消す」を選ぶと
    //! SceneRequest::restart を立てます。確認の何回目かは OptionsState に持ちます。
    //! 「ゲームを終了」は SceneRequest::quit を立てます（実行は PusherScene）。
    class OptionsSystem : public Tsukino::ECS::ISystem {
    public:

        //! ボタンの入力を読み、オプション画面と確認ウィンドウを更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        //! オプション画面のボタンのクリックを処理し、ボタンの文字・色と音量の表示を更新します。
        //! @param  [in] registry   レジストリ
        //! @param  [in] confirming 確認ウィンドウを出しているか（出していればクリックは処理しない）
        void UpdatePage(Tsukino::ECS::Registry& registry, bool confirming);

        //! 確認ウィンドウのボタンのクリックを処理し、表示を更新します。
        //! @param  [in] registry レジストリ
        void UpdateDialog(Tsukino::ECS::Registry& registry);

        int   m_shownStep  = 0;       // 前のフレームに出していた確認の回（変わったら待ち時間を数え直す）
        float m_stepTimer  = 0.0f;    // 今の回を出してからの時間（秒）
    };
}    // namespace FruitMagic::ECS

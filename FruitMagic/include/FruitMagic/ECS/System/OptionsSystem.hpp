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
    //! 「データを消して最初から」は確認ウィンドウを出します（ConfirmState::Open。表示と実行は ConfirmDialogSystem）。
    //! 「ゲームを終了」は SceneRequest::quit を立てます（実行は PusherScene）。
    class OptionsSystem : public Tsukino::ECS::ISystem {
    public:

        //! ボタンの入力を読み、オプション画面を更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        //! オプション画面のボタンのクリックを処理し、ボタンの文字・色と音量の表示を更新します。
        //! @param  [in] registry   レジストリ
        //! @param  [in] confirming 確認ウィンドウを出しているか（出していればクリックは処理しない）
        void UpdatePage(Tsukino::ECS::Registry& registry, bool confirming);
    };
}    // namespace FruitMagic::ECS

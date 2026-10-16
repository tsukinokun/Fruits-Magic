//----------------------------------------------------------------------------
//! @file   ConfirmState.hpp
//! @brief  確認ウィンドウの状態（何の確認か・何回目か・どの画面から出したか）
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/MenuState.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 確認ウィンドウで何を確かめているかです。
    enum class ConfirmPurpose {
        None,     // 出していない
        Reset,    // データを消して最初から（オプション画面）
        Move,     // 屋台のおひっこし（強化画面の星のたねのタブ）
    };

    //! 確認の回数を返します（最後の回で「はい」を選ぶと実行する）。
    //! @param  [in] purpose 何の確認か
    //! @return 回数（データ消去は3回、おひっこしは1回）
    inline constexpr int ConfirmSteps(ConfirmPurpose purpose) {
        return (purpose == ConfirmPurpose::Reset) ? 3 : 1;
    }

    //! 確認ウィンドウの状態です（レジストリのコンテキスト）。出すのは各画面のシステム（Open）、
    //! 表示とボタンは ConfirmDialogSystem、確認中の Esc・画面の切り替え・スクロールを止めるのは MenuSystem。
    struct ConfirmState {
        ConfirmPurpose purpose = ConfirmPurpose::None;    // 何の確認か
        int            step    = 0;                       // 何回目を出しているか（0 は出していない。1〜ConfirmSteps）
        MenuKind       owner   = MenuKind::None;          // 出した画面（この画面が閉じたら取り消す）

        //! 確認を出しているかを返します。
        //! @return 出していれば true
        bool IsConfirming() const { return step > 0; }

        //! 確認の1回目を出します。
        //! @param  [in] what 何の確認か
        //! @param  [in] from 出した画面
        void Open(ConfirmPurpose what, MenuKind from) {
            purpose = what;
            owner   = from;
            step    = 1;
        }

        //! 確認を閉じます（取り消し・実行のどちらでも）。
        void Close() {
            purpose = ConfirmPurpose::None;
            owner   = MenuKind::None;
            step    = 0;
        }
    };
}    // namespace FruitMagic

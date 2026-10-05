//----------------------------------------------------------------------------
//! @file   OptionsState.hpp
//! @brief  オプション画面の状態（データ消去の確認がどこまで進んだか）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! データ消去の確認は3回まで出します。
    inline constexpr int kResetConfirmSteps = 3;

    //! オプション画面の状態です（レジストリのコンテキスト）。OptionsSystem が書き、
    //! MenuSystem が確認中の Esc・画面の切り替え・スクロールを止めるのに読みます。
    struct OptionsState {
        int confirmStep = 0;    // データ消去の確認の何回目を出しているか（0 は出していない。1〜kResetConfirmSteps）

        //! データ消去の確認を出しているかを返します。
        //! @return 出していれば true
        bool IsConfirming() const { return confirmStep > 0; }
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   Settings.hpp
//! @brief  オプションの設定（音量・消音・操作説明の表示）
//! @detail セーブデータとは別の Saves/settings.json に保存します。
//!         「データを消して最初から」でもセーブデータだけが消え、設定は残ります。
//!         まだ保存していないときの値（初めて遊ぶときの音量など）は Assets/Data/Settings.json に書きます。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! オプションの設定です。Registry のコンテキストに置きます。
    struct Settings {
        float       bgmVolume        = 0.7f;     // BGM の音量（0〜1、10% 刻み。既定は Assets/Data/Settings.json）
        float       seVolume         = 0.8f;     // 効果音の音量（0〜1、10% 刻み。同上）
        bool        muted            = false;    // 消音中か（M キーでも切り替わる）
        bool        showControlsHint = true;     // 画面下に操作説明を出すか
        bool        showCutIn        = true;     // 果物が取れたときのカットイン（図鑑に初めて載った・色違い）を出すか
        std::string path;                        // 保存先（空なら保存しない。バランス計測の自動プレイは読むだけで保存しない）

        //! 既定の保存先を返します（アセットルートの Saves/settings.json）。
        //! @return 保存先のパス
        static std::string DefaultPath();

        //! 既定の値（まだ保存していないときの値）を読み込みます。無い項目はコードの値のままです。
        //! @param  [in] defaultsPath 既定の値のファイル（Assets/Data/Settings.json）
        //! @return 読み込めたら true
        bool LoadDefaults(const std::string& defaultsPath);

        //! 保存先から読み込みます。無い・壊れているときは今の値（既定の値）のままです。
        //! @param  [in] filePath 保存先
        //! @param  [in] saveBack 変えたときにこのファイルへ保存するか（false なら path を空にして読むだけ）
        //! @return 読み込めたら true
        bool Load(const std::string& filePath, bool saveBack = true);

        //! 保存します（一時ファイルに書いてから置き換える）。path が空なら何もしません。
        //! @return 保存できたら true
        bool Save() const;

        //! 音量を 10% 動かします（0〜1 に丸め、刻みの端数も丸める）。
        //! @param  [in,out] volume 音量
        //! @param  [in]     steps  動かす段数（+1 で 10% 上げる、-1 で下げる）
        static void StepVolume(float& volume, int steps);
    };
}    // namespace FruitMagic

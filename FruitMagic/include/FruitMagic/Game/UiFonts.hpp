//----------------------------------------------------------------------------
//! @file   UiFonts.hpp
//! @brief  画面の文字に使うフォント（ふつう・太字）
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Engine/Asset/AssetHandle.hpp>

// 名前空間 : Tsukino::Asset
namespace Tsukino::Asset {
    class AssetManager;    // 前方宣言
}

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {
    struct UiConfig;    // 前方宣言

    //! 画面の文字に使うフォントです。Registry のコンテキストに置きます。
    //! ファイルは UiConfig（Ui.json の "fonts"）で決まり、読み込めなかったものは空のハンドル（エンジンの標準フォント）になります。
    struct UiFonts {
        Tsukino::Asset::AssetHandle regular;    // ふつう（説明文など読む文字）
        Tsukino::Asset::AssetHandle bold;       // 太字（数・タイトル・ボタンなど目立たせる文字）

        //! フォントを読み込みます。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] ui           画面の設定（フォントのファイル）
        void Load(Tsukino::Asset::AssetManager& assetManager, const UiConfig& ui);

        //! 太さに合うフォントを返します。
        //! @param  [in] isBold 太字か
        //! @return フォントのハンドル（空ならエンジンの標準フォント）
        const Tsukino::Asset::AssetHandle& Get(bool isBold) const { return isBold ? bold : regular; }
    };

    //! レジストリに置いたフォントのうち、太さに合うものを返します。
    //! @param  [in] registry レジストリ
    //! @param  [in] isBold   太字か
    //! @return フォントのハンドル（コンテキストに無ければ空のハンドル＝エンジンの標準フォント）
    Tsukino::Asset::AssetHandle GetUiFont(Tsukino::ECS::Registry& registry, bool isBold);
}    // namespace FruitMagic

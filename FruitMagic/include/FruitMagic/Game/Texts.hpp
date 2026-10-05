//----------------------------------------------------------------------------
//! @file   Texts.hpp
//! @brief  画面に出す文言（Assets/Data/Texts.json）
//! @detail 文言は "hud": { "coins": "コイン: {n}" } のように画面ごとにまとめて書き、コードからは "hud.coins" で引きます。
//!         {n} や {name} の所には Format で値を差し込みます。
//----------------------------------------------------------------------------
#pragma once
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <utility>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 画面に出す文言です。Registry のコンテキストに置きます。
    class Texts {
    public:

        //! 差し込む値（{名前} と、そこに入れる文字列）です。
        using Arg = std::pair<const char*, std::wstring>;

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Texts.json）
        //! @return 読み込めたら true（読めなくても動くが、文言は [キー] になる）
        bool Load(const std::string& path);

        //! 文言を返します。
        //! @param  [in] key 文言のキー（"hud.coins" など）
        //! @return 文言。無ければ "[キー]"（画面で抜けに気づけるように。ログにも1回だけ出す）
        const std::wstring& Get(const std::string& key) const;

        //! 文言の {名前} に値を差し込んで返します。
        //! @param  [in] key  文言のキー
        //! @param  [in] args 差し込む値
        //! @return 差し込んだ文言
        std::wstring Format(const std::string& key, std::initializer_list<Arg> args) const;

    private:
        std::unordered_map<std::string, std::wstring>         m_texts;      // キーごとの文言
        mutable std::unordered_map<std::string, std::wstring> m_missing;    // 無かったキーの代わりの文言（"[キー]"）
    };

    //! レジストリに置いた文言を返します。
    //! @param  [in] registry レジストリ
    //! @return 文言（コンテキストに無ければ空の文言。どのキーも "[キー]" になる）
    const Texts& GetTexts(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic

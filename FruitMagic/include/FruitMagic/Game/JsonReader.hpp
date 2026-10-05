//----------------------------------------------------------------------------
//! @file   JsonReader.hpp
//! @brief  定義データ（Assets/Data の JSON）を読むための共通の関数
//! @detail 任意項目の省略を許すため、cereal のアーカイブではなく cereal 同梱の rapidjson を直接使います。
//!         Read 系の関数は、項目が無い・型が違うときは書き込み先をそのまま残します（＝既定値のまま）。
//----------------------------------------------------------------------------
#pragma once
#include <cereal/external/rapidjson/document.h>

#include <hlsl++.h>

#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 定義データ（JSON）のファイルを読み込みます。先頭の UTF-8 の BOM は取り除きます。
    //! @param  [in] path 読み込むファイル
    //! @return 中身。読めなかった場合は空文字列
    //! @note   メモ帳などで保存すると BOM が付くことがあり、JSON の解析器はそれを読めないため
    std::string ReadDataText(const std::string& path);

    //! 文字列を UTF-8 から表示用のワイド文字列へ変換します。
    //! @param  [in] utf8 UTF-8 の文字列
    //! @return ワイド文字列
    std::wstring Utf8ToWide(const std::string& utf8);
}    // namespace FruitMagic

// 名前空間 : FruitMagic::Json
namespace FruitMagic::Json {
    namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

    using Value    = rj::Value;       //!< JSON の値
    using Document = rj::Document;    //!< JSON の文書（ルート）

    //! JSON ファイルを読み込んで解析します。読めない・壊れている・ルートがオブジェクトでないときは Warn を出します。
    //! @param  [in]  path  読み込むファイル
    //! @param  [out] doc   解析結果
    //! @param  [in]  owner ログに出す読み込み元の名前（"EconomyConfig" など）
    //! @return 成功したら true
    bool ParseFile(const std::string& path, Document& doc, const std::string& owner);

    //! 子の値を探します。
    //! @param  [in] obj 探すオブジェクト
    //! @param  [in] key 項目名
    //! @return 見つかった値。無ければ nullptr
    const Value* Find(const Value& obj, const char* key);

    //! 子のオブジェクトを探します。
    //! @param  [in] obj 探すオブジェクト
    //! @param  [in] key 項目名
    //! @return 見つかったオブジェクト。無い・オブジェクトでなければ nullptr
    const Value* FindObject(const Value& obj, const char* key);

    //! 子の配列を探します。
    //! @param  [in] obj 探すオブジェクト
    //! @param  [in] key 項目名
    //! @return 見つかった配列。無い・配列でなければ nullptr
    const Value* FindArray(const Value& obj, const char* key);

    //! 項目があれば値を読み込みます（数値は小数を切り捨てて整数にする）。
    //! @param  [in]     obj 読み込み元のオブジェクト
    //! @param  [in]     key 項目名
    //! @param  [in,out] out 読み込み先（項目が無い・型が違うときはそのまま）
    //! @return 読み込んだら true
    bool Read(const Value& obj, const char* key, float& out);
    bool Read(const Value& obj, const char* key, int& out);
    bool Read(const Value& obj, const char* key, long long& out);
    bool Read(const Value& obj, const char* key, bool& out);
    bool Read(const Value& obj, const char* key, std::string& out);
    bool Read(const Value& obj, const char* key, std::wstring& out);

    //! 項目があれば {x,y} / {x,y,z} を読み込みます。足りない要素は元の値のまま残します。
    //! @param  [in]     obj 読み込み元のオブジェクト
    //! @param  [in]     key 項目名
    //! @param  [in,out] out 読み込み先
    //! @return 読み込んだら true
    bool ReadVec(const Value& obj, const char* key, hlslpp::float2& out);
    bool ReadVec(const Value& obj, const char* key, hlslpp::float3& out);

    //! 項目があれば色 {r,g,b} / {r,g,b,a} を読み込みます。足りない要素は元の値のまま残します。
    //! @param  [in]     obj 読み込み元のオブジェクト
    //! @param  [in]     key 項目名
    //! @param  [in,out] out 読み込み先
    //! @return 読み込んだら true
    bool ReadColor(const Value& obj, const char* key, hlslpp::float3& out);
    bool ReadColor(const Value& obj, const char* key, hlslpp::float4& out);

    //! 色の値 {r,g,b(,a)} そのものを読み込みます（配列の要素など、項目名の無い値用）。
    //! @param  [in]     value 色のオブジェクト
    //! @param  [in,out] out   読み込み先
    //! @return 読み込んだら true
    bool ReadColorValue(const Value& value, hlslpp::float3& out);
    bool ReadColorValue(const Value& value, hlslpp::float4& out);

    //! 項目があれば数値の配列を読み込みます（中身を置き換える）。
    //! @param  [in]     obj 読み込み元のオブジェクト
    //! @param  [in]     key 項目名
    //! @param  [in,out] out 読み込み先
    //! @return 読み込んだら true
    bool ReadArray(const Value& obj, const char* key, std::vector<float>& out);
}    // namespace FruitMagic::Json

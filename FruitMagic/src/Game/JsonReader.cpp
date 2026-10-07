//----------------------------------------------------------------------------
//! @file   JsonReader.cpp
//! @brief  定義データ（Assets/Data の JSON）を読むための共通の関数の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/error/en.h>

#include <filesystem>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //----------------------------------------------------------------------------
    //! 定義データ（JSON）のファイルを読み込みます。先頭の UTF-8 の BOM は取り除きます。
    //----------------------------------------------------------------------------
    std::string ReadDataText(const std::string& path) {
        std::string text = Tsukino::IO::FileSystem::ReadText(Tsukino::Core::Path(path));
        if(text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF && static_cast<unsigned char>(text[1]) == 0xBB &&
           static_cast<unsigned char>(text[2]) == 0xBF)
            text.erase(0, 3);
        return text;
    }

    //----------------------------------------------------------------------------
    //! 文字列を UTF-8 から表示用のワイド文字列へ変換します。
    //----------------------------------------------------------------------------
    std::wstring Utf8ToWide(const std::string& utf8) {
        return std::filesystem::path(std::u8string(utf8.begin(), utf8.end())).wstring();
    }
}    // namespace FruitMagic

// 名前空間 : FruitMagic::Json
namespace FruitMagic::Json {
    namespace {
        //--------------------------------------------------------------
        //! 項目の数値を探します。
        //! @param  [in] obj 探すオブジェクト
        //! @param  [in] key 項目名
        //! @return 数値の値。無い・数値でなければ nullptr
        //--------------------------------------------------------------
        const Value* FindNumber(const Value& obj, const char* key) {
            const Value* value = Find(obj, key);
            return (value && value->IsNumber()) ? value : nullptr;
        }

        //--------------------------------------------------------------
        //! オブジェクトの要素を名前の順に float へ読み込みます。
        //! @param  [in]     value 読み込み元のオブジェクト
        //! @param  [in]     names 要素の名前
        //! @param  [in]     count 要素の数
        //! @param  [in,out] out   読み込み先（足りない要素はそのまま）
        //! @return value がオブジェクトなら true
        //--------------------------------------------------------------
        bool ReadComponents(const Value& value, const char* const* names, int count, float* out) {
            if(!value.IsObject())
                return false;
            for(int i = 0; i < count; ++i)
                Read(value, names[i], out[i]);
            return true;
        }

        constexpr const char* kVecNames[]   = {"x", "y", "z"};
        constexpr const char* kColorNames[] = {"r", "g", "b", "a"};
    }    // namespace

    //----------------------------------------------------------------------------
    //! JSON ファイルを読み込んで解析します。
    //----------------------------------------------------------------------------
    bool ParseFile(const std::string& path, Document& doc, const std::string& owner) {
        const std::string text = ReadDataText(path);
        if(text.empty()) {
            Tsukino::Core::Log::Warn(owner + ": cannot read " + path + ". Using defaults.");
            return false;
        }

        doc.Parse(text.c_str());
        if(doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn(owner + ": invalid JSON in " + path + " (" +
                                     (doc.HasParseError() ? rj::GetParseError_En(doc.GetParseError()) : "root is not an object") + "). Using defaults.");
            return false;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! 子の値を探します。
    //----------------------------------------------------------------------------
    const Value* Find(const Value& obj, const char* key) {
        if(!obj.IsObject())
            return nullptr;
        auto it = obj.FindMember(key);
        return (it != obj.MemberEnd()) ? &it->value : nullptr;
    }

    //----------------------------------------------------------------------------
    //! 子のオブジェクトを探します。
    //----------------------------------------------------------------------------
    const Value* FindObject(const Value& obj, const char* key) {
        const Value* value = Find(obj, key);
        return (value && value->IsObject()) ? value : nullptr;
    }

    //----------------------------------------------------------------------------
    //! 子の配列を探します。
    //----------------------------------------------------------------------------
    const Value* FindArray(const Value& obj, const char* key) {
        const Value* value = Find(obj, key);
        return (value && value->IsArray()) ? value : nullptr;
    }

    //----------------------------------------------------------------------------
    //! 項目があれば値を読み込みます。
    //----------------------------------------------------------------------------
    bool Read(const Value& obj, const char* key, float& out) {
        const Value* value = FindNumber(obj, key);
        if(!value)
            return false;
        out = static_cast<float>(value->GetDouble());
        return true;
    }

    bool Read(const Value& obj, const char* key, int& out) {
        const Value* value = FindNumber(obj, key);
        if(!value)
            return false;
        out = static_cast<int>(value->GetDouble());
        return true;
    }

    bool Read(const Value& obj, const char* key, long long& out) {
        const Value* value = FindNumber(obj, key);
        if(!value)
            return false;
        out = value->IsInt64() ? value->GetInt64() : static_cast<long long>(value->GetDouble());
        return true;
    }

    bool Read(const Value& obj, const char* key, bool& out) {
        const Value* value = Find(obj, key);
        if(!value || !value->IsBool())
            return false;
        out = value->GetBool();
        return true;
    }

    bool Read(const Value& obj, const char* key, std::string& out) {
        const Value* value = Find(obj, key);
        if(!value || !value->IsString())
            return false;
        out = value->GetString();
        return true;
    }

    bool Read(const Value& obj, const char* key, std::wstring& out) {
        std::string text;
        if(!Read(obj, key, text))
            return false;
        out = Utf8ToWide(text);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 項目があれば {x,y} / {x,y,z} を読み込みます。
    //----------------------------------------------------------------------------
    bool ReadVec(const Value& obj, const char* key, hlslpp::float2& out) {
        const Value* value = FindObject(obj, key);
        float        v[2]  = {out.x, out.y};
        if(!value || !ReadComponents(*value, kVecNames, 2, v))
            return false;
        out = hlslpp::float2(v[0], v[1]);
        return true;
    }

    bool ReadVec(const Value& obj, const char* key, hlslpp::float3& out) {
        const Value* value = FindObject(obj, key);
        float        v[3]  = {out.x, out.y, out.z};
        if(!value || !ReadComponents(*value, kVecNames, 3, v))
            return false;
        out = hlslpp::float3(v[0], v[1], v[2]);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 項目があれば、数の配列を count 個まで読み込みます。
    //----------------------------------------------------------------------------
    bool ReadFloats(const Value& obj, const char* key, float* out, int count) {
        auto it = obj.FindMember(key);
        if(it == obj.MemberEnd() || !it->value.IsArray())
            return false;
        int index = 0;
        for(const Value& item : it->value.GetArray()) {
            if(index >= count)
                break;
            if(item.IsNumber())
                out[index] = item.GetFloat();
            ++index;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! 項目があれば色 {r,g,b} / {r,g,b,a} を読み込みます。
    //----------------------------------------------------------------------------
    bool ReadColor(const Value& obj, const char* key, hlslpp::float3& out) {
        const Value* value = FindObject(obj, key);
        return value && ReadColorValue(*value, out);
    }

    bool ReadColor(const Value& obj, const char* key, hlslpp::float4& out) {
        const Value* value = FindObject(obj, key);
        return value && ReadColorValue(*value, out);
    }

    //----------------------------------------------------------------------------
    //! 色の値 {r,g,b(,a)} そのものを読み込みます。
    //----------------------------------------------------------------------------
    bool ReadColorValue(const Value& value, hlslpp::float3& out) {
        float c[3] = {out.x, out.y, out.z};
        if(!ReadComponents(value, kColorNames, 3, c))
            return false;
        out = hlslpp::float3(c[0], c[1], c[2]);
        return true;
    }

    bool ReadColorValue(const Value& value, hlslpp::float4& out) {
        float c[4] = {out.x, out.y, out.z, out.w};
        if(!ReadComponents(value, kColorNames, 4, c))
            return false;
        out = hlslpp::float4(c[0], c[1], c[2], c[3]);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 項目があれば数値の配列を読み込みます。
    //----------------------------------------------------------------------------
    bool ReadArray(const Value& obj, const char* key, std::vector<float>& out) {
        const Value* value = FindArray(obj, key);
        if(!value)
            return false;
        out.clear();
        for(const Value& item : value->GetArray()) {
            if(item.IsNumber())
                out.push_back(static_cast<float>(item.GetDouble()));
        }
        return true;
    }
}    // namespace FruitMagic::Json

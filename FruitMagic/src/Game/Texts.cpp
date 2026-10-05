//----------------------------------------------------------------------------
//! @file   Texts.cpp
//! @brief  画面に出す文言の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/Texts.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //--------------------------------------------------------------
        //! オブジェクトの文字列を "親.子" のキーで集めます（入れ子は何段でもよい）。
        //! @param  [in]     obj    集めるオブジェクト
        //! @param  [in]     prefix キーの頭（ルートでは空）
        //! @param  [in,out] out    集めた文言
        //--------------------------------------------------------------
        void Collect(const Json::Value& obj, const std::string& prefix, std::unordered_map<std::string, std::wstring>& out) {
            for(auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
                const std::string key = prefix.empty() ? std::string(it->name.GetString()) : prefix + "." + it->name.GetString();
                if(it->value.IsString())
                    out[key] = Utf8ToWide(it->value.GetString());
                else if(it->value.IsObject())
                    Collect(it->value, key, out);
            }
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! レジストリに置いた文言を返します。
    //----------------------------------------------------------------------------
    const Texts& GetTexts(Tsukino::ECS::Registry& registry) {
        static const Texts kEmpty;
        return registry.HasContext<Texts>() ? registry.GetContext<Texts>() : kEmpty;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool Texts::Load(const std::string& path) {
        m_texts.clear();
        m_missing.clear();

        Json::Document doc;
        if(!Json::ParseFile(path, doc, "Texts"))
            return false;

        Collect(doc, "", m_texts);
        Tsukino::Core::Log::Info("Texts: loaded " + std::to_string(m_texts.size()) + " texts.");
        return true;
    }

    //----------------------------------------------------------------------------
    //! 文言を返します。
    //----------------------------------------------------------------------------
    const std::wstring& Texts::Get(const std::string& key) const {
        auto it = m_texts.find(key);
        if(it != m_texts.end())
            return it->second;

        auto missing = m_missing.find(key);
        if(missing == m_missing.end()) {
            Tsukino::Core::Log::Warn("Texts: \"" + key + "\" is not in Texts.json.");
            missing = m_missing.emplace(key, L"[" + Utf8ToWide(key) + L"]").first;
        }
        return missing->second;
    }

    //----------------------------------------------------------------------------
    //! 文言の {名前} に値を差し込んで返します。
    //----------------------------------------------------------------------------
    std::wstring Texts::Format(const std::string& key, std::initializer_list<Arg> args) const {
        std::wstring text = Get(key);
        for(const Arg& arg : args) {
            const std::wstring token = L"{" + Utf8ToWide(arg.first) + L"}";
            for(size_t pos = text.find(token); pos != std::wstring::npos; pos = text.find(token, pos + arg.second.size()))
                text.replace(pos, token.size(), arg.second);
        }
        return text;
    }
}    // namespace FruitMagic

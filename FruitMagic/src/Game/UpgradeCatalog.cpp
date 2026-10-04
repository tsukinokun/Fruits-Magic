//----------------------------------------------------------------------------
//! @file   UpgradeCatalog.cpp
//! @brief  台の強化の定義データの読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>

#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

        //--------------------------------------------------------------
        //! 項目があれば数値を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（項目が無ければそのまま）
        //--------------------------------------------------------------
        template <class T>
        void ReadNumber(const rj::Value& obj, const char* key, T& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsNumber())
                out = static_cast<T>(it->value.GetDouble());
        }

        //--------------------------------------------------------------
        //! 項目があれば文字列を表示用に読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（項目が無ければそのまま）
        //--------------------------------------------------------------
        void ReadText(const rj::Value& obj, const char* key, std::wstring& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsString())
                out = Utf8ToWide(it->value.GetString());
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! レベルでの効果の値を返します。
    //----------------------------------------------------------------------------
    float UpgradeDef::ValueAt(int level) const {
        if(level <= 0 || levels.empty())
            return baseValue;
        return levels[std::min(level, MaxLevel()) - 1].value;
    }

    //----------------------------------------------------------------------------
    //! 効果の値を表示用の文字列にします。
    //----------------------------------------------------------------------------
    std::wstring UpgradeDef::FormatValue(float value) const {
        if(value == 0.0f && !zeroText.empty())
            return zeroText;

        // 整数ならそのまま、そうでなければ小数1桁
        std::wstring number;
        if(std::abs(value - std::round(value)) < 0.001f) {
            number = std::to_wstring(static_cast<long long>(std::lround(value)));
        } else {
            const long long tenths = std::llround(value * 10.0f);
            number                 = std::to_wstring(tenths / 10) + L"." + std::to_wstring(std::llabs(tenths % 10));
        }

        std::wstring text = format;
        const size_t pos  = text.find(L"{v}");
        if(pos == std::wstring::npos)
            return number + text;
        return text.replace(pos, 3, number);
    }

    //----------------------------------------------------------------------------
    //! 定義データを読み込みます。
    //----------------------------------------------------------------------------
    bool UpgradeCatalog::Load(const std::string& path) {
        m_upgrades.clear();

        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        auto upgrades = (text.empty() || doc.HasParseError() || !doc.IsObject()) ? doc.MemberEnd() : doc.FindMember("upgrades");
        if(upgrades == doc.MemberEnd() || !upgrades->value.IsArray()) {
            Tsukino::Core::Log::Warn("UpgradeCatalog: cannot read \"upgrades\" from " + path + ". No upgrades are available.");
            return false;
        }

        for(const rj::Value& u : upgrades->value.GetArray()) {
            if(!u.IsObject())
                continue;

            UpgradeDef def;
            auto       id = u.FindMember("id");
            if(id == u.MemberEnd() || !id->value.IsString()) {
                Tsukino::Core::Log::Warn("UpgradeCatalog: an upgrade without \"id\" was skipped.");
                continue;
            }
            def.id   = id->value.GetString();
            def.name = Utf8ToWide(def.id);
            ReadText(u, "name", def.name);
            ReadText(u, "description", def.description);
            ReadText(u, "format", def.format);
            ReadText(u, "zeroText", def.zeroText);
            ReadNumber(u, "base", def.baseValue);

            auto levels = u.FindMember("levels");
            if(levels != u.MemberEnd() && levels->value.IsArray()) {
                for(const rj::Value& l : levels->value.GetArray()) {
                    if(!l.IsObject())
                        continue;
                    UpgradeLevel level;
                    ReadNumber(l, "coins", level.coins);
                    ReadNumber(l, "fruit", level.fruit);
                    ReadNumber(l, "value", level.value);
                    level.coins = std::max(0, level.coins);
                    level.fruit = std::max(0, level.fruit);
                    def.levels.push_back(level);
                }
            }
            if(def.levels.empty()) {
                Tsukino::Core::Log::Warn("UpgradeCatalog: upgrade \"" + def.id + "\" has no \"levels\" and was skipped.");
                continue;
            }
            if(Find(def.id) != nullptr) {
                Tsukino::Core::Log::Warn("UpgradeCatalog: duplicate upgrade \"" + def.id + "\" was skipped.");
                continue;
            }

            m_upgrades.push_back(def);
        }

        Tsukino::Core::Log::Info("UpgradeCatalog: loaded " + std::to_string(m_upgrades.size()) + " upgrades.");
        return !m_upgrades.empty();
    }

    //----------------------------------------------------------------------------
    //! id から強化を探します。
    //----------------------------------------------------------------------------
    const UpgradeDef* UpgradeCatalog::Find(const std::string& id) const {
        for(const UpgradeDef& def : m_upgrades) {
            if(def.id == id)
                return &def;
        }
        return nullptr;
    }
}    // namespace FruitMagic

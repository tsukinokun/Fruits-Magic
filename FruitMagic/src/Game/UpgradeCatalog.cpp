//----------------------------------------------------------------------------
//! @file   UpgradeCatalog.cpp
//! @brief  台の強化の定義データの読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/UpgradeCatalog.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic
namespace FruitMagic {
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

        Json::Document     doc;
        const Json::Value* upgrades = Json::ParseFile(path, doc, "UpgradeCatalog") ? Json::FindArray(doc, "upgrades") : nullptr;
        if(!upgrades) {
            Tsukino::Core::Log::Warn("UpgradeCatalog: cannot read \"upgrades\" from " + path + ". No upgrades are available.");
            return false;
        }

        for(const Json::Value& u : upgrades->GetArray()) {
            if(!u.IsObject())
                continue;

            UpgradeDef def;
            if(!Json::Read(u, "id", def.id)) {
                Tsukino::Core::Log::Warn("UpgradeCatalog: an upgrade without \"id\" was skipped.");
                continue;
            }
            def.name = Utf8ToWide(def.id);
            Json::Read(u, "name", def.name);
            Json::Read(u, "description", def.description);
            Json::Read(u, "format", def.format);
            Json::Read(u, "zeroText", def.zeroText);
            Json::Read(u, "base", def.baseValue);

            if(const Json::Value* levels = Json::FindArray(u, "levels")) {
                for(const Json::Value& l : levels->GetArray()) {
                    if(!l.IsObject())
                        continue;
                    UpgradeLevel level;
                    Json::Read(l, "coins", level.coins);
                    Json::Read(l, "fruit", level.fruit);
                    Json::Read(l, "value", level.value);
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

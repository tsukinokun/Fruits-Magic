//----------------------------------------------------------------------------
//! @file   MagicCatalog.cpp
//! @brief  魔法の定義データの読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/MagicCatalog.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;
    }    // namespace

    //----------------------------------------------------------------------------
    //! 効果の数値を返します。
    //----------------------------------------------------------------------------
    float MagicDef::Param(const std::string& key, float fallback) const {
        auto it = params.find(key);
        return (it != params.end()) ? it->second : fallback;
    }

    //----------------------------------------------------------------------------
    //! 定義データを読み込みます。
    //----------------------------------------------------------------------------
    bool MagicCatalog::Load(const std::string& path) {
        m_magics.clear();

        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn("MagicCatalog: cannot read " + path + ".");
            return false;
        }

        auto magics = doc.FindMember("magics");
        if(magics == doc.MemberEnd() || !magics->value.IsArray()) {
            Tsukino::Core::Log::Warn("MagicCatalog: Magic.json has no \"magics\" array.");
            return false;
        }

        for(const rj::Value& m : magics->value.GetArray()) {
            if(!m.IsObject())
                continue;

            MagicDef def;
            auto     id = m.FindMember("id");
            if(id == m.MemberEnd() || !id->value.IsString()) {
                Tsukino::Core::Log::Warn("MagicCatalog: a magic without \"id\" was skipped.");
                continue;
            }
            def.id = id->value.GetString();

            auto name = m.FindMember("name");
            def.name  = Utf8ToWide((name != m.MemberEnd() && name->value.IsString()) ? std::string(name->value.GetString()) : def.id);

            auto cost = m.FindMember("cost");
            if(cost != m.MemberEnd() && cost->value.IsNumber())
                def.cost = static_cast<int>(cost->value.GetDouble());

            auto slot = m.FindMember("key");
            if(slot != m.MemberEnd() && slot->value.IsNumber())
                def.slot = static_cast<int>(slot->value.GetDouble());

            auto unlockZukan = m.FindMember("unlockZukan");
            if(unlockZukan != m.MemberEnd() && unlockZukan->value.IsNumber())
                def.unlockZukan = std::max(0, static_cast<int>(unlockZukan->value.GetDouble()));

            auto params = m.FindMember("params");
            if(params != m.MemberEnd() && params->value.IsObject()) {
                for(auto p = params->value.MemberBegin(); p != params->value.MemberEnd(); ++p) {
                    if(p->value.IsNumber())
                        def.params[p->name.GetString()] = static_cast<float>(p->value.GetDouble());
                }
            }

            if(def.slot < 1 || def.slot > kMagicSlotCount || FindBySlot(def.slot) >= 0) {
                Tsukino::Core::Log::Warn("MagicCatalog: magic \"" + def.id + "\" has an invalid or duplicate key " + std::to_string(def.slot) + " and was skipped.");
                continue;
            }

            m_magics.push_back(def);
        }

        Tsukino::Core::Log::Info("MagicCatalog: loaded " + std::to_string(m_magics.size()) + " magics.");
        return !m_magics.empty();
    }

    //----------------------------------------------------------------------------
    //! 魔法が解放済みかを返します。
    //----------------------------------------------------------------------------
    bool MagicCatalog::IsUnlocked(int magicIndex, int registeredCount) const {
        if(magicIndex < 0 || magicIndex >= static_cast<int>(m_magics.size()))
            return false;
        return registeredCount >= m_magics[magicIndex].unlockZukan;
    }

    //----------------------------------------------------------------------------
    //! 添字の魔法が指定の id かを返します。
    //----------------------------------------------------------------------------
    bool MagicCatalog::IsMagic(int magicIndex, const char* id) const {
        return magicIndex >= 0 && magicIndex < static_cast<int>(m_magics.size()) && m_magics[magicIndex].id == id;
    }

    //----------------------------------------------------------------------------
    //! 枠に割り当てられた魔法の添字を返します。
    //----------------------------------------------------------------------------
    int MagicCatalog::FindBySlot(int slot) const {
        for(size_t i = 0; i < m_magics.size(); ++i) {
            if(m_magics[i].slot == slot)
                return static_cast<int>(i);
        }
        return -1;
    }
}    // namespace FruitMagic

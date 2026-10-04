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

        const std::string text = Tsukino::IO::FileSystem::ReadText(Tsukino::Core::Path(path));
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

            auto unlocked = m.FindMember("unlocked");
            if(unlocked != m.MemberEnd() && unlocked->value.IsBool())
                def.unlocked = unlocked->value.GetBool();

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

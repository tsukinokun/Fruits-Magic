//----------------------------------------------------------------------------
//! @file   MagicCatalog.cpp
//! @brief  魔法の定義データの読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/MagicCatalog.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
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

        Json::Document doc;
        if(!Json::ParseFile(path, doc, "MagicCatalog"))
            return false;

        const Json::Value* magics = Json::FindArray(doc, "magics");
        if(!magics) {
            Tsukino::Core::Log::Warn("MagicCatalog: Magic.json has no \"magics\" array.");
            return false;
        }

        for(const Json::Value& m : magics->GetArray()) {
            if(!m.IsObject())
                continue;

            MagicDef def;
            if(!Json::Read(m, "id", def.id)) {
                Tsukino::Core::Log::Warn("MagicCatalog: a magic without \"id\" was skipped.");
                continue;
            }
            def.name = Utf8ToWide(def.id);
            Json::Read(m, "name", def.name);
            Json::Read(m, "cost", def.cost);
            Json::Read(m, "key", def.slot);
            Json::Read(m, "unlockZukan", def.unlockZukan);
            def.unlockZukan = std::max(0, def.unlockZukan);

            if(const Json::Value* params = Json::FindObject(m, "params")) {
                for(auto p = params->MemberBegin(); p != params->MemberEnd(); ++p) {
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

//----------------------------------------------------------------------------
//! @file   SouvenirCatalog.cpp
//! @brief  おみやげの定義データの読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/SouvenirCatalog.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <filesystem>
#include <unordered_set>

// 名前空間 : FruitMagic
namespace FruitMagic {

    namespace {
        //! 効果の種類の名前（JSON の "effect"）です。
        struct EffectName {
            const char*    name;
            SouvenirEffect effect;
        };
        constexpr EffectName kEffects[] = {
            {"fruitValue", SouvenirEffect::FruitValue},       {"mana", SouvenirEffect::Mana},
            {"variant", SouvenirEffect::Variant},             {"seedGain", SouvenirEffect::SeedGain},
            {"rouletteHit", SouvenirEffect::RouletteHit},     {"magicDuration", SouvenirEffect::MagicDuration},
            {"offlineHours", SouvenirEffect::OfflineHours},   {"startCoins", SouvenirEffect::StartCoins},
        };

        // 数の配列を読む（無ければそのまま）
        void ReadFloats(const Json::Value& obj, const char* key, std::vector<float>& out) {
            const Json::Value* list = Json::FindArray(obj, key);
            if(!list)
                return;
            out.clear();
            for(const Json::Value& v : list->GetArray()) {
                if(v.IsNumber())
                    out.push_back(std::max(0.0f, v.GetFloat()));
            }
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 定義データともらい方の設定を読み込みます。
    //----------------------------------------------------------------------------
    bool SouvenirCatalog::Load(const std::string& dataRoot) {
        m_souvenirs.clear();
        const std::filesystem::path root(dataRoot);

        //--------------------------------------------------------------
        // もらい方の設定（無ければ既定値）
        //--------------------------------------------------------------
        Json::Document config;
        if(Json::ParseFile((root / "Souvenir.json").string(), config, "SouvenirCatalog")) {
            Json::Read(config, "valuePerExtra", valuePerExtra);
            Json::Read(config, "maxPerMove", maxPerMove);
            ReadFloats(config, "rarityWeights", rarityWeights);
            ReadFloats(config, "rarityBoost", rarityBoost);
            Json::Read(config, "coinsForFullBoost", coinsForFullBoost);
        }
        valuePerExtra     = std::max(1.0f, valuePerExtra);
        maxPerMove        = std::max(1, maxPerMove);
        coinsForFullBoost = std::max(1.0f, coinsForFullBoost);

        //--------------------------------------------------------------
        // おみやげ（フォルダ内の *.json をすべて読む。列挙順は環境依存なので名前順に並べてから）
        //--------------------------------------------------------------
        const std::filesystem::path dir = root / "Souvenirs";
        std::error_code             ec;
        if(!std::filesystem::is_directory(dir, ec)) {
            Tsukino::Core::Log::Warn("SouvenirCatalog: souvenir folder not found: " + dir.generic_string());
            return false;
        }
        std::vector<std::filesystem::path> files;
        for(const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if(entry.is_regular_file() && entry.path().extension() == ".json")
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());

        std::unordered_set<std::string> ids;
        for(const std::filesystem::path& file : files) {
            Json::Document doc;
            if(!Json::ParseFile(file.string(), doc, "SouvenirCatalog"))
                continue;

            SouvenirDef def;
            def.id = file.stem().string();
            Json::Read(doc, "id", def.id);
            if(!ids.insert(def.id).second) {
                Tsukino::Core::Log::Warn("SouvenirCatalog: duplicate souvenir \"" + def.id + "\" was skipped.");
                continue;
            }
            def.name = Utf8ToWide(def.id);
            Json::Read(doc, "name", def.name);
            Json::Read(doc, "model", def.model);
            Json::ReadVec(doc, "modelRotation", def.modelRotation);
            Json::Read(doc, "rarity", def.rarity);
            Json::Read(doc, "perLevel", def.perLevel);
            Json::Read(doc, "maxLevel", def.maxLevel);
            def.rarity   = std::clamp(def.rarity, 0, 3);
            def.maxLevel = std::max(1, def.maxLevel);

            std::string effect;
            Json::Read(doc, "effect", effect);
            const auto found = std::find_if(std::begin(kEffects), std::end(kEffects), [&](const EffectName& e) { return effect == e.name; });
            if(found == std::end(kEffects)) {
                Tsukino::Core::Log::Warn("SouvenirCatalog: souvenir \"" + def.id + "\" has unknown effect \"" + effect + "\" and was skipped.");
                continue;
            }
            def.effect = found->effect;
            if(def.model.empty()) {
                Tsukino::Core::Log::Warn("SouvenirCatalog: souvenir \"" + def.id + "\" has no \"model\" and was skipped.");
                continue;
            }
            m_souvenirs.push_back(def);
        }

        // 図鑑の並び: レア度 → id（id は重複しないので、毎回同じ並びになる。
        // stable_sort は hlsl++ の型を持つ構造体の一時バッファで拡張アラインメントのエラーになるため使わない）
        std::sort(m_souvenirs.begin(), m_souvenirs.end(), [](const SouvenirDef& a, const SouvenirDef& b) {
            return (a.rarity != b.rarity) ? a.rarity < b.rarity : a.id < b.id;
        });

        Tsukino::Core::Log::Info("SouvenirCatalog: loaded " + std::to_string(m_souvenirs.size()) + " souvenirs.");
        return !m_souvenirs.empty();
    }

    //----------------------------------------------------------------------------
    //! id からおみやげの添字を探します。
    //----------------------------------------------------------------------------
    int SouvenirCatalog::FindIndex(const std::string& id) const {
        for(size_t i = 0; i < m_souvenirs.size(); ++i) {
            if(m_souvenirs[i].id == id)
                return static_cast<int>(i);
        }
        return -1;
    }
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   FruitCatalog.cpp
//! @brief  果物とランクの定義データの読み込み
//! @detail 任意項目の省略を許すため、cereal のアーカイブではなく cereal 同梱の rapidjson を直接使います。
//----------------------------------------------------------------------------
#include <FruitMagic/Game/FruitCatalog.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <filesystem>
#include <unordered_set>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = Json::rj;

        //--------------------------------------------------------------
        //! JSON ファイルを読み込んで解析します。
        //! @param  [in]  path 読み込むファイル
        //! @param  [out] doc  解析結果
        //! @return 成功したら true（失敗はログに出す）
        //--------------------------------------------------------------
        bool ParseJsonFile(const std::filesystem::path& path, rj::Document& doc) {
            return Json::ParseFile(path.generic_string(), doc, "FruitCatalog");
        }

        //--------------------------------------------------------------
        //! 任意項目の文字列を読みます。
        //! @param  [in] obj      読み込み元のオブジェクト
        //! @param  [in] key      項目名
        //! @param  [in] fallback 項目が無いときの値
        //! @return 読み込んだ値
        //--------------------------------------------------------------
        std::string GetString(const rj::Value& obj, const char* key, const std::string& fallback) {
            std::string value = fallback;
            Json::Read(obj, key, value);
            return value;
        }

        //--------------------------------------------------------------
        //! 任意項目の数値を読みます。
        //! @param  [in] obj      読み込み元のオブジェクト
        //! @param  [in] key      項目名
        //! @param  [in] fallback 項目が無いときの値
        //! @return 読み込んだ値
        //--------------------------------------------------------------
        float GetFloat(const rj::Value& obj, const char* key, float fallback) {
            float value = fallback;
            Json::Read(obj, key, value);
            return value;
        }

        //--------------------------------------------------------------
        //! 任意項目の整数を読みます。
        //! @param  [in] obj      読み込み元のオブジェクト
        //! @param  [in] key      項目名
        //! @param  [in] fallback 項目が無いときの値
        //! @return 読み込んだ値
        //--------------------------------------------------------------
        int GetInt(const rj::Value& obj, const char* key, int fallback) {
            int value = fallback;
            Json::Read(obj, key, value);
            return value;
        }

        //--------------------------------------------------------------
        //! 任意項目の3要素（{x,y,z} または {r,g,b}）を読みます。
        //! @param  [in] obj      読み込み元のオブジェクト
        //! @param  [in] key      項目名
        //! @param  [in] names    3要素の名前（"xyz" や "rgb"）
        //! @param  [in] fallback 項目が無いときの値
        //! @return 読み込んだ値
        //--------------------------------------------------------------
        hlslpp::float3 GetFloat3(const rj::Value& obj, const char* key, const char* names, const hlslpp::float3& fallback) {
            hlslpp::float3 value = fallback;
            if(names[0] == 'r')
                Json::ReadColor(obj, key, value);
            else
                Json::ReadVec(obj, key, value);
            return value;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 当たり判定の外接する高さの半分を返します。
    //----------------------------------------------------------------------------
    float FruitDef::HalfHeightOfBounds() const {
        switch(shape) {
            case FruitShape::Box: return halfExtent.y;
            case FruitShape::Capsule: return halfHeight + radius;
            case FruitShape::Sphere:
            default: return radius;
        }
    }

    //----------------------------------------------------------------------------
    //! 定義データを読み込みます。不正な果物は警告を出して読み飛ばします。
    //----------------------------------------------------------------------------
    bool FruitCatalog::Load(const std::string& dataRoot) {
        m_ranks.clear();
        m_fruits.clear();

        const std::filesystem::path root(dataRoot);

        //--------------------------------------------------------------
        // ランク
        //--------------------------------------------------------------
        {
            rj::Document doc;
            if(!ParseJsonFile(root / "FruitRanks.json", doc))
                return false;

            auto ranks = doc.FindMember("ranks");
            if(ranks == doc.MemberEnd() || !ranks->value.IsArray()) {
                Tsukino::Core::Log::Warn("FruitCatalog: FruitRanks.json has no \"ranks\" array.");
                return false;
            }

            for(const rj::Value& r : ranks->value.GetArray()) {
                if(!r.IsObject())
                    continue;
                FruitRank rank;
                rank.id    = GetString(r, "id", "");
                rank.name  = Utf8ToWide(GetString(r, "name", rank.id));
                rank.order = GetInt(r, "order", static_cast<int>(m_ranks.size()));
                if(rank.id.empty()) {
                    Tsukino::Core::Log::Warn("FruitCatalog: a rank without \"id\" was skipped.");
                    continue;
                }
                m_ranks.push_back(rank);
            }

            std::stable_sort(m_ranks.begin(), m_ranks.end(), [](const FruitRank& a, const FruitRank& b) { return a.order < b.order; });
        }

        //--------------------------------------------------------------
        // 果物（フォルダ内の *.json をすべて読む）
        //--------------------------------------------------------------
        const std::filesystem::path fruitDir = root / "Fruits";
        std::error_code             ec;
        if(!std::filesystem::is_directory(fruitDir, ec)) {
            Tsukino::Core::Log::Warn("FruitCatalog: fruit folder not found: " + fruitDir.generic_string());
            return false;
        }

        // ファイルの列挙順は環境依存なので、名前順に並べてから読む（読み込み結果を毎回同じにする）
        std::vector<std::filesystem::path> files;
        for(const auto& entry : std::filesystem::directory_iterator(fruitDir, ec)) {
            if(entry.is_regular_file() && entry.path().extension() == ".json")
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());

        std::unordered_set<std::string> ids;
        for(const std::filesystem::path& file : files) {
            rj::Document doc;
            if(!ParseJsonFile(file, doc))
                continue;

            const std::string where = file.filename().generic_string();

            FruitDef def;
            def.id = GetString(doc, "id", file.stem().generic_string());
            if(!ids.insert(def.id).second) {
                Tsukino::Core::Log::Warn("FruitCatalog: duplicate fruit id \"" + def.id + "\" in " + where + " was skipped.");
                continue;
            }

            def.name = Utf8ToWide(GetString(doc, "name", def.id));

            const std::string rankId = GetString(doc, "rank", "");
            auto rankIt = std::find_if(m_ranks.begin(), m_ranks.end(), [&](const FruitRank& r) { return r.id == rankId; });
            if(rankIt == m_ranks.end()) {
                Tsukino::Core::Log::Warn("FruitCatalog: unknown rank \"" + rankId + "\" in " + where + " was skipped.");
                continue;
            }
            def.rankIndex = static_cast<int>(rankIt - m_ranks.begin());

            const std::string shape = GetString(doc, "shape", "sphere");
            if(shape == "sphere") {
                def.shape = FruitShape::Sphere;
            } else if(shape == "box") {
                def.shape = FruitShape::Box;
            } else if(shape == "capsule") {
                def.shape = FruitShape::Capsule;
            } else {
                Tsukino::Core::Log::Warn("FruitCatalog: unknown shape \"" + shape + "\" in " + where + " was skipped.");
                continue;
            }

            def.radius          = GetFloat(doc, "radius", def.radius);
            def.halfHeight      = GetFloat(doc, "halfHeight", def.halfHeight);
            def.halfExtent      = GetFloat3(doc, "halfExtent", "xyz", def.halfExtent);
            def.mass            = GetFloat(doc, "mass", def.mass);
            def.value           = GetInt(doc, "value", def.value);
            def.mana            = GetInt(doc, "mana", def.mana);
            def.spawnWeight     = GetFloat(doc, "spawnWeight", def.spawnWeight);
            def.unlockTreeLevel = GetInt(doc, "unlockTreeLevel", def.unlockTreeLevel);
            def.color           = GetFloat3(doc, "color", "rgb", def.color);
            // 色違いの色。省略時は通常の色の RGB を回して、同じ明るさの別の色にする
            def.shinyColor      = GetFloat3(doc, "shinyColor", "rgb", hlslpp::float3(def.color.z, def.color.x, def.color.y));
            def.modelPath       = GetString(doc, "model", def.shape == FruitShape::Box ? AssetPaths::kBlockModel : AssetPaths::kBallModel);
            def.customModel     = doc.HasMember("model");
            def.modelRotation   = GetFloat3(doc, "modelRotation", "xyz", def.modelRotation);
            def.modelScale      = GetFloat(doc, "modelScale", def.modelScale);
            if(auto tint = doc.FindMember("tintBase"); tint != doc.MemberEnd() && tint->value.IsBool())
                def.tintBase = tint->value.GetBool();

            // 飾りのパーツ（任意）。形が分からないものは読み飛ばす
            auto parts = doc.FindMember("parts");
            if(parts != doc.MemberEnd() && parts->value.IsArray()) {
                for(const rj::Value& p : parts->value.GetArray()) {
                    if(!p.IsObject())
                        continue;
                    FruitPart part;
                    const std::string partShape = GetString(p, "shape", "box");
                    if(partShape == "sphere") {
                        part.shape = FruitPartShape::Sphere;
                    } else if(partShape == "box") {
                        part.shape = FruitPartShape::Box;
                    } else {
                        Tsukino::Core::Log::Warn("FruitCatalog: unknown part shape \"" + partShape + "\" in " + where + " was skipped.");
                        continue;
                    }
                    part.offset   = GetFloat3(p, "offset", "xyz", part.offset);
                    part.size     = GetFloat3(p, "size", "xyz", part.size);
                    part.rotation = GetFloat3(p, "rotation", "xyz", part.rotation);
                    part.color    = GetFloat3(p, "color", "rgb", part.color);
                    part.glow     = GetFloat(p, "glow", part.glow);
                    def.parts.push_back(part);
                }
            }

            if(def.radius <= 0.0f || def.mass <= 0.0f || def.halfExtent.x <= 0.0f || def.halfExtent.y <= 0.0f || def.halfExtent.z <= 0.0f) {
                Tsukino::Core::Log::Warn("FruitCatalog: non-positive size or mass in " + where + " was skipped.");
                continue;
            }

            m_fruits.push_back(def);
        }

        // ランク順、同じランクの中は id 順（図鑑の並びにそのまま使える）。
        // id は重複を除いてあるので順序は一意に決まり、stable_sort は要らない
        // （FruitDef は16バイト境界の hlslpp::float3 を持つため、MSVC の stable_sort の一時領域が作れない）
        std::sort(m_fruits.begin(), m_fruits.end(), [](const FruitDef& a, const FruitDef& b) {
            return (a.rankIndex != b.rankIndex) ? (a.rankIndex < b.rankIndex) : (a.id < b.id);
        });

        Tsukino::Core::Log::Info("FruitCatalog: loaded " + std::to_string(m_fruits.size()) + " fruits, " + std::to_string(m_ranks.size()) + " ranks.");
        return !m_ranks.empty() && !m_fruits.empty();
    }

    //----------------------------------------------------------------------------
    //! id から果物の添字を探します。
    //----------------------------------------------------------------------------
    int FruitCatalog::FindIndex(const std::string& id) const {
        for(size_t i = 0; i < m_fruits.size(); ++i) {
            if(m_fruits[i].id == id)
                return static_cast<int>(i);
        }
        return -1;
    }

    //----------------------------------------------------------------------------
    //! 指定の果樹の段階で出現できる果物から、出現の重みで1つ選びます。
    //----------------------------------------------------------------------------
    std::vector<int> FruitCatalog::SpawnableFruits(int treeLevel) const {
        std::vector<int> result;
        for(size_t i = 0; i < m_fruits.size(); ++i) {
            if(m_fruits[i].unlockTreeLevel <= treeLevel && m_fruits[i].spawnWeight > 0.0f)
                result.push_back(static_cast<int>(i));
        }
        return result;
    }

    //----------------------------------------------------------------------------
    //! 指定の果樹の段階で出現できる果物から、出現の重みで1つ選びます。
    //----------------------------------------------------------------------------
    int FruitCatalog::PickSpawnable(int treeLevel, std::mt19937& rng) const {
        float total = 0.0f;
        for(const FruitDef& def : m_fruits) {
            if(def.unlockTreeLevel <= treeLevel && def.spawnWeight > 0.0f)
                total += def.spawnWeight;
        }
        if(total <= 0.0f)
            return -1;

        float pick = std::uniform_real_distribution<float>(0.0f, total)(rng);
        for(size_t i = 0; i < m_fruits.size(); ++i) {
            const FruitDef& def = m_fruits[i];
            if(def.unlockTreeLevel > treeLevel || def.spawnWeight <= 0.0f)
                continue;
            pick -= def.spawnWeight;
            if(pick <= 0.0f)
                return static_cast<int>(i);
        }

        // 浮動小数の誤差で最後まで残った場合は、出現できる最後の果物
        for(size_t i = m_fruits.size(); i-- > 0;) {
            if(m_fruits[i].unlockTreeLevel <= treeLevel && m_fruits[i].spawnWeight > 0.0f)
                return static_cast<int>(i);
        }
        return -1;
    }
}    // namespace FruitMagic

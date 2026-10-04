//----------------------------------------------------------------------------
//! @file   SaveData.cpp
//! @brief  セーブデータの保存と読み込みの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/SaveData.hpp>

#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/prettywriter.h>
#include <cereal/external/rapidjson/stringbuffer.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

// 名前空間 : FruitMagic::SaveData
namespace FruitMagic::SaveData {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

        //! @brief セーブデータの形式の版。形式を変えたら上げ、読み込み側で古い版を変換する
        constexpr int kVersion = 1;

        //--------------------------------------------------------------
        //! 項目があれば整数を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（項目が無ければそのまま）
        //--------------------------------------------------------------
        template <class T>
        void ReadInteger(const rj::Value& obj, const char* key, T& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsInt64())
                out = static_cast<T>(it->value.GetInt64());
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 既定のセーブファイルのパスを返します。
    //----------------------------------------------------------------------------
    std::string DefaultPath() {
        return (Tsukino::IO::FileSystem::GetAssetRootPath() / "Saves/save.json").string();
    }

    //----------------------------------------------------------------------------
    //! 今の時刻を返します（1970年からの秒数）。
    //----------------------------------------------------------------------------
    long long NowSeconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    //----------------------------------------------------------------------------
    //! プレイヤーの資源を保存します。
    //----------------------------------------------------------------------------
    bool Save(Tsukino::ECS::Registry& registry, const std::string& path) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>())
            return false;

        const GameState&        state      = registry.GetContext<GameState>();
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();

        rj::StringBuffer               buffer;
        rj::PrettyWriter<rj::StringBuffer> writer(buffer);
        writer.SetIndent(' ', 2);

        writer.StartObject();
        writer.Key("version");
        writer.Int(kVersion);
        writer.Key("savedAt");
        writer.Int64(NowSeconds());
        writer.Key("coins");
        writer.Int(state.coins);
        writer.Key("fruitPoints");
        writer.Int64(state.fruitPoints);
        writer.Key("harvestValue");
        writer.Int64(state.harvestValue);
        writer.Key("mana");
        writer.Int(state.mana);

        // 強化のレベル（id → レベル）。果樹の段階はここから計算し直すので保存しない
        writer.Key("upgrades");
        writer.StartObject();
        for(const auto& [id, level] : state.upgradeLevels) {
            writer.Key(id.c_str());
            writer.Int(level);
        }
        writer.EndObject();

        // 収穫数（果物の id → バリエーションの id → 数）。0 の枠は書かない
        writer.Key("harvest");
        writer.StartObject();
        for(size_t f = 0; f < state.harvestCounts.size() && f < catalog.Fruits().size(); ++f) {
            const auto& counts = state.harvestCounts[f];
            if(std::none_of(counts.begin(), counts.end(), [](int n) { return n > 0; }))
                continue;

            writer.Key(catalog.Fruits()[f].id.c_str());
            writer.StartObject();
            for(size_t v = 0; v < counts.size() && v < collection.Variants().size(); ++v) {
                if(counts[v] <= 0)
                    continue;
                writer.Key(collection.Variants()[v].id.c_str());
                writer.Int(counts[v]);
            }
            writer.EndObject();
        }
        writer.EndObject();
        writer.EndObject();

        //--------------------------------------------------------------
        // 一時ファイルに書いてから置き換える（書いている途中で落ちても前のセーブが残る）
        //--------------------------------------------------------------
        std::error_code             error;
        const std::filesystem::path target(path);
        std::filesystem::create_directories(target.parent_path(), error);

        const std::filesystem::path temporary = target.string() + ".tmp";
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            if(!out) {
                Tsukino::Core::Log::Warn("SaveData: cannot write " + temporary.string() + ".");
                return false;
            }
            out.write(buffer.GetString(), static_cast<std::streamsize>(buffer.GetSize()));
            out << '\n';
            if(!out) {
                Tsukino::Core::Log::Warn("SaveData: failed while writing " + temporary.string() + ".");
                return false;
            }
        }
        std::filesystem::rename(temporary, target, error);
        if(error) {
            Tsukino::Core::Log::Warn("SaveData: cannot replace " + path + " (" + error.message() + ").");
            return false;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! セーブファイルを読み込み、GameState に反映します。
    //----------------------------------------------------------------------------
    bool Load(Tsukino::ECS::Registry& registry, const std::string& path, long long& savedAt) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<FruitCatalog>() || !registry.HasContext<CollectionConfig>())
            return false;
        if(!Tsukino::IO::FileSystem::Exists(Tsukino::Core::Path(path))) {
            Tsukino::Core::Log::Info("SaveData: no save file. Starting a new game.");
            return false;
        }

        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            // 次の保存で上書きされて手で直す機会も失わないよう、壊れたファイルを別名で残す
            std::error_code error;
            std::filesystem::copy_file(path, path + ".broken", std::filesystem::copy_options::overwrite_existing, error);
            Tsukino::Core::Log::Warn("SaveData: " + path + " is broken. Starting a new game (the broken file was copied to " + path + ".broken).");
            return false;
        }

        GameState&              state      = registry.GetContext<GameState>();
        const FruitCatalog&     catalog    = registry.GetContext<FruitCatalog>();
        const CollectionConfig& collection = registry.GetContext<CollectionConfig>();

        savedAt = 0;
        ReadInteger(doc, "savedAt", savedAt);
        ReadInteger(doc, "coins", state.coins);
        ReadInteger(doc, "fruitPoints", state.fruitPoints);
        ReadInteger(doc, "harvestValue", state.harvestValue);
        ReadInteger(doc, "mana", state.mana);
        state.coins       = std::max(0, state.coins);
        state.fruitPoints = std::max(0LL, state.fruitPoints);
        state.mana        = std::clamp(state.mana, 0, state.maxMana);

        //--------------------------------------------------------------
        // 強化のレベル
        //--------------------------------------------------------------
        state.upgradeLevels.clear();
        auto upgrades = doc.FindMember("upgrades");
        if(upgrades != doc.MemberEnd() && upgrades->value.IsObject()) {
            for(auto it = upgrades->value.MemberBegin(); it != upgrades->value.MemberEnd(); ++it) {
                if(it->value.IsInt() && it->value.GetInt() > 0)
                    state.upgradeLevels[it->name.GetString()] = it->value.GetInt();
            }
        }

        //--------------------------------------------------------------
        // 収穫数。定義データから消えた果物・バリエーションは読み飛ばす
        //--------------------------------------------------------------
        for(auto& counts : state.harvestCounts)
            std::fill(counts.begin(), counts.end(), 0);

        auto harvest = doc.FindMember("harvest");
        if(harvest != doc.MemberEnd() && harvest->value.IsObject()) {
            for(auto fruit = harvest->value.MemberBegin(); fruit != harvest->value.MemberEnd(); ++fruit) {
                const int f = catalog.FindIndex(fruit->name.GetString());
                if(f < 0 || f >= static_cast<int>(state.harvestCounts.size()) || !fruit->value.IsObject()) {
                    Tsukino::Core::Log::Warn(std::string("SaveData: fruit \"") + fruit->name.GetString() + "\" is no longer defined. Its harvest was skipped.");
                    continue;
                }
                for(auto variant = fruit->value.MemberBegin(); variant != fruit->value.MemberEnd(); ++variant) {
                    const int v = collection.FindIndex(variant->name.GetString());
                    if(v < 0 || v >= static_cast<int>(state.harvestCounts[f].size()) || !variant->value.IsInt())
                        continue;
                    state.harvestCounts[f][v] = std::max(0, variant->value.GetInt());
                }
            }
        }

        Tsukino::Core::Log::Info("SaveData: loaded " + path + ".");
        return true;
    }
}    // namespace FruitMagic::SaveData

//----------------------------------------------------------------------------
//! @file   Settings.cpp
//! @brief  オプションの設定の保存と読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/Settings.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/prettywriter.h>
#include <cereal/external/rapidjson/stringbuffer.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //! @brief 音量の1段（10%）
        constexpr float kVolumeStep = 0.1f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! 既定の保存先を返します。
    //----------------------------------------------------------------------------
    std::string Settings::DefaultPath() {
        return (Tsukino::IO::FileSystem::GetAssetRootPath() / "Saves/settings.json").string();
    }

    namespace {
        //! 設定の項目を読みます（既定の値のファイルと保存先で同じ書き方）。
        void ReadSettings(const Json::Document& doc, Settings& settings) {
            Json::Read(doc, "bgmVolume", settings.bgmVolume);
            Json::Read(doc, "seVolume", settings.seVolume);
            Json::Read(doc, "muted", settings.muted);
            Json::Read(doc, "showControlsHint", settings.showControlsHint);
            Json::Read(doc, "showCutIn", settings.showCutIn);
            Settings::StepVolume(settings.bgmVolume, 0);
            Settings::StepVolume(settings.seVolume, 0);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 既定の値を読み込みます。
    //----------------------------------------------------------------------------
    bool Settings::LoadDefaults(const std::string& defaultsPath) {
        Json::Document doc;
        if(!Json::ParseFile(defaultsPath, doc, "Settings"))
            return false;
        ReadSettings(doc, *this);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 保存先から読み込みます。
    //----------------------------------------------------------------------------
    bool Settings::Load(const std::string& filePath, bool saveBack) {
        path = saveBack ? filePath : std::string();
        if(filePath.empty() || !Tsukino::IO::FileSystem::Exists(Tsukino::Core::Path(filePath)))
            return false;

        Json::Document doc;
        if(!Json::ParseFile(filePath, doc, "Settings"))
            return false;

        ReadSettings(doc, *this);
        return true;
    }

    //----------------------------------------------------------------------------
    //! 保存します。
    //----------------------------------------------------------------------------
    bool Settings::Save() const {
        if(path.empty())
            return false;

        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;
        rj::StringBuffer                   buffer;
        rj::PrettyWriter<rj::StringBuffer> writer(buffer);
        writer.SetIndent(' ', 2);
        writer.StartObject();
        writer.Key("bgmVolume");
        writer.Double(bgmVolume);
        writer.Key("seVolume");
        writer.Double(seVolume);
        writer.Key("muted");
        writer.Bool(muted);
        writer.Key("showControlsHint");
        writer.Bool(showControlsHint);
        writer.Key("showCutIn");
        writer.Bool(showCutIn);
        writer.EndObject();

        //--------------------------------------------------------------
        // 一時ファイルに書いてから置き換える（書いている途中で落ちても前の設定が残る）
        //--------------------------------------------------------------
        std::error_code             error;
        const std::filesystem::path target(path);
        std::filesystem::create_directories(target.parent_path(), error);

        const std::filesystem::path temporary = target.string() + ".tmp";
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            if(!out) {
                Tsukino::Core::Log::Warn("Settings: cannot write " + temporary.string() + ".");
                return false;
            }
            out.write(buffer.GetString(), static_cast<std::streamsize>(buffer.GetSize()));
            out << '\n';
        }
        std::filesystem::rename(temporary, target, error);
        if(error) {
            Tsukino::Core::Log::Warn("Settings: cannot replace " + path + " (" + error.message() + ").");
            return false;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! 音量を 10% 動かします。
    //----------------------------------------------------------------------------
    void Settings::StepVolume(float& volume, int steps) {
        // 小数の誤差がたまらないよう、段数に直してから動かす
        const int level = static_cast<int>(std::lround(volume / kVolumeStep)) + steps;
        volume          = std::clamp(static_cast<float>(level) * kVolumeStep, 0.0f, 1.0f);
    }
}    // namespace FruitMagic

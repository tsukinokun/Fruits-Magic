//----------------------------------------------------------------------------
//! @file   RouletteConfig.cpp
//! @brief  チェッカーとルーレットの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/RouletteConfig.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

#include <algorithm>

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
        void ReadFloat(const rj::Value& obj, const char* key, float& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsNumber())
                out = static_cast<float>(it->value.GetDouble());
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool RouletteConfig::Load(const std::string& path) {
        const std::string text = Tsukino::IO::FileSystem::ReadText(Tsukino::Core::Path(path));
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn("RouletteConfig: cannot read " + path + ". Using defaults.");
            return false;
        }

        float stock = static_cast<float>(maxStock);
        ReadFloat(doc, "hitChance", hitChance);
        ReadFloat(doc, "spinSeconds", spinSeconds);
        ReadFloat(doc, "resultSeconds", resultSeconds);
        ReadFloat(doc, "maxStock", stock);
        ReadFloat(doc, "checkerHalfWidth", checkerHalfWidth);
        ReadFloat(doc, "checkerRange", checkerRange);
        ReadFloat(doc, "checkerPeriod", checkerPeriod);

        hitChance = std::clamp(hitChance, 0.0f, 1.0f);
        maxStock  = std::max(1, static_cast<int>(stock));

        Tsukino::Core::Log::Info("RouletteConfig: hitChance=" + std::to_string(hitChance) + " maxStock=" + std::to_string(maxStock) +
                                 " checkerHalfWidth=" + std::to_string(checkerHalfWidth));
        return true;
    }
}    // namespace FruitMagic

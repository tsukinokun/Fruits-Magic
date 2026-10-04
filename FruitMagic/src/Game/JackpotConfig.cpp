//----------------------------------------------------------------------------
//! @file   JackpotConfig.cpp
//! @brief  ジャックポット穴の設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/JackpotConfig.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>

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
        template <class T>
        void ReadNumber(const rj::Value& obj, const char* key, T& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsNumber())
                out = static_cast<T>(it->value.GetDouble());
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool JackpotConfig::Load(const std::string& path) {
        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn("JackpotConfig: cannot read " + path + ". Using defaults.");
            return false;
        }

        ReadNumber(doc, "holeHalfWidth", holeHalfWidth);
        ReadNumber(doc, "holeHalfDepth", holeHalfDepth);
        ReadNumber(doc, "frontOffset", frontOffset);
        ReadNumber(doc, "range", range);
        ReadNumber(doc, "period", period);
        ReadNumber(doc, "openSeconds", openSeconds);
        ReadNumber(doc, "closedSeconds", closedSeconds);
        ReadNumber(doc, "spins", spins);
        ReadNumber(doc, "bonusCoins", bonusCoins);

        period        = std::max(0.1f, period);
        openSeconds   = std::max(0.0f, openSeconds);
        closedSeconds = std::max(0.0f, closedSeconds);
        spins         = std::max(0, spins);
        bonusCoins    = std::max(0, bonusCoins);
        return true;
    }
}    // namespace FruitMagic

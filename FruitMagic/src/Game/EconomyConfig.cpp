//----------------------------------------------------------------------------
//! @file   EconomyConfig.cpp
//! @brief  コインのやりくりの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/EconomyConfig.hpp>

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
    bool EconomyConfig::Load(const std::string& path) {
        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        if(text.empty() || doc.HasParseError() || !doc.IsObject()) {
            Tsukino::Core::Log::Warn("EconomyConfig: cannot read " + path + ". Using defaults.");
            return false;
        }

        ReadNumber(doc, "startCoins", startCoins);
        ReadNumber(doc, "reliefBelow", reliefBelow);
        ReadNumber(doc, "reliefSeconds", reliefSeconds);
        ReadNumber(doc, "maxCoinsOnTable", maxCoinsOnTable);

        startCoins      = std::max(0, startCoins);
        reliefBelow     = std::max(0, reliefBelow);
        reliefSeconds   = std::max(0.5f, reliefSeconds);
        maxCoinsOnTable = std::max(0, maxCoinsOnTable);
        return true;
    }
}    // namespace FruitMagic

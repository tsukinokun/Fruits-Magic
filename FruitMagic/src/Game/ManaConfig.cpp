//----------------------------------------------------------------------------
//! @file   ManaConfig.cpp
//! @brief  マナの獲得量の設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/ManaConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool ManaConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "ManaConfig"))
            return false;

        Json::Read(doc, "maxMana", maxMana);
        Json::Read(doc, "coinPayout", coinPayout);
        Json::Read(doc, "coinGutter", coinGutter);
        Json::Read(doc, "fruitGutterRatio", fruitGutterRatio);

        maxMana = std::max(1, maxMana);
        return true;
    }
}    // namespace FruitMagic

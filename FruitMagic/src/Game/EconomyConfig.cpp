//----------------------------------------------------------------------------
//! @file   EconomyConfig.cpp
//! @brief  コインのやりくりの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/EconomyConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool EconomyConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "EconomyConfig"))
            return false;

        Json::Read(doc, "startCoins", startCoins);
        Json::Read(doc, "reliefBelow", reliefBelow);
        Json::Read(doc, "reliefSeconds", reliefSeconds);
        Json::Read(doc, "maxCoinsOnTable", maxCoinsOnTable);
        Json::Read(doc, "reliefAmount", reliefAmount);
        Json::Read(doc, "launchInterval", launchInterval);
        Json::Read(doc, "launchLaneSpeed", launchLaneSpeed);

        startCoins      = std::max(0, startCoins);
        reliefBelow     = std::max(0, reliefBelow);
        reliefSeconds   = std::max(0.5f, reliefSeconds);
        maxCoinsOnTable = std::max(0, maxCoinsOnTable);
        reliefAmount    = std::max(1, reliefAmount);
        launchInterval  = std::max(0.0f, launchInterval);
        launchLaneSpeed = std::max(0.0f, launchLaneSpeed);
        return true;
    }
}    // namespace FruitMagic

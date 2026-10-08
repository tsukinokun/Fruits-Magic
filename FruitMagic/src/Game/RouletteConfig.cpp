//----------------------------------------------------------------------------
//! @file   RouletteConfig.cpp
//! @brief  チェッカーとルーレットの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/RouletteConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool RouletteConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "RouletteConfig"))
            return false;

        Json::Read(doc, "hitChance", hitChance);
        Json::Read(doc, "coinChance", coinChance);
        Json::Read(doc, "coinAmount", coinAmount);
        Json::Read(doc, "coinShowerSeconds", coinShowerSeconds);
        Json::Read(doc, "resultSeconds", resultSeconds);
        Json::Read(doc, "maxStock", maxStock);
        Json::Read(doc, "checkerRange", checkerRange);
        Json::Read(doc, "checkerPeriod", checkerPeriod);
        Json::ReadFloats(doc, "reelStopSeconds", reelStopSeconds.data(), kReelCount);
        Json::Read(doc, "reachExtraSeconds", reachExtraSeconds);
        Json::Read(doc, "reachMissChance", reachMissChance);
        Json::Read(doc, "flySeconds", flySeconds);
        Json::Read(doc, "fruitSpawnMargin", fruitSpawnMargin);
        Json::Read(doc, "fruitSpawnLift", fruitSpawnLift);
        Json::Read(doc, "fruitSpawnBackMargin", fruitSpawnBackMargin);

        hitChance         = std::clamp(hitChance, 0.0f, 1.0f);
        coinChance        = std::clamp(coinChance, 0.0f, 1.0f);
        coinAmount        = std::max(0, coinAmount);
        coinShowerSeconds = std::max(0.0f, coinShowerSeconds);
        maxStock          = std::max(1, maxStock);
        // 左から順に止まるよう、前の列より後にする
        for(int reel = 0; reel < kReelCount; ++reel)
            reelStopSeconds[reel] = std::max(reelStopSeconds[reel], reel > 0 ? reelStopSeconds[reel - 1] + 0.05f : 0.1f);
        reachExtraSeconds = std::max(0.0f, reachExtraSeconds);
        reachMissChance   = std::clamp(reachMissChance, 0.0f, 1.0f);
        flySeconds        = std::max(0.05f, flySeconds);

        Tsukino::Core::Log::Info("RouletteConfig: hitChance=" + std::to_string(hitChance) + " maxStock=" + std::to_string(maxStock));
        return true;
    }
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   JackpotConfig.cpp
//! @brief  ジャックポットチャンスの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/JackpotConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool JackpotConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "JackpotConfig"))
            return false;

        Json::Read(doc, "chanceRate", chanceRate);
        Json::Read(doc, "winRate", winRate);
        Json::Read(doc, "resultSeconds", resultSeconds);
        Json::Read(doc, "bonusCoins", bonusCoins);
        Json::Read(doc, "consolationCoins", consolationCoins);
        Json::Read(doc, "spins", spins);
        Json::ReadFloats(doc, "reelStopSeconds", reelStopSeconds.data(), kReelCount);
        Json::Read(doc, "symbolFruit", symbolFruit);
        Json::Read(doc, "winShowerSeconds", winShowerSeconds);
        Json::Read(doc, "loseShowerSeconds", loseShowerSeconds);
        Json::Read(doc, "winNoticeSeconds", winNoticeSeconds);
        Json::ReadVec(doc, "chanceEffectPosition", chanceEffectPosition);
        Json::ReadVec(doc, "winEffectPosition", winEffectPosition);

        chanceRate       = std::clamp(chanceRate, 0.0f, 1.0f);
        winRate          = std::clamp(winRate, 0.0f, 1.0f);
        for(int reel = 0; reel < kReelCount; ++reel)
            reelStopSeconds[reel] = std::max(reelStopSeconds[reel], reel > 0 ? reelStopSeconds[reel - 1] + 0.05f : 0.1f);
        resultSeconds    = std::max(0.1f, resultSeconds);
        bonusCoins       = std::max(0, bonusCoins);
        consolationCoins = std::max(0, consolationCoins);
        spins            = std::max(0, spins);
        return true;
    }
}    // namespace FruitMagic

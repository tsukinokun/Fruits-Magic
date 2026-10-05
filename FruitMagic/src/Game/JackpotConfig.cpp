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
        Json::Read(doc, "spinSeconds", spinSeconds);
        Json::Read(doc, "resultSeconds", resultSeconds);
        Json::Read(doc, "bonusCoins", bonusCoins);
        Json::Read(doc, "consolationCoins", consolationCoins);
        Json::Read(doc, "spins", spins);
        Json::Read(doc, "flipInterval", flipInterval);
        Json::Read(doc, "winShowerSeconds", winShowerSeconds);
        Json::Read(doc, "loseShowerSeconds", loseShowerSeconds);
        Json::Read(doc, "winNoticeSeconds", winNoticeSeconds);
        Json::ReadVec(doc, "chanceEffectPosition", chanceEffectPosition);
        Json::ReadVec(doc, "winEffectPosition", winEffectPosition);

        chanceRate       = std::clamp(chanceRate, 0.0f, 1.0f);
        winRate          = std::clamp(winRate, 0.0f, 1.0f);
        spinSeconds      = std::max(0.1f, spinSeconds);
        resultSeconds    = std::max(0.1f, resultSeconds);
        bonusCoins       = std::max(0, bonusCoins);
        consolationCoins = std::max(0, consolationCoins);
        spins            = std::max(0, spins);
        flipInterval     = std::max(0.02f, flipInterval);
        return true;
    }
}    // namespace FruitMagic

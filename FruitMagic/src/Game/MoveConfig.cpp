//----------------------------------------------------------------------------
//! @file   MoveConfig.cpp
//! @brief  屋台のおひっこしの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/MoveConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool MoveConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "MoveConfig"))
            return false;

        Json::Read(doc, "unlockTreeStage", unlockTreeStage);
        Json::Read(doc, "seedDivisor", seedDivisor);
        Json::Read(doc, "minSeeds", minSeeds);

        unlockTreeStage = std::max(0, unlockTreeStage);
        seedDivisor     = std::max(1.0f, seedDivisor);
        minSeeds        = std::max(1, minSeeds);
        return true;
    }
}    // namespace FruitMagic

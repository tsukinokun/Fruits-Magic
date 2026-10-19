//----------------------------------------------------------------------------
//! @file   MoveConfig.hpp
//! @brief  屋台のおひっこし（転生）の設定
//! @detail Assets/Data/Move.json から読み込みます。無い項目は既定値のままです。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! おひっこしの設定です。Registry のコンテキストに置きます。
    struct MoveConfig {
        int   unlockTreeStage = 4;         // この果樹の段階以上の果物を図鑑に1つでも登録すると、おひっこしできるようになる
        float seedDivisor     = 30.0f;     // 星のたね = floor(√(今回の屋台の収穫の価値 ÷ これ) × 獲得量の倍率)
        int   minSeeds        = 3;         // もらえる星のたねがこれ未満のうちは、おひっこしできない

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Move.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };
}    // namespace FruitMagic

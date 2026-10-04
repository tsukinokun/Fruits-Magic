//----------------------------------------------------------------------------
//! @file   GameState.hpp
//! @brief  プレイヤーの資源（手持ちコイン・収穫など）
//! @detail Registry のコンテキストに1つだけ置き、各システムから参照します。
//!         マナ（M3）やセーブ（M6）の対象もここに足していきます。
//----------------------------------------------------------------------------
#pragma once
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! プレイヤーの資源です。
    struct GameState {
        int              coins     = 50;    // 手持ちのコイン枚数
        int              treeLevel = 0;     // 果樹の段階（出現できる果物が決まる。M5 の強化で上がる）
        std::vector<int> harvestCounts;     // 果物ごとの収穫数（FruitCatalog::Fruits() と同じ添字）
    };
}    // namespace FruitMagic

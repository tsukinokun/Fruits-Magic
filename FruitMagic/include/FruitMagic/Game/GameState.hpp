//----------------------------------------------------------------------------
//! @file   GameState.hpp
//! @brief  プレイヤーの資源（手持ちコイン・収穫・マナなど）
//! @detail Registry のコンテキストに1つだけ置き、各システムから参照します。
//!         セーブ（M6）の対象もここに足していきます。
//----------------------------------------------------------------------------
#pragma once
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! プレイヤーの資源です。
    struct GameState {
        int              coins     = 50;     // 手持ちのコイン枚数
        int              treeLevel = 0;      // 果樹の段階（出現できる果物が決まる。M5 の強化で上がる）
        std::vector<int> harvestCounts;      // 果物ごとの収穫数（FruitCatalog::Fruits() と同じ添字）
        int              mana      = 0;      // 今のマナ（魔法を撃つと減る）
        int              maxMana   = 100;    // マナの上限
    };
}    // namespace FruitMagic

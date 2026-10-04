//----------------------------------------------------------------------------
//! @file   GameState.hpp
//! @brief  プレイヤーの資源（手持ちコイン・収穫・マナなど）
//! @detail Registry のコンテキストに1つだけ置き、各システムから参照します。
//!         セーブ（M6）の対象もここに足していきます。
//----------------------------------------------------------------------------
#pragma once
#include <map>
#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! プレイヤーの資源です。
    struct GameState {
        int                           coins        = 50;     // 手持ちのコイン枚数
        long long                     fruitPoints  = 0;      // 手持ちの果実（収穫した果物の価値 × バリエーションの倍率。強化に使う）
        int                           treeLevel    = 0;      // 果樹の段階（出現できる果物が決まる。果樹の強化のレベルと同じ）
        std::map<std::string, int>    upgradeLevels;         // 強化のレベル [強化の id]（無ければ 0）
        std::vector<std::vector<int>> harvestCounts;         // 収穫数 [果物（FruitCatalog の添字）][バリエーション（CollectionConfig の添字）]
        long long                     harvestValue = 0;      // 収穫した果物の価値の合計（価値 × バリエーションの倍率。転生ポイントに使う）
        int                           mana         = 0;      // 今のマナ（魔法を撃つと減る）
        int                           maxMana      = 100;    // マナの上限

        //! 強化のレベルを返します。
        //! @param  [in] id 強化の識別子
        //! @return レベル（強化していなければ 0）
        int UpgradeLevelOf(const std::string& id) const {
            auto it = upgradeLevels.find(id);
            return (it != upgradeLevels.end()) ? it->second : 0;
        }

        //! 図鑑に登録済みの枠（1回以上収穫した果物 × バリエーション）の数を返します。
        //! @return 登録済みの枠の数
        int RegisteredCount() const {
            int count = 0;
            for(const auto& variants : harvestCounts) {
                for(int n : variants) {
                    if(n > 0)
                        ++count;
                }
            }
            return count;
        }

        //! 収穫した果物の合計数を返します。
        //! @return 合計数
        int HarvestTotal() const {
            int total = 0;
            for(const auto& variants : harvestCounts) {
                for(int n : variants)
                    total += n;
            }
            return total;
        }
    };
}    // namespace FruitMagic

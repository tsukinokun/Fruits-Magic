//----------------------------------------------------------------------------
//! @file   MagicCatalog.hpp
//! @brief  魔法の定義データ
//! @detail Assets/Data/Magic.json を読み込みます。名前・コスト・キー・解放状態・効果の数値はデータで持ち、
//!         効果の中身（何が起きるか）は id ごとにシステム側で実装します（例: "shake" → ShakeMagicSystem）。
//----------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! ボタン・キーに割り当てられる魔法の枠の数です。
    inline constexpr int kMagicSlotCount = 5;

    //! 魔法1つの定義です。
    struct MagicDef {
        std::string                            id;                // 識別子（効果の実装を選ぶのに使う）
        std::wstring                           name;              // 表示名
        int                                    cost     = 10;     // 撃つのに必要なマナ
        int                                    slot     = 1;      // 割り当てる枠（1〜kMagicSlotCount。数字キーも同じ番号）
        bool                                   unlocked = false;  // 解放済みか
        std::unordered_map<std::string, float> params;            // 効果ごとの数値（"duration" など）

        //! 効果の数値を返します。
        //! @param  [in] key      数値の名前
        //! @param  [in] fallback 定義に無いときの値
        //! @return 数値
        float Param(const std::string& key, float fallback) const;
    };

    //! 魔法の定義データを保持するクラスです。Registry のコンテキストに置いて共有します。
    class MagicCatalog {
    public:

        //! 定義データを読み込みます。
        //! @param  [in] path 定義ファイル（Magic.json）
        //! @return 1つ以上読み込めたら true
        bool Load(const std::string& path);

        //! 魔法の一覧を返します。
        //! @return 魔法の一覧
        const std::vector<MagicDef>& Magics() const { return m_magics; }

        //! 枠に割り当てられた魔法の添字を返します。
        //! @param  [in] slot 枠の番号（1〜kMagicSlotCount）
        //! @return 添字。割り当てが無ければ -1
        int FindBySlot(int slot) const;

    private:
        std::vector<MagicDef> m_magics;    // 魔法の一覧
    };
}    // namespace FruitMagic

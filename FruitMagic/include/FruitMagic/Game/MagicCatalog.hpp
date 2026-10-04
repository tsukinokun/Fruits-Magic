//----------------------------------------------------------------------------
//! @file   MagicCatalog.hpp
//! @brief  魔法の定義データ
//! @detail Assets/Data/Magic.json を読み込みます。名前・コスト・キー・解放条件・効果の数値はデータで持ち、
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
        int                                    unlockZukan = 0;   // 解放に必要な図鑑の登録数（0 なら最初から使える）
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

        //! 魔法が解放済みかを返します。解放は図鑑の登録数から毎回決まる（セーブには持たない）。
        //! @param  [in] magicIndex      魔法の添字
        //! @param  [in] registeredCount 図鑑の登録数（GameState::RegisteredCount()）
        //! @return 解放済みなら true（添字が範囲外なら false）
        bool IsUnlocked(int magicIndex, int registeredCount) const;

        //! 添字の魔法が指定の id かを返します。効果を実装するシステムが、自分の魔法の MagicCastEvent かを調べるのに使います。
        //! @param  [in] magicIndex 魔法の添字
        //! @param  [in] id         効果の id（"shake" など）
        //! @return 添字が範囲内で id が一致すれば true
        bool IsMagic(int magicIndex, const char* id) const;

        //! 枠に割り当てられた魔法の添字を返します。
        //! @param  [in] slot 枠の番号（1〜kMagicSlotCount）
        //! @return 添字。割り当てが無ければ -1
        int FindBySlot(int slot) const;

    private:
        std::vector<MagicDef> m_magics;    // 魔法の一覧
    };
}    // namespace FruitMagic

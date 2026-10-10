//----------------------------------------------------------------------------
//! @file   UpgradeCatalog.hpp
//! @brief  台の強化の定義データ
//! @detail Assets/Data/Upgrades.json を読み込みます。名前・説明・段階ごとの価格と効果の値はデータで持ち、
//!         値が何に効くか（プッシャーの振幅・果樹の段階など）は id ごとに UpgradeSystem が決めます。
//----------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 強化に払うもの（台の強化はコイン、果物と魔法の強化は FP）です。
    enum class UpgradeCurrency {
        Coins,    // コイン
        Fruit,    // フルーツポイント（FP）
    };

    //! 強化の1段階です。
    struct UpgradeLevel {
        long long cost  = 0;       // 値段（UpgradeDef::currency で払う）
        float     value = 0.0f;    // この段階での効果の値
    };

    //! 強化1種の定義です。
    struct UpgradeDef {
        std::string               id;                // 識別子（効果の実装を選ぶのに使う）
        std::wstring              name;              // 表示名
        std::wstring              description;       // 説明
        std::wstring              format = L"{v}";   // 効果の値の表示形式（"{v}" が値に置き換わる）
        std::wstring              zeroText;          // 値が 0 のときの表示（空なら format で表示）
        UpgradeCurrency           currency = UpgradeCurrency::Coins;    // 払うもの
        float                     baseValue = 0.0f;  // 強化していないときの効果の値
        std::vector<UpgradeLevel> levels;            // 段階ごとの価格と効果（添字 0 が Lv1）

        //! 段階の数（最大レベル）を返します。
        //! @return 最大レベル
        int MaxLevel() const { return static_cast<int>(levels.size()); }

        //! レベルでの効果の値を返します。
        //! @param  [in] level レベル（0 は強化なし。範囲外は端に丸める）
        //! @return 効果の値
        float ValueAt(int level) const;

        //! 効果の値を表示用の文字列にします。
        //! @param  [in] value 効果の値
        //! @return 表示用の文字列
        std::wstring FormatValue(float value) const;
    };

    //! 強化の定義データを保持するクラスです。Registry のコンテキストに置いて共有します。
    class UpgradeCatalog {
    public:

        //! 定義データを読み込みます。不正な強化は警告を出して読み飛ばします。
        //! @param  [in] path 定義ファイル（Upgrades.json）
        //! @return 1つ以上読み込めたら true
        bool Load(const std::string& path);

        //! 強化の一覧を返します（ファイルに書いた順）。
        //! @return 強化の一覧
        const std::vector<UpgradeDef>& Upgrades() const { return m_upgrades; }

        //! id から強化を探します。
        //! @param  [in] id 強化の識別子
        //! @return 定義。見つからなければ nullptr
        const UpgradeDef* Find(const std::string& id) const;

    private:
        std::vector<UpgradeDef> m_upgrades;    // 強化の一覧
    };
}    // namespace FruitMagic

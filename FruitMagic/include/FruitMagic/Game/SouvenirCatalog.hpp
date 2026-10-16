//----------------------------------------------------------------------------
//! @file   SouvenirCatalog.hpp
//! @brief  おみやげ（おひっこしでもらう、果物以外の食べ物）の定義データ
//! @detail Assets/Data/Souvenirs/*.json（1つ＝1種）と、もらい方の設定 Assets/Data/Souvenir.json を読み込みます。
//!         果物と同じく、ファイルを足すだけで増えます（おみやげごとの専用コードは書かない）。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! おみやげの効果の種類です（レベル × 1レベルごとの量だけ効く）。
    enum class SouvenirEffect {
        FruitValue,       // 収穫の FP（%。倍率に掛ける）
        Mana,             // マナの獲得量（%）
        Variant,          // 色違い・金色の出やすさ（%）
        SeedGain,         // おひっこしでもらう星のたね（%）
        RouletteHit,      // ルーレットで果物が当たる確率（+% ポイント）
        MagicDuration,    // 魔法の効果時間（%）
        OfflineHours,     // おるすばんの上限時間（+時間）
        StartCoins,       // おひっこしした屋台の最初のコイン（+枚）
    };

    //! おみやげ1種の定義です。
    struct SouvenirDef {
        std::string    id;                                                   // 識別子（セーブに使う）
        std::wstring   name;                                                 // 表示名
        std::string    model;                                                // モデル（リポジトリルート相対）
        hlslpp::float3 modelRotation = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 見せるときの向き（度）
        int            rarity        = 0;                                    // レア度（0 野菜・1 パンとお菓子・2 料理・3 ごちそう）
        SouvenirEffect effect        = SouvenirEffect::FruitValue;           // 効果の種類
        float          perLevel      = 1.0f;                                 // 1レベルごとの効果の量
        int            maxLevel      = 5;                                    // レベルの上限
    };

    //! おみやげの定義と、もらい方の設定を保持するクラスです。Registry のコンテキストに置きます。
    class SouvenirCatalog {
    public:

        //! 定義データともらい方の設定を読み込みます。不正なおみやげは警告を出して読み飛ばします。
        //! @param  [in] dataRoot 定義データのフォルダ（Assets/Data。Souvenirs/ と Souvenir.json を読む）
        //! @return 1つ以上読み込めたら true
        bool Load(const std::string& dataRoot);

        //! おみやげの一覧を返します（図鑑の並び: レア度 → id）。
        //! @return おみやげの一覧
        const std::vector<SouvenirDef>& Souvenirs() const { return m_souvenirs; }

        //! id からおみやげの添字を探します。
        //! @param  [in] id おみやげの識別子
        //! @return 添字。見つからなければ -1
        int FindIndex(const std::string& id) const;

        //--------------------------------------------------------------
        // もらい方（Souvenir.json）
        //--------------------------------------------------------------
        float              valuePerExtra     = 400.0f;                        // 今回の屋台の収穫の価値がこれだけ増えるごとに1つ多くもらえる（最低1つ）
        int                maxPerMove        = 3;                             // 1回のおひっこしでもらえる数の上限
        std::vector<float> rarityWeights     = {60.0f, 28.0f, 10.0f, 2.0f};   // レア度ごとの出やすさ（手持ちのコインが0のとき）
        std::vector<float> rarityBoost       = {0.0f, 1.0f, 3.0f, 8.0f};      // コインを最大まで持っていたとき、重みに足す倍率（重み × (1 + これ × コインの割合)）
        float              coinsForFullBoost = 3000.0f;                       // このコインを持っておひっこしすると、レアの上げ幅が最大になる

    private:
        std::vector<SouvenirDef> m_souvenirs;    // おみやげの一覧
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   FruitCatalog.hpp
//! @brief  果物とランクの定義データ
//! @detail Assets/Data/FruitRanks.json と Assets/Data/Fruits/*.json を読み込み、果物の定義を保持します。
//!         果物を増やすときは JSON を1つ置くだけでよく、果物ごとの専用コードは書きません。
//!         図鑑・出現抽選・解放条件はすべてこの定義から組み立てます。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <random>
#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 果物の当たり判定の形状です。
    enum class FruitShape {
        Sphere,     // 球（radius）
        Box,        // 箱（halfExtent）
        Capsule,    // 縦長のカプセル（radius ＋ halfHeight）
    };

    //! 果物のランク（小・中・大・伝説 など）の定義です。
    struct FruitRank {
        std::string  id;           // 識別子（果物の "rank" から参照される）
        std::wstring name;         // 表示名
        int          order = 0;    // 並び順（小さいほど下位）
    };

    //! 果物1種の定義です。JSON の1ファイルに対応します。
    struct FruitDef {
        std::string  id;                      // 識別子（ファイル名と同じにしておく）
        std::wstring name;                    // 表示名
        int          rankIndex = 0;           // FruitCatalog::Ranks() の添字

        FruitShape     shape      = FruitShape::Sphere;                      // 当たり判定の形状
        float          radius     = 3.0f;                                    // 球・カプセルの半径（cm）
        float          halfHeight = 0.0f;                                    // カプセルの円柱部分の半分の高さ（cm）
        hlslpp::float3 halfExtent = hlslpp::float3(3.0f, 3.0f, 3.0f);        // 箱の半サイズ（cm）

        float mass        = 2.0f;     // 質量（コイン1枚 = 1）
        int   value       = 1;        // 価値（転生ポイントなどの計算に使う）
        int   mana        = 1;        // 落ちたときに増えるマナ（M3 で使う）
        float spawnWeight = 1.0f;     // 出現抽選の重み
        int   unlockTreeLevel = 0;    // この果樹の段階以上で出現する

        hlslpp::float3 color = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 仮の見た目の色（モデル差し替えまで）
        std::string    modelPath;                                     // 見た目のモデル（リポジトリルート相対）

        //! 当たり判定の外接する高さの半分を返します。
        //! @return 中心から上端（下端）までの距離（cm）
        float HalfHeightOfBounds() const;
    };

    //! 果物とランクの定義データを保持するクラスです。Registry のコンテキストに置いて共有します。
    class FruitCatalog {
    public:

        //! 定義データを読み込みます。不正な果物は警告を出して読み飛ばします。
        //! @param  [in] dataRoot データフォルダ（例: "<アセットルート>/Assets/Data"）
        //! @return ランクと果物が1つ以上読み込めたら true
        bool Load(const std::string& dataRoot);

        //! ランクの一覧を返します（order 順）。
        //! @return ランクの一覧
        const std::vector<FruitRank>& Ranks() const { return m_ranks; }

        //! 果物の一覧を返します（ランク順、同ランク内は id 順）。
        //! @return 果物の一覧
        const std::vector<FruitDef>& Fruits() const { return m_fruits; }

        //! id から果物の添字を探します。
        //! @param  [in] id 果物の識別子
        //! @return 添字。見つからなければ -1
        int FindIndex(const std::string& id) const;

        //! 指定の果樹の段階で出現できる果物から、出現の重みで1つ選びます。
        //! @param  [in]     treeLevel 果樹の段階
        //! @param  [in,out] rng       乱数生成器
        //! @return 選んだ果物の添字。出現できる果物が無ければ -1
        int PickSpawnable(int treeLevel, std::mt19937& rng) const;

    private:
        std::vector<FruitRank> m_ranks;     // ランクの一覧
        std::vector<FruitDef>  m_fruits;    // 果物の一覧
    };

    //! 文字列を UTF-8 から表示用のワイド文字列へ変換します。
    //! @param  [in] utf8 UTF-8 の文字列
    //! @return ワイド文字列
    std::wstring Utf8ToWide(const std::string& utf8);
}    // namespace FruitMagic

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

    //! 果物の飾りのパーツ（ヘタ・葉・軸など）の形です。
    enum class FruitPartShape {
        Sphere,    // 球（Ball.fbx）
        Box,       // 箱（Block.fbx）
    };

    //! 果物の飾りのパーツです。当たり判定は持たず、果物と一緒に動く見た目だけの物です。
    //! 位置と大きさは果物の見た目の半サイズに対する比率で書くので、果物が大きくなっても形が崩れません。
    struct FruitPart {
        FruitPartShape shape    = FruitPartShape::Box;                 // 形
        hlslpp::float3 offset   = hlslpp::float3(0.0f, 1.0f, 0.0f);    // 果物の中心からの位置（果物の半サイズに対する比率。y=1 で上端）
        hlslpp::float3 size     = hlslpp::float3(0.3f, 0.1f, 0.3f);    // 半サイズ（果物の半サイズに対する比率）
        hlslpp::float3 rotation = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 回転（度。X → Y → Z の順に回す）
        hlslpp::float3 color    = hlslpp::float3(0.3f, 0.75f, 0.25f);  // 色
        float          glow     = 0.0f;                                // 輪郭の光の強さ（0 で光らない）
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

        hlslpp::float3 color      = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 見た目の色（通常）
        hlslpp::float3 shinyColor = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 色違いのときの色
        std::string    modelPath;                                     // 見た目のモデル（リポジトリルート相対）
        std::vector<FruitPart> parts;                                 // 飾りのパーツ（無くてもよい）

        //--------------------------------------------------------------
        // 作り込んだモデル（JSON に "model" を書いたとき）の合わせ方。
        // モデルは当たり判定の中に収まる大きさへ縦横比を保って拡大し、中心を当たり判定の中心に合わせる
        //--------------------------------------------------------------
        bool           customModel   = false;                              // "model" を指定した（モデルを当たり判定に合わせて置く）
        hlslpp::float3 modelRotation = hlslpp::float3(0.0f, 0.0f, 0.0f);    // モデルの向きの補正（度。X → Y → Z の順に回す）
        float          modelScale    = 1.0f;                               // 当たり判定に収めた大きさに掛ける倍率（見た目の微調整）
        bool           tintBase      = true;                               // 通常の色（color）もモデルに掛けるか。false ならモデル自身の色のまま（色違い・金色は掛ける）

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
}    // namespace FruitMagic

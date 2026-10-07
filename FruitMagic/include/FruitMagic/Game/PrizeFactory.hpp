//----------------------------------------------------------------------------
//! @file   PrizeFactory.hpp
//! @brief  台の部品と景品のエンティティ生成
//! @detail 見た目（モデル）・コライダー・剛体をまとめて付けたエンティティを生成します。
//!         シーンの初期配置とシステム（コインの投入・果物の補充など）の両方から使うため、
//!         Registry のコンテキストに置いて共有します。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/StageConfig.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>

#include <hlsl++.h>

#include <string>
#include <unordered_map>

// 名前空間 : Tsukino::Asset
namespace Tsukino::Asset {
    class AssetManager;    // 前方宣言
}

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    struct FruitDef;    // 前方宣言

    //! 台の部品と景品のエンティティを生成するクラスです。
    class PrizeFactory {
    public:

        //! 生成に使うモデルを読み込み、景品の大きさ・物理・見た目の設定を受け取ります。
        //! @param  [in] assetManager アセットマネージャー（果物のモデルを後から読むため保持する）
        //! @param  [in] layout       台の寸法と景品の物理（コインの大きさ・摩擦など）
        //! @param  [in] stage        見た目の設定（コインの色・輪郭の光）
        void Initialize(Tsukino::Asset::AssetManager& assetManager, const TableLayout& layout, const StageConfig& stage);

        //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] type       剛体の種類
        //! @param  [in] color      見た目の色（省略時は白）
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                       Tsukino::BuiltIn::ECS::RigidbodyType type, const hlslpp::float3& color = hlslpp::float3(1.0f, 1.0f, 1.0f));

        //! 見た目だけの箱（コライダー無し）を生成します。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] opacity    不透明度（0〜1）
        //! @param  [in] color      マテリアルの基本色に掛ける色（白なら元のまま）
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateVisualBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                             float opacity = 1.0f, const hlslpp::float3& color = hlslpp::float3(1.0f, 1.0f, 1.0f));

        //! 景品のコインを生成します。
        //! @param  [in] registry 生成先のレジストリ
        //! @param  [in] position 中心のワールド座標
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateCoin(Tsukino::ECS::Registry& registry, const hlslpp::float3& position);

        //! 果物を定義データから生成します。
        //! @param  [in] registry     生成先のレジストリ
        //! @param  [in] def          果物の定義
        //! @param  [in] fruitIndex   FruitCatalog::Fruits() での添字
        //! @param  [in] variantIndex CollectionConfig::Variants() での添字（0 は通常）
        //! @param  [in] color        見た目の色（バリエーションで決まる）
        //! @param  [in] glow         輪郭の光の強さ（バリエーションで決まる）
        //! @param  [in] position     中心のワールド座標
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateFruit(Tsukino::ECS::Registry& registry, const FruitDef& def, int fruitIndex, int variantIndex, const hlslpp::float3& color,
                                         float glow, const hlslpp::float3& position);

        //! 果物の見た目だけ（当たり判定・物理・景品の情報を持たない）を生成します。台の上の果物と同じ見た目で、
        //! 演出（カットインなど）に使います。破棄は DestroyPrize で、飾りのパーツも一緒に消えます。
        //! @param  [in] registry 生成先のレジストリ
        //! @param  [in] def      果物の定義
        //! @param  [in] color    見た目の色（バリエーションで決まる）
        //! @param  [in] glow     輪郭の光の強さ（バリエーションで決まる）
        //! @param  [in] position 中心のワールド座標
        //! @return 生成したエンティティ（見た目は当たり判定の外接の箱 FruitHalfExtent に収まる）
        Tsukino::ECS::Entity CreateFruitVisual(Tsukino::ECS::Registry& registry, const FruitDef& def, const hlslpp::float3& color, float glow,
                                               const hlslpp::float3& position);

        //! 果物の見た目の大きさ（当たり判定の外接の半サイズ）を返します。
        //! @param  [in] def 果物の定義
        //! @return 半サイズ
        static hlslpp::float3 FruitHalfExtent(const FruitDef& def);

        //! 見た目だけの球（コライダー無し）を生成します。ちょうちんなどの飾りに使います。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 半サイズ
        //! @param  [in] color      色
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateVisualBall(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                              const hlslpp::float3& color);

        //! 見た目だけのモデル（コライダー無し）を置きます。屋台などの飾りに使います。
        //! @param  [in] registry 生成先のレジストリ
        //! @param  [in] path     モデルのパス（リポジトリルート相対）
        //! @param  [in] position モデルの原点のワールド座標
        //! @param  [in] rotation 向き（度。X → Y → Z の順に回す）
        //! @param  [in] scale    拡大率
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateVisualModel(Tsukino::ECS::Registry& registry, const std::string& path, const hlslpp::float3& position,
                                               const hlslpp::float3& rotation, const hlslpp::float3& scale);

        //! 景品（コイン・果物）を破棄予約します。果物の飾りのパーツも一緒に破棄します。
        //! @param  [in] registry レジストリ
        //! @param  [in] entity   破棄する景品
        static void DestroyPrize(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity);

        //! 度で表した回転（X → Y → Z の順に回す）をクォータニオンにします。
        //! @param  [in] degrees 各軸の回転（度）
        //! @return クォータニオン
        static hlslpp::quaternion EulerDegrees(const hlslpp::float3& degrees);

        //! コインの半分の大きさを返します（Table.json の coin.halfExtent）。
        //! @return 半分の大きさ
        const hlslpp::float3& CoinHalfExtent() const { return m_layout.coinHalfExtent; }

    private:

        //! 果物の見た目（モデル・色・輪郭の光・飾りのパーツ）を付けます。
        //! @param  [in] registry レジストリ
        //! @param  [in] fruit    果物のエンティティ（TransformComponent を持つ）
        //! @param  [in] def      果物の定義
        //! @param  [in] color    見た目の色
        //! @param  [in] glow     輪郭の光の強さ
        void AttachFruitLook(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity fruit, const FruitDef& def, const hlslpp::float3& color, float glow);

        //! 果物に飾りのパーツを付けます。
        //! @param  [in] registry         レジストリ
        //! @param  [in] fruit            果物のエンティティ
        //! @param  [in] def              果物の定義（parts を使う）
        //! @param  [in] fruitModelHalf   果物のモデルのスケール1での半サイズ
        void AttachParts(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity fruit, const FruitDef& def, const hlslpp::float3& fruitModelHalf);

        //! 読み込んだモデルと、スケール1での大きさです。
        struct ModelInfo {
            Tsukino::Asset::AssetHandle handle;                                         // モデルのハンドル
            hlslpp::float3              halfExtent = hlslpp::float3(1.0f, 1.0f, 1.0f);    // スケール1での半サイズ
            hlslpp::float3              center     = hlslpp::float3(0.0f, 0.0f, 0.0f);    // スケール1での外接の箱の中心（原点が底にあるモデルではずれる）
            hlslpp::float3              baseColor  = hlslpp::float3(1.0f, 1.0f, 1.0f);    // マテリアルの基本色（色を付けるときに掛ける）
        };

        //! 作り込んだモデルを、当たり判定に合わせて子のエンティティとして付けます。
        //! 親（当たり判定を持つエンティティ）は拡縮しないので、モデルの拡縮・向き・中心のずれはこの子で受け持ちます。
        //! @param  [in] registry   レジストリ
        //! @param  [in] parent     親のエンティティ（当たり判定を持つ）
        //! @param  [in] path       モデルのパス
        //! @param  [in] color      マテリアルの基本色に掛ける色（白なら元のまま）
        //! @param  [in] targetHalf 合わせる大きさ（当たり判定の外接の半サイズ）
        //! @param  [in] rotation   向きの補正（度）
        //! @param  [in] uniform    縦横比を保つか（保つときは targetHalf に収まる最大の大きさ）
        //! @param  [in] extraScale 合わせた大きさに掛ける倍率
        //! @return 見た目の子のエンティティ
        Tsukino::ECS::Entity AttachModelVisual(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity parent, const std::string& path, const hlslpp::float3& color,
                                               const hlslpp::float3& targetHalf, const hlslpp::float3& rotation, bool uniform, float extraScale);

        //! モデルを読み込み、大きさを測ります（同じパスは2回目から使い回す）。
        //! @param  [in] path モデルのパス（リポジトリルート相対）
        //! @return 読み込んだモデルの情報
        const ModelInfo& GetModel(const std::string& path);

        //! 見た目の色を付けます。MaterialPropertyBlockComponent でマテリアルの基本色を「モデルの基本色 × 色」に置き換えます
        //! （アセットは複製しない。白なら何もしない＝モデル自身の色のまま）。
        //! @param  [in] registry レジストリ
        //! @param  [in] entity   ModelComponent を持つエンティティ
        //! @param  [in] model    エンティティが使うモデル（基本色を使う）
        //! @param  [in] color    色
        static void SetColor(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, const ModelInfo& model, const hlslpp::float3& color);

        Tsukino::Asset::AssetManager*              m_assetManager = nullptr;    // モデルの読み込みに使う
        TableLayout                                m_layout;                    // 景品の大きさ・物理（Initialize で受け取った写し）
        StageConfig                                m_stage;                     // 景品の見た目（同上）
        std::unordered_map<std::string, ModelInfo> m_models;                    // パスごとのモデル（色違いも同じモデルを共有する）
    };
}    // namespace FruitMagic

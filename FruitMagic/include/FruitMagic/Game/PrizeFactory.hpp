//----------------------------------------------------------------------------
//! @file   PrizeFactory.hpp
//! @brief  台の部品と景品のエンティティ生成
//! @detail 見た目（モデル）・コライダー・剛体をまとめて付けたエンティティを生成します。
//!         シーンの初期配置とシステム（コインの投入・果物の補充など）の両方から使うため、
//!         Registry のコンテキストに置いて共有します。
//----------------------------------------------------------------------------
#pragma once
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

        //! 生成に使うモデルを読み込みます。
        //! @param  [in] assetManager アセットマネージャー（果物のモデルを後から読むため保持する）
        void Initialize(Tsukino::Asset::AssetManager& assetManager);

        //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] type       剛体の種類
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                       Tsukino::BuiltIn::ECS::RigidbodyType type);

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

        //! コインの半サイズを返します。
        //! @return コインの各軸の半分サイズ
        static hlslpp::float3 CoinHalfExtent() { return hlslpp::float3(2.5f, 0.4f, 2.5f); }

    private:

        //! 読み込んだモデルと、スケール1での大きさです。
        struct ModelInfo {
            Tsukino::Asset::AssetHandle handle;                                         // モデルのハンドル
            hlslpp::float3              halfExtent = hlslpp::float3(1.0f, 1.0f, 1.0f);    // スケール1での半サイズ
        };

        //! モデルを読み込み、大きさを測ります（同じパスは2回目から使い回す）。
        //! @param  [in] path モデルのパス（リポジトリルート相対）
        //! @return 読み込んだモデルの情報
        const ModelInfo& GetModel(const std::string& path);

        //! マテリアルの基本色だけを差し替えたモデルの複製を返します（同じ組み合わせは使い回す）。
        //! @param  [in] path  元のモデルのパス
        //! @param  [in] color 基本色に掛ける色
        //! @return 複製したモデルの情報（作れなかった場合は元のモデル）
        //! @note   エンジンのモデルには描画時に色を変える手段が無いため、メッシュは共有したまま
        //!         マテリアルだけを差し替えたモデルをアセットとして登録して使う
        const ModelInfo& GetTintedModel(const std::string& path, const hlslpp::float3& color);

        Tsukino::Asset::AssetManager*              m_assetManager = nullptr;    // モデルの読み込みに使う
        std::unordered_map<std::string, ModelInfo> m_models;                    // パスごとのモデル
    };
}    // namespace FruitMagic

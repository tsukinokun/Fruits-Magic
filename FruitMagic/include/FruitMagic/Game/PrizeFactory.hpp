//----------------------------------------------------------------------------
//! @file   PrizeFactory.hpp
//! @brief  台の部品と景品のエンティティ生成
//! @detail 見た目（モデル）・コライダー・剛体をまとめて付けたエンティティを生成します。
//!         シーンの初期配置とシステム（コインの投入など）の両方から使うため、
//!         Registry のコンテキストに置いて共有します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>

#include <hlsl++.h>

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

    //! 台の部品と景品のエンティティを生成するクラスです。
    class PrizeFactory {
    public:

        //! 生成に使うモデルを読み込みます。
        //! @param  [in] assetManager アセットマネージャー
        void Initialize(Tsukino::Asset::AssetManager& assetManager);

        //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] type       剛体の種類
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                       Tsukino::BuiltIn::ECS::RigidbodyType type) const;

        //! 見た目だけの箱（コライダー無し）を生成します。
        //! @param  [in] registry   生成先のレジストリ
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] opacity    不透明度（0〜1）
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateVisualBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                             float opacity = 1.0f) const;

        //! 景品のコインを生成します。
        //! @param  [in] registry 生成先のレジストリ
        //! @param  [in] position 中心のワールド座標
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateCoin(Tsukino::ECS::Registry& registry, const hlslpp::float3& position) const;

        //! 果物の代わりの球を生成します。
        //! @param  [in] registry 生成先のレジストリ
        //! @param  [in] position 中心のワールド座標
        //! @param  [in] radius   半径
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateFruit(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, float radius) const;

        //! 箱モデルの見た目を、指定した大きさに合わせるスケールを求めます。
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @return TransformComponent に設定するスケール
        static hlslpp::float3 BoxScale(const hlslpp::float3& halfExtent);

        //! コインの半サイズを返します。
        //! @return コインの各軸の半分サイズ
        static hlslpp::float3 CoinHalfExtent() { return hlslpp::float3(2.5f, 0.4f, 2.5f); }

    private:
        Tsukino::Asset::AssetHandle m_blockModel;                // 箱の見た目に使うモデル
        Tsukino::Asset::AssetHandle m_ballModel;                 // 球の見た目に使うモデル
        float                       m_ballModelRadius = 1.0f;    // 球モデルのスケール1での半径（読み込み時にメッシュから求める）
    };
}    // namespace FruitMagic

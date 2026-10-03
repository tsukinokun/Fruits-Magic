//----------------------------------------------------------------------------
//! @file   PusherScene.hpp
//! @brief  コインプッシャー台のシーン
//! @detail プレイフィールド（床・側壁・背面パネル）、前後に往復するプッシャー、
//!         仮の景品（コインと果物の代わりの球）を配置した、プッシャー開発の土台となるシーンです。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/EngineIntegration/Scene/GameSceneBase.hpp>
#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>

#include <hlsl++.h>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! コインプッシャー台のシーンです。
    class PusherScene : public Tsukino::EngineIntegration::GameSceneBase {
    public:

        //! コンストラクタです。
        PusherScene() = default;

        //! デストラクタです。
        ~PusherScene() override = default;

        //! シーンを更新します。
        //! @param  [in] api       エンジンから提供されるAPIへの参照
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) override;

        //! シーンの終了処理を行います。
        void OnExit() override;

    private:

        //! シーン固有の初期化処理を行います。
        //! @param  [in] api エンジンから提供されるAPIへの参照
        void OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) override;

        //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
        //! @param  [in] position   中心のワールド座標
        //! @param  [in] halfExtent 各軸の半分サイズ
        //! @param  [in] type       剛体の種類
        //! @param  [in] visible    見た目を描画するか
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateBox(const hlslpp::float3& position, const hlslpp::float3& halfExtent, Tsukino::BuiltIn::ECS::RigidbodyType type, bool visible = true);

        //! 球型の物体（見た目＋Sphereコライダー＋Dynamic剛体）を生成します。
        //! @param  [in] position 中心のワールド座標
        //! @param  [in] radius   半径
        //! @return 生成したエンティティ
        Tsukino::ECS::Entity CreateSphere(const hlslpp::float3& position, float radius);

        Tsukino::Asset::AssetHandle m_blockModel;                  // 箱の見た目に使うモデル
        Tsukino::Asset::AssetHandle m_ballModel;                   // 球の見た目に使うモデル
        float                       m_ballModelRadius = 1.0f;      // 球モデルのスケール1での半径（ロード時にメッシュから求める）

        Tsukino::ECS::Entity m_pusherEntity{entt::null};    // 往復するプッシャー
        float                m_pusherTime = 0.0f;          // プッシャーの往復に使う経過時間（秒）
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   PusherScene.hpp
//! @brief  コインプッシャー台のシーン
//! @detail プレイフィールド（床・側壁・背面パネル）、前後に往復するプッシャー、景品、
//!         コインの投入口と HUD を配置し、ゲームのシステムを登録します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/EngineIntegration/Scene/GameSceneBase.hpp>
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    class PrizeFactory;    // 前方宣言

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

        //! シーンの終了処理を行います。セーブします。
        void OnExit() override;

    private:

        //! シーン固有の初期化処理を行います。
        //! @param  [in] api エンジンから提供されるAPIへの参照
        void OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) override;

        //! 筐体（床・壁・背面パネル・景品受け）とプッシャーを生成します。
        //! @param  [in] factory 生成に使うファクトリ
        void CreateCabinet(PrizeFactory& factory);

        //! 屋台の飾り（しましまの屋根・柱・ちょうちん）を生成します。
        //! @param  [in] factory 生成に使うファクトリ
        void CreateStall(PrizeFactory& factory);

        //! 起動時に台に置いておく景品を生成します。
        //! @param  [in] factory 生成に使うファクトリ
        void CreateInitialPrizes(PrizeFactory& factory);

        //! ライト・空・カメラを生成します。
        void CreateEnvironment();

        //! コインの投入口と HUD を生成します。
        //! @param  [in] factory 生成に使うファクトリ
        void CreatePlayerInterface(PrizeFactory& factory);

        Tsukino::ECS::Entity m_pusherEntity{entt::null};    // 往復するプッシャー
        float                m_pusherTime = 0.0f;          // プッシャーの往復に使う経過時間（秒）
        float                m_pusherAmplitude = 0.0f;     // 今のプッシャーの振幅（cm）。押し幅の強化の値へ少しずつ近づける
        bool                 m_skipSaveOnExit = false;     // 終了時にセーブしない（「データを消して最初から」でセーブを消した後）
    };
}    // namespace FruitMagic

//----------------------------------------------------------------------------
//! @file   PusherScene.hpp
//! @brief  コインプッシャー台のシーン
//! @detail プレイフィールド（床・側壁・背面パネル）、前後に往復するプッシャー、景品、
//!         コインの投入口と HUD を配置し、ゲームのシステムを登録します。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/MoveRules.hpp>

#include <memory>
#include <optional>
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

        //! おひっこしした直後の屋台として始めるコンストラクタです（始まったら「〇〇の屋台で開店！」の窓を出す）。
        //! @param  [in] moved おひっこしの結果（もらった星のたねとおみやげ）
        explicit PusherScene(const MoveResult& moved) : m_moved(moved) {}

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

        //! 起動時に台に置いておく景品を生成します（コインは CoinScatter で散らばらせる）。
        //! @param  [in] factory 生成に使うファクトリ
        void CreateInitialPrizes(PrizeFactory& factory);

        //! 起動時に置いた景品を、物理だけを動かして落ち着かせます（読み込み中。重なったコインが自然に崩れて止まる）。
        //! 台から落ちた物は報酬なしで消します。
        void SettleInitialPrizes();

        //! ライト・空・カメラを生成します。
        void CreateEnvironment();

        //! コインの投入口と HUD を生成します。
        //! @param  [in] factory 生成に使うファクトリ
        void CreatePlayerInterface(PrizeFactory& factory);

        Tsukino::ECS::Entity m_pusherEntity{entt::null};    // 往復するプッシャー
        std::shared_ptr<Tsukino::ECS::ISystem> m_transformSystem;    // 物理の前の Transform（起動時に景品を落ち着かせるのに直接呼ぶ）
        std::shared_ptr<Tsukino::ECS::ISystem> m_physicsSystem;      // 物理（同上）
        float                m_pusherTime = 0.0f;          // プッシャーの往復のどこまで進んだか（0〜1。1 で1往復）
        float                m_pusherAmplitude = 0.0f;     // 今のプッシャーの振幅（cm）。押し幅の強化の値へ少しずつ近づける
        bool                 m_skipSaveOnExit = false;     // 終了時にセーブしない（「データを消して最初から」でセーブを消した後）
        std::optional<MoveResult> m_moved;                 // おひっこしの結果（おひっこし直後の屋台のときだけ）
    };
}    // namespace FruitMagic

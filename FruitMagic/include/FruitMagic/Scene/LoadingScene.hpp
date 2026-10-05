//----------------------------------------------------------------------------
//! @file   LoadingScene.hpp
//! @brief  起動時のロード画面のシーン
//! @detail アセットを裏スレッドで先読みする間、進み具合のバーを表示し、
//!         読み終えたらコインプッシャー台のシーンへ切り替えます。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/AssetPreloader.hpp>

#include <Tsukino/EngineIntegration/Scene/GameSceneBase.hpp>
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 起動時のロード画面のシーンです。
    class LoadingScene : public Tsukino::EngineIntegration::GameSceneBase {
    public:

        //! コンストラクタです。
        LoadingScene() = default;

        //! デストラクタです。
        ~LoadingScene() override = default;

        //! シーンを更新します。読み終えたらコインプッシャー台のシーンへ切り替えます。
        //! @param  [in] api       エンジンから提供されるAPIへの参照
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) override;

        //! シーンの終了処理を行います。先読みが残っていれば打ち切ります。
        void OnExit() override;

    private:

        //! シーン固有の初期化処理を行います。
        //! @param  [in] api エンジンから提供されるAPIへの参照
        void OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) override;

        //! カメラ・背景・進み具合のバー・文字を生成します。
        void CreateScreen();

        AssetPreloader       m_preloader;                // アセットの先読み
        Tsukino::ECS::Entity m_barEntity{entt::null};    // 進み具合のバーの中身
        Tsukino::ECS::Entity m_textEntity{entt::null};   // 「読み込み中」の文字
        float                m_elapsed  = 0.0f;          // 表示してからの時間（秒）
        bool                 m_changing = false;         // 次のシーンへの切り替えを頼んだか
    };
}    // namespace FruitMagic

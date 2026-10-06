//----------------------------------------------------------------------------
//! @file   LoadingScene.cpp
//! @brief  起動時のロード画面のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/LoadingScene.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/UiConfig.hpp>
#include <FruitMagic/Game/UiFonts.hpp>
#include <FruitMagic/Scene/PusherScene.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cmath>
#include <memory>
#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! シーン固有の初期化処理を行います。
    //----------------------------------------------------------------------------
    void LoadingScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI&) {
        Tsukino::EngineIntegration::EngineContext* context = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();

        enum class SystemPriority : int {
            Transform = 0,
            Camera,
            Font,
            Sprite,
        };
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), (int)SystemPriority::Transform);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), (int)SystemPriority::Camera);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), (int)SystemPriority::Font);
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), (int)SystemPriority::Sprite);

        // 定義データの場所は PusherScene と同じ（Debug は作業ディレクトリ、Release は exe の隣が基準）
        const std::string dataRoot = (Tsukino::IO::FileSystem::GetAssetRootPath() / "Assets/Data").string();

        // この画面の配置と文言だけは先に（本体スレッドで）読む。小さなファイルなので待たされない
        m_scene.GetRegistry().SetContext<UiConfig>().Load(dataRoot + "/Ui.json");
        m_scene.GetRegistry().SetContext<Texts>().Load(dataRoot + "/Texts.json");

        // フォントもここで読む（初回は ttf をキャッシュへ変換する。台のシーンではキャッシュから取るだけになる）
        m_scene.GetRegistry().SetContext<UiFonts>().Load(*context->assetManager, GetUiConfig(m_scene.GetRegistry()));

        CreateScreen();

        m_preloader.Start(*context->assetManager, dataRoot);

        Tsukino::Core::Log::Info("LoadingScene: initialized.");
    }

    //----------------------------------------------------------------------------
    //! シーンを更新します。
    //----------------------------------------------------------------------------
    void LoadingScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        m_elapsed += deltaTime;

        //--------------------------------------------------------------
        // 進み具合をバーと文字に反映する（バーは左端を揃えたまま伸ばす）
        //--------------------------------------------------------------
        const float             progress = m_preloader.Progress();
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const UiConfig&         ui       = GetUiConfig(registry);
        if(m_barEntity != entt::null) {
            const float width = ui.loadingBarWidth * progress;
            auto&       t     = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_barEntity);
            t.position        = hlslpp::float3((ui.screenWidth - ui.loadingBarWidth) * 0.5f + width * 0.5f, ui.loadingBarCenterY, 0.0f);
            t.scale           = hlslpp::float3(width / AssetPaths::kWhiteTextureSize, ui.loadingBarHeight / AssetPaths::kWhiteTextureSize, 1.0f);
            t.dirty           = true;
        }
        if(m_textEntity != entt::null) {
            const int percent = static_cast<int>(std::floor(progress * 100.0f));
            registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(m_textEntity).text = GetTexts(registry).Format("loading.progress", {{"n", std::to_wstring(percent)}});
        }

        //--------------------------------------------------------------
        // 読み終えたら台のシーンへ（切り替えは次のフレームの頭で行われる）
        //--------------------------------------------------------------
        if(!m_changing && m_preloader.IsFinished() && m_elapsed >= ui.loadingMinSeconds) {
            m_changing = true;
            api.ChangeScene(std::make_unique<PusherScene>());
        }

        m_scene.Update(deltaTime);
    }

    //----------------------------------------------------------------------------
    //! シーンの終了処理を行います。
    //----------------------------------------------------------------------------
    void LoadingScene::OnExit() {
        m_preloader.Stop();
        Tsukino::Core::Log::Info("LoadingScene: exiting.");
    }

    //----------------------------------------------------------------------------
    //! カメラ・背景・進み具合のバー・文字を生成します。
    //----------------------------------------------------------------------------
    void LoadingScene::CreateScreen() {
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        const UiConfig&         ui       = GetUiConfig(registry);
        const float             barLeft  = (ui.screenWidth - ui.loadingBarWidth) * 0.5f;

        //--------------------------------------------------------------
        // カメラ。画面スプライトはメインでない正射影のカメラで描かれるが、
        // メインのカメラも無いと描画の準備が整わないので両方置く（PusherScene と同じ構成）
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(0.0f, 0.0f, -10.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& cam = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            cam.useLookAt                               = true;
            cam.lookAtTarget                            = hlslpp::float3(0.0f, 0.0f, 0.0f);
            cam.isPrimary                               = true;
        }
        {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(0.0f, 0.0f, -10.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CameraComponent& camera = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(e);
            camera.projectionType                          = Tsukino::BuiltIn::ECS::CameraComponent::ProjectionType::Orthographic;
            camera.orthoSize                               = ui.screenHeight;
            camera.isPrimary                               = false;
        }

        //--------------------------------------------------------------
        // 背景とバー。この画面で使う白い画像だけは先に（本体スレッドで）読む。8px の小さな画像なので待たされない
        //--------------------------------------------------------------
        Tsukino::EngineIntegration::EngineContext* context = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        const Tsukino::Asset::AssetHandle          white   = context->assetManager->Load(Tsukino::Core::Path(AssetPaths::kWhiteTexture));

        auto createPanel = [&](const hlslpp::float2& center, const hlslpp::float2& size, const hlslpp::float4& color, int sortOrder) {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(center.x, center.y, 0.0f);
            t.scale = hlslpp::float3(size.x / AssetPaths::kWhiteTextureSize, size.y / AssetPaths::kWhiteTextureSize, 1.0f);
            t.dirty = true;

            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e);
            sprite.textureHandle                           = white;
            sprite.tintColor                               = color;
            sprite.sortOrder                               = sortOrder;
            return e;
        };

        // 背景（屋台の夕暮れに合わせた濃い紫）
        createPanel(ui.ScreenCenter(), hlslpp::float2(ui.screenWidth, ui.screenHeight), ui.loadingBackColor, 0);

        // バーの枠と中身（中身の幅は OnUpdate が進み具合に合わせて変える）
        createPanel(hlslpp::float2(ui.ScreenCenter().x, ui.loadingBarCenterY),
                    hlslpp::float2(ui.loadingBarWidth + ui.loadingFramePadding, ui.loadingBarHeight + ui.loadingFramePadding), ui.loadingFrameColor, 1);
        m_barEntity = createPanel(hlslpp::float2(barLeft, ui.loadingBarCenterY), hlslpp::float2(0.0f, ui.loadingBarHeight), ui.loadingFillColor, 2);

        //--------------------------------------------------------------
        // 「読み込み中」の文字（バーの上）
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(ui.ScreenCenter().x, ui.loadingTextY, 0.0f);
            t.scale                                      = hlslpp::float3(ui.loadingText.scale, ui.loadingText.scale, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.fontHandle                            = GetUiFont(registry, ui.loadingText.bold);
            font.maxWidth                              = ui.screenWidth - ui.textPadding * 2.0f;
            font.text                                  = GetTexts(registry).Format("loading.progress", {{"n", L"0"}});
            font.color                                 = ui.loadingText.color;
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = ui.textOutlineColor;
            font.outlineWidth                          = ui.outlineWidth;
            font.sortOrder                             = 3;
            m_textEntity                               = e;
        }
    }
}    // namespace FruitMagic

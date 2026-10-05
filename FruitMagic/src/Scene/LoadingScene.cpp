//----------------------------------------------------------------------------
//! @file   LoadingScene.cpp
//! @brief  起動時のロード画面のシーンの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/LoadingScene.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
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
    namespace {
        //--------------------------------------------------------------
        // 画面の配置（画面ピクセル。画面は 1280 x 720）
        //--------------------------------------------------------------
        constexpr float kScreenWidth  = 1280.0f;    // 画面の幅
        constexpr float kScreenHeight = 720.0f;     // 画面の高さ
        constexpr float kBarWidth     = 560.0f;     // バーの全幅
        constexpr float kBarHeight    = 18.0f;      // バーの高さ
        constexpr float kBarCenterY   = 400.0f;     // バーの中心の高さ
        constexpr float kTextY        = 350.0f;     // 文字の中心の高さ

        //! @brief ロード画面を出しておく最短の時間（秒）。キャッシュが効いて一瞬で読み終えたときのちらつきを防ぐ
        constexpr float kMinDisplaySeconds = 0.3f;

        //! @brief バーの左端
        constexpr float BarLeft() { return (kScreenWidth - kBarWidth) * 0.5f; }
    }    // namespace

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

        CreateScreen();

        // 定義データの場所は PusherScene と同じ（Debug は作業ディレクトリ、Release は exe の隣が基準）
        const std::string dataRoot = (Tsukino::IO::FileSystem::GetAssetRootPath() / "Assets/Data").string();
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
        const float            progress = m_preloader.Progress();
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        if(m_barEntity != entt::null) {
            const float width = kBarWidth * progress;
            auto&       t     = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(m_barEntity);
            t.position        = hlslpp::float3(BarLeft() + width * 0.5f, kBarCenterY, 0.0f);
            t.scale           = hlslpp::float3(width / AssetPaths::kWhiteTextureSize, kBarHeight / AssetPaths::kWhiteTextureSize, 1.0f);
            t.dirty           = true;
        }
        if(m_textEntity != entt::null) {
            const int percent = static_cast<int>(std::floor(progress * 100.0f));
            registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(m_textEntity).text = L"読み込み中… " + std::to_wstring(percent) + L"%";
        }

        //--------------------------------------------------------------
        // 読み終えたら台のシーンへ（切り替えは次のフレームの頭で行われる）
        //--------------------------------------------------------------
        if(!m_changing && m_preloader.IsFinished() && m_elapsed >= kMinDisplaySeconds) {
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
            camera.orthoSize                               = kScreenHeight;
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
        createPanel(hlslpp::float2(kScreenWidth * 0.5f, kScreenHeight * 0.5f), hlslpp::float2(kScreenWidth, kScreenHeight),
                    hlslpp::float4(0.12f, 0.06f, 0.18f, 1.0f), 0);

        // バーの枠と中身（中身の幅は OnUpdate が進み具合に合わせて変える）
        createPanel(hlslpp::float2(kScreenWidth * 0.5f, kBarCenterY), hlslpp::float2(kBarWidth + 6.0f, kBarHeight + 6.0f),
                    hlslpp::float4(0.05f, 0.02f, 0.08f, 1.0f), 1);
        m_barEntity = createPanel(hlslpp::float2(BarLeft(), kBarCenterY), hlslpp::float2(0.0f, kBarHeight), hlslpp::float4(1.0f, 0.75f, 0.35f, 1.0f), 2);

        //--------------------------------------------------------------
        // 「読み込み中」の文字（バーの上）
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity                       e = m_scene.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = hlslpp::float3(kScreenWidth * 0.5f, kTextY, 0.0f);
            t.scale                                      = hlslpp::float3(1.1f, 1.1f, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(e);
            font.text                                  = L"読み込み中… 0%";
            font.color                                 = hlslpp::float4(1.0f, 0.92f, 0.85f, 1.0f);
            font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
            font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
            font.outlineColor                          = hlslpp::float4(0.1f, 0.03f, 0.15f, 1.0f);
            font.outlineWidth                          = 2.0f;
            font.sortOrder                             = 3;
            m_textEntity                               = e;
        }
    }
}    // namespace FruitMagic

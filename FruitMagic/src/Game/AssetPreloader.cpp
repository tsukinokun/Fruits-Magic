//----------------------------------------------------------------------------
//! @file   AssetPreloader.cpp
//! @brief  アセットを裏スレッドで先読みするクラスの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/AssetPreloader.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/ECS/System/SoundSystem.hpp>

#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <Windows.h>
#include <algorithm>
#include <chrono>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //----------------------------------------------------------------------------
    //! デストラクタです。
    //----------------------------------------------------------------------------
    AssetPreloader::~AssetPreloader() {
        Stop();
    }

    //----------------------------------------------------------------------------
    //! 裏スレッドで先読みを始めます。
    //----------------------------------------------------------------------------
    void AssetPreloader::Start(Tsukino::Asset::AssetManager& assetManager, const std::string& dataRoot) {
        Stop();
        m_loaded   = 0;
        m_total    = 0;
        m_finished = false;
        m_thread   = std::jthread([this, &assetManager, dataRoot](std::stop_token stopToken) { Run(stopToken, assetManager, dataRoot); });
    }

    //----------------------------------------------------------------------------
    //! 読み込みを打ち切り、裏スレッドの終わりを待ちます。
    //----------------------------------------------------------------------------
    void AssetPreloader::Stop() {
        if(!m_thread.joinable())
            return;
        m_thread.request_stop();
        m_thread.join();
    }

    //----------------------------------------------------------------------------
    //! 進み具合を返します。
    //----------------------------------------------------------------------------
    float AssetPreloader::Progress() const {
        if(m_finished)
            return 1.0f;
        const int total = m_total;
        if(total <= 0)
            return 0.0f;
        return std::clamp(static_cast<float>(m_loaded) / static_cast<float>(total), 0.0f, 1.0f);
    }

    //----------------------------------------------------------------------------
    //! 読み込むアセットのパスを集めます。
    //----------------------------------------------------------------------------
    std::vector<std::string> AssetPreloader::CollectPaths(const std::string& dataRoot) {
        std::vector<std::string> paths = {AssetPaths::kBlockModel, AssetPaths::kBallModel, AssetPaths::kWhiteTexture, AssetPaths::kSparkleTexture,
                                          AssetPaths::kGlowTexture, AssetPaths::kRingTexture, AssetPaths::kCoinIconTexture, AssetPaths::kFpIconTexture};
        auto add = [&](const std::string& path) {
            if(!path.empty() && std::find(paths.begin(), paths.end(), path) == paths.end())
                paths.push_back(path);
        };

        // 果物のモデル（定義データで差し替えられる）
        FruitCatalog catalog;
        if(catalog.Load(dataRoot)) {
            for(const FruitDef& def : catalog.Fruits())
                add(def.modelPath);
        }

        // 効果音
        for(const std::string& file : ECS::SoundSystem::ListSoundFiles(dataRoot + "/Sounds.json"))
            add(file);

        return paths;
    }

    //----------------------------------------------------------------------------
    //! 裏スレッドの本体です。
    //----------------------------------------------------------------------------
    void AssetPreloader::Run(std::stop_token stopToken, Tsukino::Asset::AssetManager& assetManager, const std::string& dataRoot) {
        // テクスチャの変換（WIC）は COM を使うので、このスレッドでも初期化しておく
        const HRESULT comResult = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        const auto                     startTime = std::chrono::steady_clock::now();
        const std::vector<std::string> paths     = CollectPaths(dataRoot);
        m_total                                  = static_cast<int>(paths.size());

        int failed = 0;
        for(const std::string& path : paths) {
            if(stopToken.stop_requested())
                break;
            const Tsukino::Asset::AssetHandle handle = assetManager.Load(Tsukino::Core::Path(path));
            if(!handle.IsValid()) {
                Tsukino::Core::Log::Warn("AssetPreloader: cannot load " + path + ".");
                ++failed;
            }
            ++m_loaded;
        }

        const long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
        Tsukino::Core::Log::Info("AssetPreloader: loaded " + std::to_string(m_loaded.load() - failed) + " / " + std::to_string(paths.size()) + " assets in " +
                                 std::to_string(elapsedMs) + " ms.");

        if(SUCCEEDED(comResult))
            ::CoUninitialize();
        m_finished = true;
    }
}    // namespace FruitMagic

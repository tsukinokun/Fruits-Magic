//----------------------------------------------------------------------------
//! @file   WinMain.cpp
//! @brief  FruitMagic のエントリポイント
//----------------------------------------------------------------------------
#include <FruitMagic/Scene/PusherScene.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineIntegration.hpp>
#include <Tsukino/Core/Log.hpp>

#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <memory>

//----------------------------------------------------------------------------
//! アプリケーションのエントリポイントです。
//! @param  [in] hInstance     アプリケーションインスタンス
//! @param  [in] hPrevInstance 非推奨（常にNULL）
//! @param  [in] lpCmdLine     コマンドライン引数
//! @param  [in] nCmdShow      ウィンドウ表示状態
//! @return 終了コード
//----------------------------------------------------------------------------
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    // 高DPIディスプレイでぼやけないよう、モニタごとのDPIを自前で扱う
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // ログをファイルへ出す（呼ばないとデバッガの外では警告が一切見えない）
    Tsukino::Core::Log::SetLogFile("Logs/FruitMagic.log");

    Tsukino::EngineIntegration::EngineIntegration engineIntegration;
    if(!engineIntegration.Initialize(1280, 720, "FruitMagic")) {
        Tsukino::Core::Log::Error("Failed to initialize EngineIntegration.");
        return EXIT_FAILURE;
    }

    Tsukino::EngineIntegration::EngineContext& engineContext = engineIntegration.GetContext();
    Tsukino::EngineIntegration::EngineAPI      engineAPI(engineContext);

    // 最初のシーン
    engineAPI.ChangeScene(std::make_unique<FruitMagic::PusherScene>());

    //--------------------------------------------------------------
    // メインループ
    // deltaTime は実測値を使う（固定値だと高リフレッシュレート環境で速く進む）
    //--------------------------------------------------------------
    auto lastTime = std::chrono::steady_clock::now();

    while(engineAPI.ProcessMessages()) {
        auto  currentTime = std::chrono::steady_clock::now();
        float deltaTime   = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime          = currentTime;
        // ウィンドウドラッグ等で1フレームが極端に長くなった場合の暴走防止
        deltaTime = std::min(deltaTime, 1.0f / 15.0f);

        engineAPI.Update(deltaTime);
        engineAPI.Render();
    }

    return 0;
}

//----------------------------------------------------------------------------
//! @file   StageConfig.hpp
//! @brief  台と屋台の見た目・光・カメラの設定（Assets/Data/Stage.json）
//! @detail 当たり判定や遊びに関わらない、見た目だけの値をまとめます。無い項目は既定値（調整済みの値）のままです。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 輪郭の光り方です（RimGlowComponent に入れる値）。
    struct RimGlowStyle {
        hlslpp::float3 color     = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 光の色
        float          intensity = 1.0f;                                 // 輪郭の光の強さ
        float          glow      = 0.0f;                                 // 全体の白い光（強くすると白く飛ぶ）
    };

    //! 見た目だけのモデルを1つ置く設定です（原点の位置・向き・拡大率）。
    struct StageProp {
        std::string    model;                                             // モデル（リポジトリルート相対）
        hlslpp::float3 position = hlslpp::float3(0.0f, 0.0f, 0.0f);      // モデルの原点の位置
        hlslpp::float3 rotation = hlslpp::float3(0.0f, 0.0f, 0.0f);      // 向き（度。X → Y → Z の順に回す）
        hlslpp::float3 scale    = hlslpp::float3(1.0f, 1.0f, 1.0f);      // 拡大率
        RimGlowStyle   glow     = {hlslpp::float3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f};    // 光り方（強さ 0 なら光らない）
    };

    //! 台と屋台の見た目・光・カメラの設定です。Registry のコンテキストに置きます。
    struct StageConfig {
        //--------------------------------------------------------------
        // 筐体の色（パステル）
        //--------------------------------------------------------------
        hlslpp::float3 floorColor     = hlslpp::float3(1.0f, 0.95f, 0.84f);     // 床（クリーム）
        hlslpp::float3 sideWallColor  = hlslpp::float3(1.0f, 0.74f, 0.82f);     // 側壁（ピンク）
        hlslpp::float3 backPanelColor = hlslpp::float3(0.68f, 0.93f, 0.84f);    // 背面パネル（ミント）
        hlslpp::float3 pusherColor    = hlslpp::float3(0.62f, 0.84f, 1.0f);     // プッシャー（空色。金のコインと見分けやすく）
        hlslpp::float3 trayColor      = hlslpp::float3(0.84f, 0.78f, 1.0f);     // 手前の景品受け（ラベンダー）
        hlslpp::float3 gutterColor    = hlslpp::float3(0.55f, 0.47f, 0.72f);    // 横の溝（濃いめ）

        //--------------------------------------------------------------
        // しましまの屋根
        //--------------------------------------------------------------
        int            awningStripes         = 8;                                       // しまの数
        float          awningY               = 45.0f;                                   // 中心の高さ
        float          awningZ               = -42.0f;                                  // 中心のZ
        float          awningHalfDepth       = 12.0f;                                   // 奥行の半分
        float          awningTiltDegrees     = 18.0f;                                   // 手前へ下げる角度（度）
        float          awningOverhang        = 6.0f;                                    // 台の左右へはみ出す幅
        float          awningHalfThickness   = 0.6f;                                    // 屋根の厚みの半分
        hlslpp::float3 awningRedColor        = hlslpp::float3(0.95f, 0.38f, 0.42f);    // 赤のしま
        hlslpp::float3 awningWhiteColor      = hlslpp::float3(1.0f, 0.97f, 0.92f);     // 白のしま
        float          awningBallRadius      = 1.6f;                                    // 手前の縁に並べる玉の半径
        float          awningBallDrop        = 1.2f;                                    // 玉を縁から下げる量

        //--------------------------------------------------------------
        // 柱
        //--------------------------------------------------------------
        float          postInset    = 1.5f;                                    // 屋根の端から内側へ
        float          postBack     = 1.0f;                                    // 屋根の手前の縁から奥へ
        float          postHalfSize = 1.2f;                                    // 太さの半分
        hlslpp::float3 postColor    = hlslpp::float3(0.78f, 0.56f, 0.36f);    // 木の色

        //--------------------------------------------------------------
        // 屋台の飾り（見た目だけのモデル。電飾など）
        //--------------------------------------------------------------
        std::vector<StageProp> props;    // 置くモデル（Stage.json の "props"）

        //--------------------------------------------------------------
        // ちょうちん（モデルと、温かい色の点光源）
        //--------------------------------------------------------------
        std::string                 lanternModel;                                            // モデル（空なら置かない）
        std::vector<hlslpp::float3> lanternPositions;                                        // 置く位置（モデルの原点の位置。吊り下げ型は吊るす点）
        hlslpp::float3              lanternRotation       = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 向き（度）
        hlslpp::float3              lanternScale          = hlslpp::float3(10.0f, 10.0f, 10.0f); // 拡大率
        RimGlowStyle                lanternGlow           = {hlslpp::float3(1.0f, 0.8f, 0.4f), 0.8f, 0.25f};    // 光り方（強くすると白く飛んで色が分からなくなる）
        hlslpp::float3              lanternLightOffset    = hlslpp::float3(0.0f, -4.0f, 0.0f);   // 点光源の位置（置く位置から）
        hlslpp::float3              lanternLightColor     = hlslpp::float3(1.0f, 0.65f, 0.35f);  // 点光源の色
        float                       lanternLightIntensity = 900.0f;                              // 点光源の強さ
        float                       lanternLightRange     = 70.0f;                               // 点光源の届く距離

        //--------------------------------------------------------------
        // ライト（減衰は intensity / (d^2 + 1) なので、点光源は距離の2乗のオーダーにする）
        //--------------------------------------------------------------
        hlslpp::float3 sunDirection = hlslpp::float3(-0.4f, -0.7f, -0.6f);    // ディレクショナルライトの向き（少し低い夕方の日差し）
        hlslpp::float3 sunColor     = hlslpp::float3(1.0f, 0.88f, 0.75f);     // その色
        float          sunIntensity = 1.5f;                                   // その強さ
        bool           sunShadow    = true;                                   // 影を落とすか
        hlslpp::float3 lampPosition  = hlslpp::float3(0.0f, 60.0f, 0.0f);     // 筐体の上の点光源の位置
        hlslpp::float3 lampColor     = hlslpp::float3(1.0f, 0.85f, 0.6f);     // その色
        float          lampIntensity = 6000.0f;                               // その強さ
        float          lampRange     = 200.0f;                                // その届く距離

        //--------------------------------------------------------------
        // 台の周りを漂う光の粒（カメラを中心に折り返すので、台の大きさに合わせて狭く・少なめに）
        //--------------------------------------------------------------
        int            particleCount     = 160;
        hlslpp::float3 particleVolume    = hlslpp::float3(260.0f, 140.0f, 260.0f);
        hlslpp::float3 particleColor     = hlslpp::float3(1.0f, 0.85f, 0.55f);
        float          particleMinSize   = 0.25f;
        float          particleMaxSize   = 0.7f;
        hlslpp::float3 particleDrift     = hlslpp::float3(1.5f, 0.8f, 0.0f);
        float          particleSway      = 3.0f;
        float          particleNearFade  = 25.0f;

        //--------------------------------------------------------------
        // カメラ（プレイヤーの目線：手前斜め上から台を見下ろす）
        //--------------------------------------------------------------
        hlslpp::float3 cameraPosition      = hlslpp::float3(0.0f, 75.0f, 105.0f);
        hlslpp::float3 cameraLookAt        = hlslpp::float3(0.0f, 0.0f, -5.0f);
        float          cameraNear          = 1.0f;
        float          cameraFar           = 5000.0f;
        hlslpp::float3 debugCameraPosition = hlslpp::float3(60.0f, 60.0f, 80.0f);    // Debug ビルドのデバッグカメラ
        hlslpp::float3 debugCameraLookAt   = hlslpp::float3(0.0f, 0.0f, 0.0f);
        float          debugCameraSpeed    = 100.0f;
        float          debugCameraSprint   = 300.0f;

        //--------------------------------------------------------------
        // 目印
        //--------------------------------------------------------------
        float          launchMarkerHalfThickness = 0.2f;     // 投入口の目印（半透明の板）の厚みの半分
        float          launchMarkerOpacity       = 0.5f;     // その不透明度
        float          checkerMarkerY            = -3.0f;    // チェッカーの目印の高さ
        float          checkerMarkerOffsetZ      = 3.0f;     // 台の手前端からの距離
        float          checkerMarkerHalfThickness = 0.3f;    // 厚みの半分
        float          checkerMarkerHalfDepth    = 2.5f;     // 奥行の半分
        RimGlowStyle   checkerMarkerGlow         = {hlslpp::float3(1.0f, 0.7f, 0.1f), 1.0f, 0.4f};    // 台の上で見失わないよう金色に光らせる

        //--------------------------------------------------------------
        // 景品の見た目
        //--------------------------------------------------------------
        hlslpp::float3 coinColor    = hlslpp::float3(1.0f, 0.78f, 0.25f);    // コインの色（金色）
        float          coinGlow     = 0.25f;                                 // コインの輪郭の光の強さ
        hlslpp::float3 coinTint     = hlslpp::float3(1.0f, 0.92f, 0.6f);     // コインのモデルの色に掛ける色（モデルを使うとき。白ならモデルの色のまま）
        float          prizeRimPower = 3.0f;                                 // 景品（コイン・果物・飾り）の輪郭の光の鋭さ

        //--------------------------------------------------------------
        // 魔法の見た目
        //--------------------------------------------------------------
        hlslpp::float3 wallColor        = hlslpp::float3(0.4f, 0.85f, 1.0f);    // 魔法の壁の色（水色）
        float          wallOpacity      = 0.5f;                                 // その不透明度
        float          wallRimIntensity = 1.5f;                                 // 輪郭の光の強さ
        float          wallGlow         = 0.6f;                                 // 全体の光
        hlslpp::float3 swellColor       = hlslpp::float3(1.0f, 0.45f, 0.7f);    // 「ふくらむ」の間のプッシャーの輪郭の色
        float          swellIntensity   = 1.0f;                                 // 輪郭の光の強さ（脈打つ中心）
        float          swellIntensityPulse = 0.8f;                              // その脈打つ幅
        float          swellGlow        = 0.25f;                                // 全体の光（脈打つ中心）
        float          swellGlowPulse   = 0.15f;                                // その脈打つ幅
        float          swellPulseSpeed  = 8.0f;                                 // 脈打つ速さ（ラジアン/秒）

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Stage.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);
    };

    //! レジストリに置いた見た目の設定を返します。
    //! @param  [in] registry レジストリ
    //! @return 見た目の設定（コンテキストに無ければ既定値）
    const StageConfig& GetStageConfig(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic

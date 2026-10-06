//----------------------------------------------------------------------------
//! @file   TableLayout.hpp
//! @brief  プッシャー台の寸法・配置と、景品の物理（Assets/Data/Table.json）
//! @detail シーンの構築と各システム（投入・落下判定・魔法）で共有する寸法をまとめます。
//!         単位はエンジン規約どおり 1unit ≒ 1cm。奥が -Z、手前（プレイヤー側・落下口）が +Z です。
//!         JSON には元になる値だけを書き、ほかの位置（プッシャーの上面・投入位置など）はここで計算します。
//!         既定値は調整済みの値で、JSON が無いときや寸法が矛盾しているときはこの値で動きます。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic::Layout
namespace FruitMagic::Layout {
    //! @brief 画面の右に当たるワールドの X の向き（+1 か -1）。
    //!        カメラは手前（+Z）から奥（-Z）を見ているので、画面の右はワールドの -X になる。
    //!        プレイヤーの入力（←→・マウス）を画面上の向きに合わせるのに使う（カメラの向きと対なのでデータにしない）
    inline constexpr float kScreenRightX = -1.0f;
}    // namespace FruitMagic::Layout

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! プッシャー台の寸法・配置と、景品の物理です。Registry のコンテキストに置きます。
    struct TableLayout {
        //--------------------------------------------------------------
        // プレイフィールド
        //--------------------------------------------------------------
        float fieldHalfWidth  = 30.0f;      // プレイフィールドの半幅（X）
        float fieldFrontZ     = 40.0f;      // 手前端のZ（ここから先へ押し出された物が落ちる）
        float fieldBackZ      = -105.0f;    // 奥端のZ（プッシャーが引っ込んでも床が切れない位置）
        float staticThickness = 10.0f;      // 床・トレイのコライダーの半分の厚み（すり抜け対策で厚めにする）

        //--------------------------------------------------------------
        // プッシャー
        //--------------------------------------------------------------
        float pusherSideGap        = 0.5f;     // 側壁との隙間（半幅 = フィールドの半幅 - これ）
        float pusherHalfHeight     = 5.0f;     // 半分の高さ（Y）
        float pusherSinkDepth      = 2.0f;     // 床へ埋める深さ。床面に下端の角があるとコインがプッシャーの下へ潜り込むため
        float pusherHalfDepth      = 32.0f;    // 半分の奥行（Z）。押し幅を最大まで強化して前に出ても、後端が背面パネルより奥に残る奥行が要る
        float pusherMinFrontZ      = -37.0f;   // 最も引っ込んだときの前面のZ。押し幅を強化しても変えない（投入位置がここで決まるため）
        float pusherAmplitude      = 12.0f;    // 往復の振幅（強化前）。押し幅の強化で TableStats::pusherAmplitude が増える
        float pusherMaxAmplitude   = 24.0f;    // 振幅の上限（強化＋魔法「ふくらむ」）。データがこれを超えても丸める
        float pusherPeriod         = 3.0f;     // 往復の周期（秒）
        float pusherAmplitudeSpeed = 4.0f;     // 振幅を強化の値へ近づける速さ（cm/秒）。一気に変えると速度が跳ねて景品を弾き飛ばすため

        //--------------------------------------------------------------
        // 背面パネル（プッシャーが引っ込むときに上面の景品を掻き落とす）
        //--------------------------------------------------------------
        float backPanelDistance      = 13.0f;    // プッシャーが最も引っ込んだときの前面から、パネルの中心までの距離
        float backPanelHalfThickness = 1.0f;     // 半分の厚み（Z）
        float backPanelHalfHeight    = 15.0f;    // 半分の高さ（Y）
        float backPanelSink          = 3.0f;     // パネルの下端をプッシャーの上面へ食い込ませる深さ（隙間があるとコインが下をくぐる）

        //--------------------------------------------------------------
        // 落下口・景品受け
        //--------------------------------------------------------------
        float payoutHalfWidth = 20.0f;     // 手前の中央の範囲の半幅（チェッカーが動く範囲・ルーレットの果物を補充する範囲）。落ちた判定には使わない
        float dropJudgeY      = -20.0f;    // これより下に落ちた物を「台から落ちた」と判定する高さ（落ちる様子が見えるようトレイの少し上）
        float trayTopY        = -30.0f;    // 景品受けトレイ上面の高さ
        float trayOffsetZ     = 15.0f;     // フィールドの手前端からトレイの中心までの距離
        float trayHalfDepth   = 18.0f;     // トレイの半分の奥行
        float gutterExtraWidth = 5.0f;     // トレイと横の溝がフィールドの外へはみ出す幅
        float gutterDepth     = 6.0f;      // 横の溝の底がトレイより低い量

        //--------------------------------------------------------------
        // 側壁
        //--------------------------------------------------------------
        float sideWallHalfThickness = 1.0f;     // 半分の厚み（X）
        float sideWallHeight        = 12.0f;    // 床面からの高さの半分
        float sideWallOpenLength    = 24.0f;    // 手前の端から側壁の無い長さ（Z）。ここでは端に寄った景品が横の溝へこぼれる

        //--------------------------------------------------------------
        // コインの投入
        //--------------------------------------------------------------
        float launchLaneHalfWidth = 25.0f;    // 投入位置を動かせる左右の範囲
        float launchDropHeight    = 1.5f;     // プッシャー上面から投入位置までの高さ。高いとプッシャーにめり込んで背面パネルの下へ運ばれる
        float launchMarkerHeight  = 4.0f;     // 投入位置から目印までの高さ（積もったコインに埋もれないよう）
        float minLaunchClearance  = 8.0f;     // 背面パネルの前面とプッシャーの前面の間に要る隙間（コインが落ちる場所）

        //--------------------------------------------------------------
        // コインのシャワー（魔法「メテオコイン」・ジャックポット）を降らせる範囲。
        // プッシャーが最も前に出ても届かない手前側に落とす（届く所に落とすとプッシャーの上や下に入り込む）
        //--------------------------------------------------------------
        float showerBackMargin  = 4.0f;    // プッシャーが最も前に出たときの前面からの余白
        float showerFrontMargin = 4.0f;    // 台の手前端からの余白
        float showerSideMargin  = 1.0f;    // 側壁からの余白
        float showerDropY       = 4.0f;    // 落とす高さ。高い所から落とすと敷き詰めたコインを突き抜けて床へめり込む
        int   showerMaxPerFrame = 3;       // 1フレームに落とす最大の枚数（同じ所に重なって弾け飛ばないように）

        //--------------------------------------------------------------
        // 景品の物理
        //--------------------------------------------------------------
        hlslpp::float3 coinHalfExtent = hlslpp::float3(2.5f, 0.4f, 2.5f);    // コインの半分の大きさ
        float          coinFriction   = 0.3f;    // コインの摩擦
        int            coinValue      = 1;       // コイン1枚の払い出し枚数
        std::string    coinModel;                // コインの見た目のモデル（空なら金色の箱）。当たり判定はどちらも halfExtent の箱
        hlslpp::float3 coinModelRotation = hlslpp::float3(0.0f, 0.0f, 0.0f);    // コインのモデルの向きの補正（度。平たく寝かせるため）
        std::string    coinMaterial;             // コインのモデルに使うマテリアル（.tmat。空ならモデルのマテリアル）
        float          fruitFriction  = 0.5f;    // 果物の摩擦
        float          fruitRestitution = 0.2f;  // 果物の反発

        //! @brief 物理に渡す1フレームの経過時間の上限（秒）。重いフレームで一気に進めてすり抜けるのを防ぐ
        float maxSimulationStep = 1.0f / 30.0f;

        //! @brief 物理のめり込みを直さずに許す量（cm）。コインの厚み（0.8cm）より大きいと、上に載った果物がコインの層に沈む
        float penetrationSlop = 0.2f;

        //! @brief 物理が離れていても接触を作り始める距離（cm）。小さすぎると速い物が薄いコインをすり抜けやすい
        float speculativeContactDistance = 1.0f;

        //--------------------------------------------------------------
        // 起動時の台の上
        //--------------------------------------------------------------
        float              initialCoinFrontBack = 5.0f;     // コインを敷き始める列の、プッシャーが届く最前の位置からの戻り
        float              initialCoinGap       = 0.3f;     // 敷き詰めるコインの間の隙間
        float              initialCoinSideMargin = 0.5f;    // 側壁との余白
        float              initialLift          = 0.5f;     // 床・コインから浮かせる高さ（めり込み防止）
        int                pusherTopCoinHalfCount = 3;      // プッシャーの上に置くコインの列（中央から左右にこの数）
        float              pusherTopCoinSpacing = 7.0f;     // その間隔
        float              pusherTopCoinOffsetZ = 8.0f;     // プッシャーの中心から手前へずらす量
        std::vector<float> initialFruitX        = {-16.0f, -6.0f, 6.0f, 16.0f};    // 最初から置く果物のX
        float              initialFruitStepZ    = 10.0f;    // 果物を1つおきに奥へずらす量

        //--------------------------------------------------------------
        // ここから下は上の値から計算する位置
        //--------------------------------------------------------------

        //! プッシャーの半幅を返します。
        float PusherHalfWidth() const { return fieldHalfWidth - pusherSideGap; }

        //! プッシャーの中心の高さを返します。
        float PusherCenterY() const { return pusherHalfHeight - pusherSinkDepth; }

        //! プッシャーの上面の高さを返します。
        float PusherTopY() const { return PusherCenterY() + pusherHalfHeight; }

        //! 振幅に対する往復の中心Zを返します。最も引っ込んだ位置は変えず、振幅が増えた分だけ前へ出す。
        //! @param  [in] amplitude 往復の振幅
        //! @return 往復の中心Z
        float PusherCenterZ(float amplitude) const { return pusherMinFrontZ - pusherHalfDepth + amplitude; }

        //! プッシャーの前面が最も前に出たときのZを返します（押し幅を最大まで強化したとき）。
        float PusherMaxFrontZ() const { return pusherMinFrontZ + pusherMaxAmplitude * 2.0f; }

        //! 側壁の手前端のZを返します。ここより手前は左右が開いている。
        float SideWallFrontZ() const { return fieldFrontZ - sideWallOpenLength; }

        //! 背面パネルの中心のZを返します。
        float BackPanelZ() const { return pusherMinFrontZ - backPanelDistance; }

        //! 背面パネルの前面のZを返します。
        float BackPanelFrontZ() const { return BackPanelZ() + backPanelHalfThickness; }

        //! 投入位置のZを返します。背面パネルの前面と、プッシャーが最も引っ込んだときの前面の中間。
        //! ここなら投入したコインは必ずプッシャー上面に落ちる（床へ直接落とすとプッシャーの下に閉じ込められる）
        float LaunchZ() const { return (BackPanelFrontZ() + pusherMinFrontZ) * 0.5f; }

        //! 投入位置の高さを返します。
        float LaunchY() const { return PusherTopY() + launchDropHeight; }

        //! 投入位置の目印の高さを返します。
        float LaunchMarkerY() const { return LaunchY() + launchMarkerHeight; }

        //! 設定ファイルを読み込みます。寸法が矛盾しているときは Warn を出し、すべて既定値に戻します。
        //! @param  [in] path 設定ファイル（Table.json）
        //! @return 読み込めて矛盾も無ければ true
        bool Load(const std::string& path);

        //! 寸法が矛盾していないかを調べます。
        //! @param  [out] reason 矛盾していたときの説明
        //! @return 矛盾が無ければ true
        bool Validate(std::string& reason) const;
    };

    //! レジストリに置いた台の寸法を返します。
    //! @param  [in] registry レジストリ
    //! @return 台の寸法（コンテキストに無ければ既定の寸法）
    const TableLayout& GetTableLayout(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic

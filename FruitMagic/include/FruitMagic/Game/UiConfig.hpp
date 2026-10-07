//----------------------------------------------------------------------------
//! @file   UiConfig.hpp
//! @brief  画面の UI の配置・色・表示時間（Assets/Data/Ui.json）
//! @detail 位置・大きさは画面ピクセル（左上原点。画面は screenWidth x screenHeight）。
//!         描画の重なり順（sortOrder）は UI の層の決まりなのでコードに残します。無い項目は既定値（調整済みの値）のままです。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <string>
#include <unordered_map>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 文字の揃え方です。
    enum class UiAlign {
        Left,
        Center,
        Right,
    };

    //! 文字1つの置き方です。
    struct UiText {
        hlslpp::float2 position = hlslpp::float2(0.0f, 0.0f);          // 位置（揃え方の基準点）
        float          scale    = 1.0f;                                 // 大きさ
        hlslpp::float4 color    = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 色
        UiAlign        align    = UiAlign::Left;                        // 揃え方
        bool           bold     = false;                                // 太字のフォントを使うか（UiFonts）
        float          maxWidth = 0.0f;                                 // 収める幅（超えたら縮める）。0 なら位置と揃え方から画面に収まる幅を決める
    };

    //! 文字の大きさと色です（位置は配置の計算で決まるもの）。
    struct UiFont {
        float          scale = 1.0f;                                     // 大きさ
        hlslpp::float4 color = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 色
        bool           bold  = false;                                     // 太字のフォントを使うか（UiFonts）
    };

    //! スロット（3リール）の置き方と色です。ふだんの小さなスロットと、ジャックポットの大きなスロットで同じ形を使います。
    struct SlotLayout {
        hlslpp::float2 center        = hlslpp::float2(640.0f, 66.0f);                     // 板の中心
        hlslpp::float2 panelSize     = hlslpp::float2(260.0f, 108.0f);                    // 板の大きさ
        hlslpp::float4 panelColor    = hlslpp::float4(0.12f, 0.06f, 0.18f, 0.92f);        // 板の色
        float          windowOffsetY = -6.0f;                                             // 板の中心からリールの窓の中心まで
        hlslpp::float2 windowSize    = hlslpp::float2(70.0f, 74.0f);                      // リールの窓の大きさ（この外の絵柄は切れる）
        float          windowSpacing = 80.0f;                                             // 窓の間隔（中心から中心）
        hlslpp::float4 windowColor   = hlslpp::float4(1.0f, 0.97f, 0.9f, 1.0f);          // 窓の地の色
        float          flashBorder   = 5.0f;                                              // 窓の周りの光る縁の太さ
        hlslpp::float4 flashColor    = hlslpp::float4(1.0f, 0.85f, 0.3f, 1.0f);           // 止まったとき・リーチ・当たりで光る縁の色
        float          symbolSize    = 46.0f;                                             // 絵柄（3D の果物・コイン）の大きさ
        float          symbolPitch   = 56.0f;                                             // 絵柄の縦の間隔
        float          spinSpeed     = 900.0f;                                            // 回っているときの絵柄の速さ（ピクセル/秒）
        float          reachSpeed    = 320.0f;                                            // リーチの3列目の速さ（ゆっくり）
        float          stopSeconds   = 0.25f;                                             // 止まり始めてから止まるまで（少し行き過ぎて戻る）
        float          overshoot     = 1.6f;                                              // 止まるときの行き過ぎの強さ（0 なら行き過ぎない）
        int            sortOrder     = 2;                                                 // 重ね順（板。窓・絵柄・縁はこれより上に順に重ねる）
    };

    //! 画面の UI の配置・色・表示時間です。Registry のコンテキストに置きます。
    struct UiConfig {
        //--------------------------------------------------------------
        // 画面
        //--------------------------------------------------------------
        float screenWidth  = 1280.0f;    // 画面の幅
        float screenHeight = 720.0f;     // 画面の高さ

        //! 画面の中心を返します。
        hlslpp::float2 ScreenCenter() const { return hlslpp::float2(screenWidth * 0.5f, screenHeight * 0.5f); }

        //! 画面全体を覆う板の大きさを返します。ウィンドウの縦横比が UI と違うと、UI は真ん中に寄って
        //! 左右（または上下）に余白ができるので、その余白まで覆えるよう画面の3倍にとります。
        hlslpp::float2 FullScreenCover() const { return hlslpp::float2(screenWidth, screenHeight) * 3.0f; }

        //--------------------------------------------------------------
        // フォント（.dfont。空ならエンジンの標準フォント）。読み込みは UiFonts
        //--------------------------------------------------------------
        std::string fontRegularPath = "Assets/Fonts/Rounded-Medium.dfont";       // ふつう（説明文など読む文字）
        std::string fontBoldPath    = "Assets/Fonts/Rounded-ExtraBold.dfont";    // 太字（数・タイトル・ボタンなど目立たせる文字）

        //--------------------------------------------------------------
        // 文字の縁
        //--------------------------------------------------------------
        hlslpp::float4 hudOutlineColor  = hlslpp::float4(0.15f, 0.08f, 0.05f, 1.0f);    // HUD の文字の縁の色
        hlslpp::float4 textOutlineColor = hlslpp::float4(0.1f, 0.03f, 0.15f, 1.0f);     // 画面（図鑑・強化など）の文字の縁の色
        float          outlineWidth     = 2.0f;                                         // 文字の縁の太さ
        float          textPadding      = 8.0f;                                         // 文字を枠（ボタン・画面・列）に収めるときの内側の余白

        //--------------------------------------------------------------
        // HUD の文字（"coins" "mana" などの種類ごと）
        //--------------------------------------------------------------
        std::unordered_map<std::string, UiText> hudTexts;
        hlslpp::float4 rouletteJackpotColor = hlslpp::float4(1.0f, 0.65f, 0.1f, 1.0f);    // ジャックポットチャンスの間のルーレットの文字の色
        hlslpp::float2 harvestIconPosition  = hlslpp::float2(46.0f, 224.0f);              // 「〇〇 ゲット！」の左の果物（3D）の中心
        float          harvestIconSize      = 36.0f;                                      // その大きさ
        float          harvestIconSpinSpeed = 120.0f;                                     // その回る速さ（度/秒）

        //--------------------------------------------------------------
        // マナゲージ
        //--------------------------------------------------------------
        float          manaGaugeLeft    = 28.0f;     // 左端
        float          manaGaugeCenterY = 100.0f;    // 中心の高さ
        float          manaGaugeWidth   = 220.0f;    // 全幅
        float          manaGaugeHeight  = 14.0f;     // 高さ
        float          manaGaugePadding = 4.0f;      // 背景の、中身より大きい分
        hlslpp::float4 manaGaugeBackColor = hlslpp::float4(0.1f, 0.05f, 0.15f, 0.8f);
        hlslpp::float4 manaGaugeFillColor = hlslpp::float4(0.75f, 0.45f, 1.0f, 1.0f);

        //--------------------------------------------------------------
        // おすそわけ待ちのリング
        //--------------------------------------------------------------
        float          reliefRingDiameter  = 36.0f;
        hlslpp::float2 reliefRingCenter    = hlslpp::float2(46.0f, 268.0f);
        hlslpp::float4 reliefRingBackColor = hlslpp::float4(0.1f, 0.05f, 0.15f, 0.7f);
        hlslpp::float4 reliefRingFillColor = hlslpp::float4(0.75f, 1.0f, 0.55f, 1.0f);

        //--------------------------------------------------------------
        // 魔法ボタン（画面下に横並び）
        //--------------------------------------------------------------
        float          magicButtonWidth    = 200.0f;
        float          magicButtonHeight   = 46.0f;
        float          magicButtonGap      = 16.0f;
        float          magicButtonY        = 640.0f;    // 中心の高さ（横は画面の中央に揃える）
        float          magicLabelScale     = 0.72f;     // 「4 おおきくなーれ 60」がボタンに収まる大きさ
        hlslpp::float4 magicLabelOutline   = hlslpp::float4(0.15f, 0.05f, 0.2f, 1.0f);
        hlslpp::float4 magicLockedColor    = hlslpp::float4(0.35f, 0.35f, 0.4f, 0.8f);     // 未解放・割り当てなし
        hlslpp::float4 magicActiveColor    = hlslpp::float4(0.55f, 0.35f, 0.75f, 0.9f);    // 効果中
        hlslpp::float4 magicNoManaColor    = hlslpp::float4(0.3f, 0.2f, 0.45f, 0.85f);     // マナ不足
        hlslpp::float4 magicHoverColor     = hlslpp::float4(1.0f, 0.7f, 1.0f, 1.0f);       // 撃てる（カーソルが重なっている）
        hlslpp::float4 magicReadyColor     = hlslpp::float4(0.85f, 0.45f, 0.95f, 1.0f);    // 撃てる

        //--------------------------------------------------------------
        // 画面（図鑑・強化）の開閉ボタン（右上）
        //--------------------------------------------------------------
        float          menuButtonX      = 1170.0f;
        float          zukanButtonY     = 40.0f;
        float          upgradeButtonY   = 90.0f;
        float          menuButtonWidth  = 170.0f;
        float          menuButtonHeight = 42.0f;
        float          menuLabelScale   = 0.85f;
        hlslpp::float4 zukanButtonColor   = hlslpp::float4(0.95f, 0.55f, 0.65f, 1.0f);
        hlslpp::float4 upgradeButtonColor = hlslpp::float4(0.45f, 0.75f, 0.4f, 1.0f);
        float          optionsButtonY     = 140.0f;
        hlslpp::float4 optionsButtonColor = hlslpp::float4(0.55f, 0.6f, 0.85f, 1.0f);

        //--------------------------------------------------------------
        // 画面の左右のパネル（左: 台のようす、右: 進み具合とおすすめ）。
        // 位置は板の左上。高さは左右で同じ（中身の行の多い方に合わせる）
        //--------------------------------------------------------------
        hlslpp::float2 sideLeftPosition    = hlslpp::float2(16.0f, 288.0f);     // 左のパネルの左上
        hlslpp::float2 sideRightPosition   = hlslpp::float2(964.0f, 288.0f);    // 右のパネルの左上
        float          sideWidth           = 300.0f;    // パネルの幅
        float          sidePadding         = 12.0f;     // 板の内側の余白
        float          sideHeaderPitch     = 28.0f;     // 見出しの行の高さ
        float          sideRowPitch        = 24.0f;     // ふつうの行の高さ
        float          sideSectionGap      = 10.0f;     // まとまりの間のすき間
        float          sideIconSize        = 22.0f;     // 行頭の果物（3D）の大きさ
        float          sideIconGap         = 6.0f;      // 果物から名前まで
        float          sideIconSpinSpeed   = 40.0f;     // 行頭の果物の回る速さ（度/秒）
        int            sideRecentRows      = 5;         // 最近とれた果物の行の数
        int            sideTableRows       = 3;         // 台の上の果物の行の数
        float          sideBarHeight       = 10.0f;     // 図鑑の進み具合のバーの高さ
        float          sideButtonInset     = 4.0f;      // おすすめの強化の板の、パネルの内側の余白からのはみ出し
        hlslpp::float4 sideColor           = hlslpp::float4(0.08f, 0.04f, 0.12f, 0.55f);    // 板
        hlslpp::float4 sideBarBackColor    = hlslpp::float4(0.1f, 0.05f, 0.15f, 0.8f);
        hlslpp::float4 sideBarFillColor    = hlslpp::float4(0.95f, 0.6f, 0.75f, 1.0f);
        hlslpp::float4 sideButtonColor     = hlslpp::float4(0.3f, 0.45f, 0.3f, 0.7f);       // おすすめの強化の板（買えない）
        hlslpp::float4 sideButtonReadyColor = hlslpp::float4(0.35f, 0.7f, 0.4f, 0.85f);     // 同じく買える
        hlslpp::float4 sideButtonHoverColor = hlslpp::float4(0.5f, 0.85f, 0.55f, 0.95f);    // 同じくカーソルが重なっている
        UiFont         sideHeader          = {0.72f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f), true};
        UiFont         sideText            = {0.6f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        UiFont         sideValue           = {0.6f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), true};    // 右に揃える数
        hlslpp::float4 sideNewColor        = hlslpp::float4(0.6f, 1.0f, 0.6f, 1.0f);    // 図鑑に初めて載った果物の名前
        hlslpp::float4 sideDimColor        = hlslpp::float4(0.8f, 0.75f, 0.9f, 1.0f);   // 補足（まだ無い・足りない など）

        //--------------------------------------------------------------
        // 果物が取れたときのカットイン（画面の中ほどの帯に、果物の 3D モデルと名前を出す）。
        // 帯は横から伸びて入り、果物はぽんと出て回り、最後に縮んで消える
        //--------------------------------------------------------------
        float          cutInCenterY       = 330.0f;     // 帯の中心の高さ
        float          cutInBandHeight    = 150.0f;     // 帯の高さ
        float          cutInBandTilt      = -4.0f;      // 帯の傾き（度）
        float          cutInEdgeHeight    = 6.0f;       // 帯の上下の縁の線の太さ（色は果物の色）
        hlslpp::float4 cutInBandColor     = hlslpp::float4(0.12f, 0.05f, 0.18f, 0.95f);
        float          cutInFruitX        = 500.0f;     // 果物の中心
        float          cutInFruitSize     = 150.0f;     // 果物のいちばん長い向きの大きさ（ピクセル）
        float          cutInFruitTilt     = 15.0f;      // 果物を手前へ傾ける角度（度。上から少し見下ろす）
        float          cutInSpinSpeed     = 120.0f;     // 果物の回る速さ（度/秒）
        float          cutInPopScale      = 1.2f;       // 出てくるときに一瞬大きくなる倍率
        float          cutInTextX         = 600.0f;     // 文字の左端
        float          cutInTitleOffsetY  = -26.0f;     // 帯の中心から見出しまで
        float          cutInNameOffsetY   = 28.0f;      // 帯の中心から名前まで
        UiFont         cutInTitle         = {1.25f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), true};
        UiFont         cutInName          = {0.95f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), true};
        float          cutInInSeconds     = 0.25f;      // 入ってくる時間
        float          cutInHoldSeconds   = 1.3f;       // 止まって見せる時間
        float          cutInOutSeconds    = 0.25f;      // 消える時間
        int            cutInMaxQueue      = 3;          // 続けて取れたときに順番待ちにする数（超えた分は出さない）

        //--------------------------------------------------------------
        // 画面（図鑑・強化）の共通
        //--------------------------------------------------------------
        hlslpp::float2 menuCenter       = hlslpp::float2(640.0f, 340.0f);
        hlslpp::float2 menuSize         = hlslpp::float2(900.0f, 540.0f);
        hlslpp::float4 menuColor        = hlslpp::float4(0.08f, 0.04f, 0.12f, 0.9f);
        float          menuTitleOffsetY = 32.0f;    // 画面の上端からタイトルまで
        UiFont         menuTitle        = {1.3f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f), true};
        float          scrollBarWidth   = 10.0f;
        float          scrollBarInset   = 22.0f;    // 画面の右端からスクロールバーの中心まで
        hlslpp::float4 scrollTrackColor = hlslpp::float4(0.0f, 0.0f, 0.0f, 0.35f);

        //--------------------------------------------------------------
        // 図鑑（行＝果物、列＝バリエーション）
        //--------------------------------------------------------------
        float          zukanColumnsLeft    = 330.0f;    // 画面の左端から列の始まりまで
        float          zukanColumnsRight   = 40.0f;     // 列の終わりから画面の右端まで
        float          zukanHeaderOffsetY  = 78.0f;     // 画面の上端から列の見出しまで
        UiFont         zukanHeader         = {0.85f, hlslpp::float4(0.9f, 0.8f, 1.0f, 1.0f), true};
        float          zukanRowsGap        = 30.0f;     // 列の見出しから行の始まりまで
        float          zukanRowsBottom     = 56.0f;     // 行の終わりから画面の下端まで
        float          zukanRowPitch       = 44.0f;     // 行の高さ（行が多ければスクロールする）
        float          zukanListLeft       = 16.0f;     // スクロールの枠の、画面の左端からの余白
        float          zukanListRight      = 36.0f;     // 同じく右端からの余白（スクロールバーの分）
        hlslpp::float4 zukanThumbColor     = hlslpp::float4(0.95f, 0.6f, 0.75f, 1.0f);
        float          zukanNameX          = 36.0f;     // 画面の左端から果物の名前まで
        UiFont         zukanName           = {0.85f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        float          zukanFruitSize      = 72.0f;     // 枠の果物（3D）の大きさ（未登録は黒いシルエット）
        float          zukanFruitOffsetY   = -14.0f;    // 行の中心から果物の中心まで
        float          zukanFruitSpinSpeed = 40.0f;     // 枠の果物の回る速さ（度/秒。シルエットも回す）
        float          zukanCountOffsetY   = 40.0f;     // 行の中心から個数まで（列の中心に揃える）
        UiFont         zukanCount          = {0.85f, hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f)};
        float          zukanFooterOffsetY  = 28.0f;     // 画面の下端から集計まで
        UiFont         zukanFooter         = {0.9f, hlslpp::float4(0.85f, 1.0f, 0.85f, 1.0f)};

        //--------------------------------------------------------------
        // 強化（行＝強化）
        //--------------------------------------------------------------
        float          upgradeWalletOffsetY = 74.0f;     // 画面の上端から手持ちまで
        UiFont         upgradeWallet        = {0.9f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), true};
        float          upgradeRowsTop       = 104.0f;    // 画面の上端から行の始まりまで
        float          upgradeRowsBottom    = 24.0f;     // 行の終わりから画面の下端まで
        float          upgradeRowPitch      = 100.0f;    // 行の高さ（行が多ければスクロールする）
        float          upgradeRowSpread     = 0.2f;      // 1行の中の上段・下段のずれ（行の高さに対する割合）
        float          upgradeListLeft      = 16.0f;
        float          upgradeListRight     = 34.0f;
        hlslpp::float4 upgradeThumbColor    = hlslpp::float4(0.55f, 0.85f, 0.5f, 1.0f);
        float          upgradeNameX         = 40.0f;     // 画面の左端から名前・説明まで
        UiFont         upgradeName          = {0.9f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        UiFont         upgradeDescription   = {0.62f, hlslpp::float4(0.8f, 0.75f, 0.9f, 1.0f)};
        float          upgradeEffectX       = 445.0f;    // 画面の左端から効果・価格まで
        UiFont         upgradeEffect        = {0.75f, hlslpp::float4(0.7f, 1.0f, 0.75f, 1.0f)};
        float          upgradeCostScale     = 0.7f;
        float          upgradeButtonRight   = 40.0f;     // 購入ボタンの右端から画面の右端まで
        float          upgradeButtonWidth   = 150.0f;
        float          upgradeButtonHeight  = 46.0f;
        float          upgradeLabelScale    = 0.85f;
        hlslpp::float4 upgradeBuyColor        = hlslpp::float4(0.35f, 0.78f, 0.45f, 1.0f);    // 買える
        hlslpp::float4 upgradeBuyHoverColor   = hlslpp::float4(0.5f, 0.92f, 0.6f, 1.0f);      // 買える（カーソルが重なっている）
        hlslpp::float4 upgradeCannotBuyColor  = hlslpp::float4(0.3f, 0.28f, 0.34f, 1.0f);     // 手持ちが足りない
        hlslpp::float4 upgradeMaxedColor      = hlslpp::float4(0.75f, 0.6f, 0.25f, 1.0f);     // 最大レベル
        hlslpp::float4 upgradeAffordableColor = hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f);      // 価格の文字（足りる）
        hlslpp::float4 upgradeShortColor      = hlslpp::float4(1.0f, 0.55f, 0.55f, 1.0f);     // 価格の文字（足りない）

        //--------------------------------------------------------------
        // 「おかえり」画面
        //--------------------------------------------------------------
        hlslpp::float2 welcomeSize          = hlslpp::float2(720.0f, 300.0f);
        hlslpp::float4 welcomeColor         = hlslpp::float4(0.12f, 0.06f, 0.18f, 0.96f);
        float          welcomeTitleY        = 40.0f;     // 画面の上端から各行まで
        float          welcomeAwayY         = 92.0f;
        float          welcomeRewardY       = 132.0f;
        float          welcomeCappedY       = 170.0f;
        UiFont         welcomeTitle         = {1.3f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f), true};
        UiFont         welcomeAway          = {0.85f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        UiFont         welcomeReward        = {0.85f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f)};
        UiFont         welcomeNoFairy       = {0.75f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f)};
        UiFont         welcomeCapped        = {0.65f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f)};
        float          welcomeButtonOffsetY = 48.0f;     // 画面の下端から閉じるボタンの中心まで
        hlslpp::float2 welcomeButtonSize    = hlslpp::float2(200.0f, 50.0f);
        hlslpp::float4 welcomeButtonColor   = hlslpp::float4(0.35f, 0.78f, 0.45f, 1.0f);
        float          welcomeLabelScale    = 0.9f;

        //--------------------------------------------------------------
        // オプション画面（x は画面の左端から。中身はスクロールする領域の中に並べ、y はその上端から）
        //--------------------------------------------------------------
        float          optionsListTop       = 70.0f;      // スクロールする領域の上端（画面の上端から）
        float          optionsListBottom    = 24.0f;      // スクロールする領域の下端（画面の下端から）
        float          optionsListLeft      = 20.0f;      // スクロールする領域の左端（画面の左端から）
        float          optionsListRight     = 44.0f;      // スクロールする領域の右端（画面の右端から。スクロールバーの分あける）
        hlslpp::float4 optionsThumbColor    = hlslpp::float4(0.62f, 0.52f, 0.85f, 1.0f);
        float          optionsLabelX        = 50.0f;      // 項目名の左端
        float          optionsRowsTop       = 40.0f;      // 1行目の中心
        float          optionsRowPitch      = 64.0f;      // 行の間隔
        float          optionsMinusX        = 330.0f;     // 「−」ボタンの中心
        float          optionsValueX        = 405.0f;     // 音量の％の中心（オン/オフのボタンの中心も）
        float          optionsPlusX         = 480.0f;     // 「＋」ボタンの中心
        hlslpp::float2 optionsStepSize      = hlslpp::float2(52.0f, 42.0f);     // −/＋ ボタンの大きさ
        hlslpp::float2 optionsToggleSize    = hlslpp::float2(200.0f, 42.0f);    // オン/オフのボタンの大きさ
        UiFont         optionsLabel         = {0.9f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        UiFont         optionsValue         = {0.95f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), true};
        float          optionsButtonLabelScale = 0.85f;
        hlslpp::float4 optionsButtonFill    = hlslpp::float4(0.45f, 0.38f, 0.65f, 1.0f);    // ボタン
        hlslpp::float4 optionsButtonHover   = hlslpp::float4(0.62f, 0.52f, 0.85f, 1.0f);    // ボタン（カーソルが重なっている）
        hlslpp::float4 optionsToggleOn      = hlslpp::float4(0.35f, 0.75f, 0.45f, 1.0f);    // オン/表示
        hlslpp::float4 optionsToggleOff     = hlslpp::float4(0.4f, 0.36f, 0.45f, 1.0f);     // オフ/非表示
        hlslpp::float2 optionsQuitSize      = hlslpp::float2(240.0f, 46.0f);    // 「ゲームを終了」（設定の行の次の行。左端は項目名に揃える）
        hlslpp::float4 optionsQuitColor     = hlslpp::float4(0.4f, 0.4f, 0.5f, 1.0f);
        float          optionsDataBelowFold = 60.0f;      // 「データ」の見出しを、最初に見える範囲の下端からどれだけ下に置くか（スクロールしないと見えない）
        float          optionsDataNoteGap   = 42.0f;      // 見出しから説明まで
        float          optionsDataButtonGap = 100.0f;     // 見出しから「データを消して最初から」の中心まで
        float          optionsBottomMargin  = 40.0f;      // 最後のボタンの下の余白
        UiFont         optionsDataTitle     = {0.95f, hlslpp::float4(1.0f, 0.6f, 0.6f, 1.0f), true};
        UiFont         optionsDataNote      = {0.72f, hlslpp::float4(0.9f, 0.9f, 0.95f, 1.0f)};
        hlslpp::float2 optionsResetSize     = hlslpp::float2(340.0f, 46.0f);
        hlslpp::float4 optionsResetColor    = hlslpp::float4(0.75f, 0.35f, 0.4f, 1.0f);

        //--------------------------------------------------------------
        // データ消去の確認ウィンドウ（画面の中央に出す。y はウィンドウの上端から、ボタンの x は中央から）
        //--------------------------------------------------------------
        hlslpp::float2 confirmSize          = hlslpp::float2(640.0f, 300.0f);
        hlslpp::float4 confirmColor         = hlslpp::float4(0.2f, 0.06f, 0.1f, 0.98f);
        hlslpp::float4 confirmDimColor      = hlslpp::float4(0.0f, 0.0f, 0.0f, 0.55f);    // 後ろの画面を暗くする板
        float          confirmStepY         = 40.0f;      // 「確認 n/3」
        float          confirmMessageY      = 105.0f;     // 1行目
        float          confirmNoteY         = 150.0f;     // 2行目
        float          confirmButtonOffsetY = 60.0f;      // ボタンの中心（下端から）
        float          confirmYesOffsetX    = -150.0f;    // 消すボタンの中心
        float          confirmNoOffsetX     = 150.0f;     // やめるボタンの中心
        hlslpp::float2 confirmButtonSize    = hlslpp::float2(240.0f, 50.0f);
        UiFont         confirmStep          = {0.85f, hlslpp::float4(1.0f, 0.75f, 0.75f, 1.0f), true};
        UiFont         confirmMessage       = {1.0f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f), true};
        UiFont         confirmNote          = {0.75f, hlslpp::float4(0.95f, 0.85f, 0.85f, 1.0f)};
        hlslpp::float4 confirmYesColor      = hlslpp::float4(0.85f, 0.25f, 0.25f, 1.0f);
        hlslpp::float4 confirmYesWaitColor  = hlslpp::float4(0.4f, 0.22f, 0.24f, 1.0f);    // まだ押せない間
        hlslpp::float4 confirmNoColor       = hlslpp::float4(0.4f, 0.4f, 0.5f, 1.0f);
        float          confirmDelay         = 0.5f;       // 出てから消すボタンを押せるようになるまで（秒。連打で通らないように）

        //--------------------------------------------------------------
        // 表示時間（秒）
        //--------------------------------------------------------------
        float payoutPopupSeconds     = 1.2f;    // 払い出し表示。続けて落ちたら延長して合算する
        float harvestPopupSeconds    = 2.0f;    // 収穫表示
        float registeredPopupSeconds = 3.5f;    // 「図鑑に登録！」。通常の収穫表示より長く
        float magicLearnedSeconds    = 4.0f;    // 「新しい魔法を覚えた！」
        float muteNoticeSeconds      = 1.5f;    // 消音の切り替え
        float growNoticeSeconds      = 2.5f;    // おおきくなーれの結果
        float musicRestartDelay      = 0.4f;    // BGM の音量を変えてから流し直すまで（−/＋を続けて押しても1回だけ流し直す）

        //--------------------------------------------------------------
        // ロード画面
        //--------------------------------------------------------------
        float          loadingMinSeconds   = 0.3f;     // 出しておく最短の時間（キャッシュが効いて一瞬で読み終えたときのちらつき防止）
        float          loadingBarWidth     = 560.0f;
        float          loadingBarHeight    = 18.0f;
        float          loadingBarCenterY   = 400.0f;
        float          loadingFramePadding = 6.0f;
        float          loadingTextY        = 350.0f;
        hlslpp::float4 loadingBackColor    = hlslpp::float4(0.12f, 0.06f, 0.18f, 1.0f);    // 屋台の夕暮れに合わせた濃い紫
        hlslpp::float4 loadingFrameColor   = hlslpp::float4(0.05f, 0.02f, 0.08f, 1.0f);
        hlslpp::float4 loadingFillColor    = hlslpp::float4(1.0f, 0.75f, 0.35f, 1.0f);
        UiFont         loadingText         = {1.1f, hlslpp::float4(1.0f, 0.92f, 0.85f, 1.0f), true};

        //--------------------------------------------------------------
        // ルーレット（3リールのスロット）。ふだんは上の真ん中に小さく、ジャックポットチャンスは真ん中に大きく出す
        //--------------------------------------------------------------
        SlotLayout     slot;                                                                // ふだんのスロット
        SlotLayout     jackpotSlot     = {hlslpp::float2(640.0f, 330.0f), hlslpp::float2(560.0f, 230.0f), hlslpp::float4(0.35f, 0.22f, 0.04f, 0.95f),
                                          -12.0f, hlslpp::float2(150.0f, 170.0f), 170.0f, hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f), 8.0f,
                                          hlslpp::float4(1.0f, 0.8f, 0.15f, 1.0f), 100.0f, 120.0f, 1500.0f, 420.0f, 0.3f, 1.6f, 12};    // ジャックポットの大きなスロット（金色）
        float          slotLampOffsetY = 43.0f;                                            // ふだんのスロットの板の中心から「のこり」の玉まで
        float          slotLampSize    = 9.0f;                                             // 玉の大きさ
        float          slotLampGap     = 16.0f;                                            // 玉の間隔
        hlslpp::float4 slotLampOn      = hlslpp::float4(1.0f, 0.85f, 0.3f, 1.0f);          // ためている回転の玉
        hlslpp::float4 slotLampOff     = hlslpp::float4(0.3f, 0.25f, 0.35f, 1.0f);         // 空の玉
        float          slotFlashSeconds = 0.35f;                                           // 列が止まったときに縁が光っている時間
        float          slotFlySize      = 46.0f;                                           // 当たりの果物が飛び始めるときの大きさ（着くときは台の上の大きさ）
        float          slotFlyArc       = 60.0f;                                           // 飛ぶときに上へふくらむ高さ

        //! 既定の HUD の文字の置き方を入れます（コンストラクタ。JSON に無い種類はこのまま）。
        UiConfig();

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル（Ui.json）
        //! @return 読み込めたら true（読めなくても既定値で動く）
        bool Load(const std::string& path);

        //! HUD の文字の置き方を返します。
        //! @param  [in] kind 種類（"coins" など）
        //! @return 置き方（無ければ左上に白）
        const UiText& HudText(const std::string& kind) const;
    };

    //! レジストリに置いた UI の設定を返します。
    //! @param  [in] registry レジストリ
    //! @return UI の設定（コンテキストに無ければ既定値）
    const UiConfig& GetUiConfig(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic

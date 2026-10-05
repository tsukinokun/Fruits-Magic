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
    };

    //! 文字の大きさと色です（位置は配置の計算で決まるもの）。
    struct UiFont {
        float          scale = 1.0f;                                     // 大きさ
        hlslpp::float4 color = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 色
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

        //--------------------------------------------------------------
        // 文字の縁
        //--------------------------------------------------------------
        hlslpp::float4 hudOutlineColor  = hlslpp::float4(0.15f, 0.08f, 0.05f, 1.0f);    // HUD の文字の縁の色
        hlslpp::float4 textOutlineColor = hlslpp::float4(0.1f, 0.03f, 0.15f, 1.0f);     // 画面（図鑑・強化など）の文字の縁の色
        float          outlineWidth     = 2.0f;                                         // 文字の縁の太さ

        //--------------------------------------------------------------
        // HUD の文字（"coins" "mana" などの種類ごと）
        //--------------------------------------------------------------
        std::unordered_map<std::string, UiText> hudTexts;
        hlslpp::float4 rouletteJackpotColor = hlslpp::float4(1.0f, 0.65f, 0.1f, 1.0f);    // ジャックポットチャンスの間のルーレットの文字の色

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

        //--------------------------------------------------------------
        // 画面（図鑑・強化）の共通
        //--------------------------------------------------------------
        hlslpp::float2 menuCenter       = hlslpp::float2(640.0f, 340.0f);
        hlslpp::float2 menuSize         = hlslpp::float2(900.0f, 540.0f);
        hlslpp::float4 menuColor        = hlslpp::float4(0.08f, 0.04f, 0.12f, 0.9f);
        float          menuTitleOffsetY = 32.0f;    // 画面の上端からタイトルまで
        UiFont         menuTitle        = {1.3f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f)};
        float          scrollBarWidth   = 10.0f;
        float          scrollBarInset   = 22.0f;    // 画面の右端からスクロールバーの中心まで
        hlslpp::float4 scrollTrackColor = hlslpp::float4(0.0f, 0.0f, 0.0f, 0.35f);

        //--------------------------------------------------------------
        // 図鑑（行＝果物、列＝バリエーション）
        //--------------------------------------------------------------
        float          zukanColumnsLeft    = 330.0f;    // 画面の左端から列の始まりまで
        float          zukanColumnsRight   = 40.0f;     // 列の終わりから画面の右端まで
        float          zukanHeaderOffsetY  = 78.0f;     // 画面の上端から列の見出しまで
        UiFont         zukanHeader         = {0.85f, hlslpp::float4(0.9f, 0.8f, 1.0f, 1.0f)};
        float          zukanRowsGap        = 30.0f;     // 列の見出しから行の始まりまで
        float          zukanRowsBottom     = 56.0f;     // 行の終わりから画面の下端まで
        float          zukanRowPitch       = 44.0f;     // 行の高さ（行が多ければスクロールする）
        float          zukanListLeft       = 16.0f;     // スクロールの枠の、画面の左端からの余白
        float          zukanListRight      = 36.0f;     // 同じく右端からの余白（スクロールバーの分）
        hlslpp::float4 zukanThumbColor     = hlslpp::float4(0.95f, 0.6f, 0.75f, 1.0f);
        float          zukanNameX          = 36.0f;     // 画面の左端から果物の名前まで
        UiFont         zukanName           = {0.85f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        float          zukanSwatchOffsetX  = -30.0f;    // 列の中心から色見本まで
        float          zukanSwatchSize     = 26.0f;
        hlslpp::float4 zukanSwatchColor    = hlslpp::float4(0.2f, 0.2f, 0.25f, 1.0f);    // 作ったときの色（ZukanSystem が書き換える）
        hlslpp::float4 zukanUnknownColor   = hlslpp::float4(0.22f, 0.2f, 0.26f, 1.0f);   // 未登録の色見本の色
        float          zukanCountOffsetX   = -10.0f;    // 列の中心から個数まで
        UiFont         zukanCount          = {0.85f, hlslpp::float4(1.0f, 0.95f, 0.8f, 1.0f)};
        float          zukanFooterOffsetY  = 28.0f;     // 画面の下端から集計まで
        UiFont         zukanFooter         = {0.9f, hlslpp::float4(0.85f, 1.0f, 0.85f, 1.0f)};

        //--------------------------------------------------------------
        // 強化（行＝強化）
        //--------------------------------------------------------------
        float          upgradeWalletOffsetY = 74.0f;     // 画面の上端から手持ちまで
        UiFont         upgradeWallet        = {0.9f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f)};
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
        UiFont         welcomeTitle         = {1.3f, hlslpp::float4(1.0f, 0.85f, 0.95f, 1.0f)};
        UiFont         welcomeAway          = {0.85f, hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f)};
        UiFont         welcomeReward        = {0.85f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f)};
        UiFont         welcomeNoFairy       = {0.75f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f)};
        UiFont         welcomeCapped        = {0.65f, hlslpp::float4(0.85f, 0.8f, 0.95f, 1.0f)};
        float          welcomeButtonOffsetY = 48.0f;     // 画面の下端から閉じるボタンの中心まで
        hlslpp::float2 welcomeButtonSize    = hlslpp::float2(200.0f, 50.0f);
        hlslpp::float4 welcomeButtonColor   = hlslpp::float4(0.35f, 0.78f, 0.45f, 1.0f);
        float          welcomeLabelScale    = 0.9f;

        //--------------------------------------------------------------
        // 表示時間（秒）
        //--------------------------------------------------------------
        float payoutPopupSeconds     = 1.2f;    // 払い出し表示。続けて落ちたら延長して合算する
        float harvestPopupSeconds    = 2.0f;    // 収穫表示
        float registeredPopupSeconds = 3.5f;    // 「図鑑に登録！」。通常の収穫表示より長く
        float magicLearnedSeconds    = 4.0f;    // 「新しい魔法を覚えた！」
        float muteNoticeSeconds      = 1.5f;    // 消音の切り替え
        float growNoticeSeconds      = 2.5f;    // おおきくなーれの結果

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
        UiFont         loadingText         = {1.1f, hlslpp::float4(1.0f, 0.92f, 0.85f, 1.0f)};

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

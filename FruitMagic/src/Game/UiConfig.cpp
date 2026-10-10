//----------------------------------------------------------------------------
//! @file   UiConfig.cpp
//! @brief  画面の UI の配置・色・表示時間の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/UiConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //--------------------------------------------------------------
        //! 項目があれば文字の大きさと色 {scale, color} を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（無い要素はそのまま）
        //--------------------------------------------------------------
        void ReadFont(const Json::Value& obj, const char* key, UiFont& out) {
            if(const Json::Value* font = Json::FindObject(obj, key)) {
                Json::Read(*font, "scale", out.scale);
                Json::ReadColor(*font, "color", out.color);
                Json::Read(*font, "bold", out.bold);
            }
        }

        //--------------------------------------------------------------
        //! 項目があればスロットの置き方を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（無い要素はそのまま）
        //--------------------------------------------------------------
        void ReadSlot(const Json::Value& obj, const char* key, SlotLayout& out) {
            const Json::Value* slot = Json::FindObject(obj, key);
            if(!slot)
                return;
            Json::ReadVec(*slot, "center", out.center);
            Json::ReadVec(*slot, "panelSize", out.panelSize);
            Json::ReadColor(*slot, "panelColor", out.panelColor);
            Json::Read(*slot, "windowOffsetY", out.windowOffsetY);
            Json::ReadVec(*slot, "windowSize", out.windowSize);
            Json::Read(*slot, "windowSpacing", out.windowSpacing);
            Json::ReadColor(*slot, "windowColor", out.windowColor);
            Json::Read(*slot, "flashBorder", out.flashBorder);
            Json::ReadColor(*slot, "flashColor", out.flashColor);
            Json::Read(*slot, "symbolSize", out.symbolSize);
            Json::Read(*slot, "symbolPitch", out.symbolPitch);
            Json::Read(*slot, "spinSpeed", out.spinSpeed);
            Json::Read(*slot, "reachSpeed", out.reachSpeed);
            Json::Read(*slot, "stopSeconds", out.stopSeconds);
            Json::Read(*slot, "overshoot", out.overshoot);
            out.symbolPitch = std::max(1.0f, out.symbolPitch);
            out.stopSeconds = std::max(0.02f, out.stopSeconds);
        }

        //--------------------------------------------------------------
        //! 文字の置き方 {position, scale, color, align} を読み込みます。
        //! @param  [in]     value 読み込み元のオブジェクト
        //! @param  [in,out] out   読み込み先（無い要素はそのまま）
        //--------------------------------------------------------------
        void ReadText(const Json::Value& value, UiText& out) {
            Json::ReadVec(value, "position", out.position);
            Json::Read(value, "scale", out.scale);
            Json::ReadColor(value, "color", out.color);
            Json::Read(value, "bold", out.bold);
            Json::Read(value, "maxWidth", out.maxWidth);
            std::string align;
            if(Json::Read(value, "align", align))
                out.align = (align == "center") ? UiAlign::Center : (align == "right") ? UiAlign::Right : UiAlign::Left;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 既定の HUD の文字の置き方を入れます。
    //----------------------------------------------------------------------------
    UiConfig::UiConfig() {
        hudTexts["coins"]        = {hlslpp::float2(84.0f, 20.0f), 1.6f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), UiAlign::Left, true};
        hudTexts["mana"]         = {hlslpp::float2(260.0f, 148.0f), 0.9f, hlslpp::float4(0.85f, 0.7f, 1.0f, 1.0f), UiAlign::Left};
        hudTexts["dropPopup"]    = {hlslpp::float2(260.0f, 32.0f), 1.2f, hlslpp::float4(0.6f, 1.0f, 0.6f, 1.0f), UiAlign::Left, true};
        hudTexts["fruitPoints"]  = {hlslpp::float2(72.0f, 86.0f), 1.2f, hlslpp::float4(1.0f, 0.6f, 0.7f, 1.0f), UiAlign::Left, true};
        hudTexts["harvestPopup"] = {hlslpp::float2(72.0f, 204.0f), 1.2f, hlslpp::float4(1.0f, 0.85f, 0.9f, 1.0f), UiAlign::Left, true};
        hudTexts["relief"]       = {hlslpp::float2(72.0f, 248.0f), 0.8f, hlslpp::float4(0.75f, 1.0f, 0.85f, 1.0f), UiAlign::Left};
        hudTexts["roulette"]     = {hlslpp::float2(640.0f, 126.0f), 0.85f, hlslpp::float4(1.0f, 0.95f, 0.6f, 1.0f), UiAlign::Center, true, 640.0f};
        hudTexts["controlsHint"] = {hlslpp::float2(24.0f, 690.0f), 0.65f, hlslpp::float4(1.0f, 1.0f, 1.0f, 0.85f), UiAlign::Left};
        hudTexts["notice"]       = {hlslpp::float2(640.0f, 250.0f), 1.15f, hlslpp::float4(1.0f, 0.75f, 0.95f, 1.0f), UiAlign::Center, true};
    }

    //----------------------------------------------------------------------------
    //! レジストリに置いた UI の設定を返します。
    //----------------------------------------------------------------------------
    const UiConfig& GetUiConfig(Tsukino::ECS::Registry& registry) {
        static const UiConfig kDefault;
        return registry.HasContext<UiConfig>() ? registry.GetContext<UiConfig>() : kDefault;
    }

    //----------------------------------------------------------------------------
    //! HUD の文字の置き方を返します。
    //----------------------------------------------------------------------------
    const UiText& UiConfig::HudText(const std::string& kind) const {
        static const UiText kFallback;
        auto it = hudTexts.find(kind);
        return (it != hudTexts.end()) ? it->second : kFallback;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool UiConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "UiConfig"))
            return false;

        if(const Json::Value* screen = Json::FindObject(doc, "screen")) {
            Json::Read(*screen, "width", screenWidth);
            Json::Read(*screen, "height", screenHeight);
        }
        if(const Json::Value* fonts = Json::FindObject(doc, "fonts")) {
            Json::Read(*fonts, "regular", fontRegularPath);
            Json::Read(*fonts, "bold", fontBoldPath);
        }
        if(const Json::Value* outline = Json::FindObject(doc, "outline")) {
            Json::ReadColor(*outline, "hud", hudOutlineColor);
            Json::ReadColor(*outline, "text", textOutlineColor);
            Json::Read(*outline, "width", outlineWidth);
            Json::Read(*outline, "textPadding", textPadding);
        }
        if(const Json::Value* hud = Json::FindObject(doc, "hud")) {
            for(auto it = hud->MemberBegin(); it != hud->MemberEnd(); ++it) {
                if(it->value.IsObject())
                    ReadText(it->value, hudTexts[it->name.GetString()]);
            }
        }
        Json::ReadColor(doc, "rouletteJackpotColor", rouletteJackpotColor);
        if(const Json::Value* icon = Json::FindObject(doc, "harvestIcon")) {
            Json::ReadVec(*icon, "position", harvestIconPosition);
            Json::Read(*icon, "size", harvestIconSize);
            Json::Read(*icon, "spinSpeed", harvestIconSpinSpeed);
        }

        if(const Json::Value* mana = Json::FindObject(doc, "manaGauge")) {
            Json::Read(*mana, "left", manaGaugeLeft);
            Json::Read(*mana, "centerY", manaGaugeCenterY);
            Json::Read(*mana, "width", manaGaugeWidth);
            Json::Read(*mana, "height", manaGaugeHeight);
            Json::Read(*mana, "padding", manaGaugePadding);
            Json::ReadColor(*mana, "backColor", manaGaugeBackColor);
            Json::ReadColor(*mana, "fillColor", manaGaugeFillColor);
        }
        if(const Json::Value* ring = Json::FindObject(doc, "reliefRing")) {
            Json::Read(*ring, "diameter", reliefRingDiameter);
            Json::ReadVec(*ring, "center", reliefRingCenter);
            Json::ReadColor(*ring, "backColor", reliefRingBackColor);
            Json::ReadColor(*ring, "fillColor", reliefRingFillColor);
        }
        if(const Json::Value* icons = Json::FindObject(doc, "moneyIcons")) {
            if(const Json::Value* coin = Json::FindObject(*icons, "coin")) {
                Json::ReadVec(*coin, "center", coinIconCenter);
                Json::Read(*coin, "size", coinIconSize);
            }
            if(const Json::Value* fp = Json::FindObject(*icons, "fp")) {
                Json::ReadVec(*fp, "center", fpIconCenter);
                Json::Read(*fp, "size", fpIconSize);
            }
        }
        if(const Json::Value* magic = Json::FindObject(doc, "magicButtons")) {
            Json::Read(*magic, "width", magicButtonWidth);
            Json::Read(*magic, "height", magicButtonHeight);
            Json::Read(*magic, "gap", magicButtonGap);
            Json::Read(*magic, "y", magicButtonY);
            Json::Read(*magic, "labelScale", magicLabelScale);
            Json::ReadColor(*magic, "labelOutline", magicLabelOutline);
            Json::ReadColor(*magic, "lockedColor", magicLockedColor);
            Json::ReadColor(*magic, "activeColor", magicActiveColor);
            Json::ReadColor(*magic, "noManaColor", magicNoManaColor);
            Json::ReadColor(*magic, "hoverColor", magicHoverColor);
            Json::ReadColor(*magic, "readyColor", magicReadyColor);
        }
        if(const Json::Value* buttons = Json::FindObject(doc, "menuButtons")) {
            Json::Read(*buttons, "x", menuButtonX);
            Json::Read(*buttons, "zukanY", zukanButtonY);
            Json::Read(*buttons, "recordY", recordButtonY);
            Json::ReadColor(*buttons, "recordColor", recordButtonColor);
            Json::Read(*buttons, "upgradeY", upgradeButtonY);
            Json::Read(*buttons, "width", menuButtonWidth);
            Json::Read(*buttons, "height", menuButtonHeight);
            Json::Read(*buttons, "labelScale", menuLabelScale);
            Json::ReadColor(*buttons, "zukanColor", zukanButtonColor);
            Json::ReadColor(*buttons, "upgradeColor", upgradeButtonColor);
            Json::Read(*buttons, "optionsY", optionsButtonY);
            Json::ReadColor(*buttons, "optionsColor", optionsButtonColor);
        }
        if(const Json::Value* side = Json::FindObject(doc, "sidePanels")) {
            Json::ReadVec(*side, "leftPosition", sideLeftPosition);
            Json::ReadVec(*side, "rightPosition", sideRightPosition);
            Json::Read(*side, "width", sideWidth);
            Json::Read(*side, "padding", sidePadding);
            Json::Read(*side, "headerPitch", sideHeaderPitch);
            Json::Read(*side, "rowPitch", sideRowPitch);
            Json::Read(*side, "sectionGap", sideSectionGap);
            Json::Read(*side, "iconSize", sideIconSize);
            Json::Read(*side, "iconGap", sideIconGap);
            Json::Read(*side, "iconSpinSpeed", sideIconSpinSpeed);
            Json::Read(*side, "recentRows", sideRecentRows);
            Json::Read(*side, "tableRows", sideTableRows);
            Json::Read(*side, "barHeight", sideBarHeight);
            Json::Read(*side, "buttonInset", sideButtonInset);
            Json::ReadColor(*side, "color", sideColor);
            Json::ReadColor(*side, "barBackColor", sideBarBackColor);
            Json::ReadColor(*side, "barFillColor", sideBarFillColor);
            Json::ReadColor(*side, "buttonColor", sideButtonColor);
            Json::ReadColor(*side, "buttonReadyColor", sideButtonReadyColor);
            Json::ReadColor(*side, "buttonHoverColor", sideButtonHoverColor);
            ReadFont(*side, "header", sideHeader);
            ReadFont(*side, "text", sideText);
            ReadFont(*side, "value", sideValue);
            Json::ReadColor(*side, "newColor", sideNewColor);
            Json::ReadColor(*side, "dimColor", sideDimColor);
            sideRecentRows = std::clamp(sideRecentRows, 0, 20);
            sideTableRows  = std::clamp(sideTableRows, 0, 20);
        }
        if(const Json::Value* cutIn = Json::FindObject(doc, "cutIn")) {
            Json::Read(*cutIn, "centerY", cutInCenterY);
            Json::Read(*cutIn, "bandHeight", cutInBandHeight);
            Json::Read(*cutIn, "bandTilt", cutInBandTilt);
            Json::Read(*cutIn, "edgeHeight", cutInEdgeHeight);
            Json::ReadColor(*cutIn, "bandColor", cutInBandColor);
            Json::Read(*cutIn, "fruitX", cutInFruitX);
            Json::Read(*cutIn, "fruitSize", cutInFruitSize);
            Json::Read(*cutIn, "fruitTilt", cutInFruitTilt);
            Json::Read(*cutIn, "spinSpeed", cutInSpinSpeed);
            Json::Read(*cutIn, "popScale", cutInPopScale);
            Json::Read(*cutIn, "textX", cutInTextX);
            Json::Read(*cutIn, "titleOffsetY", cutInTitleOffsetY);
            Json::Read(*cutIn, "nameOffsetY", cutInNameOffsetY);
            ReadFont(*cutIn, "title", cutInTitle);
            ReadFont(*cutIn, "name", cutInName);
            Json::Read(*cutIn, "inSeconds", cutInInSeconds);
            Json::Read(*cutIn, "holdSeconds", cutInHoldSeconds);
            Json::Read(*cutIn, "outSeconds", cutInOutSeconds);
            Json::Read(*cutIn, "maxQueue", cutInMaxQueue);
            cutInInSeconds   = std::max(0.01f, cutInInSeconds);
            cutInHoldSeconds = std::max(0.0f, cutInHoldSeconds);
            cutInOutSeconds  = std::max(0.01f, cutInOutSeconds);
            cutInMaxQueue    = std::max(0, cutInMaxQueue);
        }
        if(const Json::Value* menu = Json::FindObject(doc, "menu")) {
            Json::ReadVec(*menu, "center", menuCenter);
            Json::ReadVec(*menu, "size", menuSize);
            Json::ReadColor(*menu, "color", menuColor);
            Json::Read(*menu, "titleOffsetY", menuTitleOffsetY);
            ReadFont(*menu, "title", menuTitle);
            Json::Read(*menu, "scrollBarWidth", scrollBarWidth);
            Json::Read(*menu, "scrollBarInset", scrollBarInset);
            Json::ReadColor(*menu, "scrollTrackColor", scrollTrackColor);
        }
        if(const Json::Value* zukan = Json::FindObject(doc, "zukan")) {
            Json::Read(*zukan, "columnsLeft", zukanColumnsLeft);
            Json::Read(*zukan, "columnsRight", zukanColumnsRight);
            Json::Read(*zukan, "headerOffsetY", zukanHeaderOffsetY);
            ReadFont(*zukan, "header", zukanHeader);
            Json::Read(*zukan, "rowsGap", zukanRowsGap);
            Json::Read(*zukan, "rowsBottom", zukanRowsBottom);
            Json::Read(*zukan, "rowPitch", zukanRowPitch);
            Json::Read(*zukan, "listLeft", zukanListLeft);
            Json::Read(*zukan, "listRight", zukanListRight);
            Json::ReadColor(*zukan, "thumbColor", zukanThumbColor);
            Json::Read(*zukan, "nameX", zukanNameX);
            ReadFont(*zukan, "name", zukanName);
            Json::Read(*zukan, "fruitSize", zukanFruitSize);
            Json::Read(*zukan, "fruitOffsetY", zukanFruitOffsetY);
            Json::Read(*zukan, "fruitSpinSpeed", zukanFruitSpinSpeed);
            Json::Read(*zukan, "countOffsetY", zukanCountOffsetY);
            ReadFont(*zukan, "count", zukanCount);
            Json::Read(*zukan, "footerOffsetY", zukanFooterOffsetY);
            ReadFont(*zukan, "footer", zukanFooter);
            zukanRowPitch = std::max(1.0f, zukanRowPitch);
        }
        if(const Json::Value* upgrade = Json::FindObject(doc, "upgrade")) {
            Json::Read(*upgrade, "walletOffsetY", upgradeWalletOffsetY);
            ReadFont(*upgrade, "wallet", upgradeWallet);
            Json::Read(*upgrade, "rowsTop", upgradeRowsTop);
            Json::Read(*upgrade, "rowsBottom", upgradeRowsBottom);
            Json::Read(*upgrade, "rowPitch", upgradeRowPitch);
            Json::Read(*upgrade, "tabY", upgradeTabY);
            Json::ReadVec(*upgrade, "tabSize", upgradeTabSize);
            Json::Read(*upgrade, "tabGap", upgradeTabGap);
            Json::Read(*upgrade, "tabLabelScale", upgradeTabLabelScale);
            Json::ReadColor(*upgrade, "tabColor", upgradeTabColor);
            Json::ReadColor(*upgrade, "tabHoverColor", upgradeTabHoverColor);
            Json::ReadColor(*upgrade, "tabSelectedColor", upgradeTabSelectedColor);
            Json::Read(*upgrade, "rowSpread", upgradeRowSpread);
            Json::Read(*upgrade, "listLeft", upgradeListLeft);
            Json::Read(*upgrade, "listRight", upgradeListRight);
            Json::ReadColor(*upgrade, "thumbColor", upgradeThumbColor);
            Json::Read(*upgrade, "nameX", upgradeNameX);
            ReadFont(*upgrade, "name", upgradeName);
            ReadFont(*upgrade, "description", upgradeDescription);
            Json::Read(*upgrade, "effectX", upgradeEffectX);
            ReadFont(*upgrade, "effect", upgradeEffect);
            Json::Read(*upgrade, "costScale", upgradeCostScale);
            Json::Read(*upgrade, "buttonRight", upgradeButtonRight);
            Json::Read(*upgrade, "buttonWidth", upgradeButtonWidth);
            Json::Read(*upgrade, "buttonHeight", upgradeButtonHeight);
            Json::Read(*upgrade, "labelScale", upgradeLabelScale);
            Json::ReadColor(*upgrade, "buyColor", upgradeBuyColor);
            Json::ReadColor(*upgrade, "buyHoverColor", upgradeBuyHoverColor);
            Json::ReadColor(*upgrade, "cannotBuyColor", upgradeCannotBuyColor);
            Json::ReadColor(*upgrade, "maxedColor", upgradeMaxedColor);
            Json::ReadColor(*upgrade, "affordableColor", upgradeAffordableColor);
            Json::ReadColor(*upgrade, "shortColor", upgradeShortColor);
            upgradeRowPitch = std::max(1.0f, upgradeRowPitch);
        }
        if(const Json::Value* welcome = Json::FindObject(doc, "welcome")) {
            Json::ReadVec(*welcome, "size", welcomeSize);
            Json::ReadColor(*welcome, "color", welcomeColor);
            Json::Read(*welcome, "titleY", welcomeTitleY);
            Json::Read(*welcome, "awayY", welcomeAwayY);
            Json::Read(*welcome, "rewardY", welcomeRewardY);
            Json::Read(*welcome, "cappedY", welcomeCappedY);
            ReadFont(*welcome, "title", welcomeTitle);
            ReadFont(*welcome, "away", welcomeAway);
            ReadFont(*welcome, "reward", welcomeReward);
            ReadFont(*welcome, "noFairy", welcomeNoFairy);
            ReadFont(*welcome, "capped", welcomeCapped);
            Json::Read(*welcome, "buttonOffsetY", welcomeButtonOffsetY);
            Json::ReadVec(*welcome, "buttonSize", welcomeButtonSize);
            Json::ReadColor(*welcome, "buttonColor", welcomeButtonColor);
            Json::Read(*welcome, "labelScale", welcomeLabelScale);
        }
        if(const Json::Value* options = Json::FindObject(doc, "options")) {
            Json::Read(*options, "listTop", optionsListTop);
            Json::Read(*options, "listBottom", optionsListBottom);
            Json::Read(*options, "listLeft", optionsListLeft);
            Json::Read(*options, "listRight", optionsListRight);
            Json::ReadColor(*options, "thumbColor", optionsThumbColor);
            Json::Read(*options, "labelX", optionsLabelX);
            Json::Read(*options, "rowsTop", optionsRowsTop);
            Json::Read(*options, "rowPitch", optionsRowPitch);
            Json::Read(*options, "minusX", optionsMinusX);
            Json::Read(*options, "valueX", optionsValueX);
            Json::Read(*options, "plusX", optionsPlusX);
            Json::ReadVec(*options, "stepSize", optionsStepSize);
            Json::ReadVec(*options, "toggleSize", optionsToggleSize);
            ReadFont(*options, "label", optionsLabel);
            ReadFont(*options, "value", optionsValue);
            Json::Read(*options, "buttonLabelScale", optionsButtonLabelScale);
            Json::ReadColor(*options, "buttonFill", optionsButtonFill);
            Json::ReadColor(*options, "buttonHover", optionsButtonHover);
            Json::ReadColor(*options, "toggleOn", optionsToggleOn);
            Json::ReadColor(*options, "toggleOff", optionsToggleOff);
            Json::ReadVec(*options, "quitSize", optionsQuitSize);
            Json::ReadColor(*options, "quitColor", optionsQuitColor);
            Json::Read(*options, "dataBelowFold", optionsDataBelowFold);
            Json::Read(*options, "dataNoteGap", optionsDataNoteGap);
            Json::Read(*options, "dataButtonGap", optionsDataButtonGap);
            Json::Read(*options, "bottomMargin", optionsBottomMargin);
            ReadFont(*options, "dataTitle", optionsDataTitle);
            ReadFont(*options, "dataNote", optionsDataNote);
            Json::ReadVec(*options, "resetSize", optionsResetSize);
            Json::ReadColor(*options, "resetColor", optionsResetColor);
        }
        if(const Json::Value* record = Json::FindObject(doc, "record")) {
            Json::Read(*record, "listTop", recordListTop);
            Json::Read(*record, "listBottom", recordListBottom);
            Json::Read(*record, "listLeft", recordListLeft);
            Json::Read(*record, "listRight", recordListRight);
            Json::ReadColor(*record, "thumbColor", recordThumbColor);
            Json::Read(*record, "columnPadding", recordColumnPadding);
            Json::Read(*record, "columnGap", recordColumnGap);
            Json::Read(*record, "rowsTop", recordRowsTop);
            Json::Read(*record, "rowPitch", recordRowPitch);
            Json::Read(*record, "sectionGap", recordSectionGap);
            Json::Read(*record, "itemIndent", recordItemIndent);
            Json::Read(*record, "bottomMargin", recordBottomMargin);
            ReadFont(*record, "heading", recordHeading);
            ReadFont(*record, "label", recordLabel);
            ReadFont(*record, "value", recordValue);
        }
        if(const Json::Value* confirm = Json::FindObject(doc, "confirm")) {
            Json::ReadVec(*confirm, "size", confirmSize);
            Json::ReadColor(*confirm, "color", confirmColor);
            Json::ReadColor(*confirm, "dimColor", confirmDimColor);
            Json::Read(*confirm, "stepY", confirmStepY);
            Json::Read(*confirm, "messageY", confirmMessageY);
            Json::Read(*confirm, "noteY", confirmNoteY);
            Json::Read(*confirm, "buttonOffsetY", confirmButtonOffsetY);
            Json::Read(*confirm, "yesOffsetX", confirmYesOffsetX);
            Json::Read(*confirm, "noOffsetX", confirmNoOffsetX);
            Json::ReadVec(*confirm, "buttonSize", confirmButtonSize);
            ReadFont(*confirm, "step", confirmStep);
            ReadFont(*confirm, "message", confirmMessage);
            ReadFont(*confirm, "note", confirmNote);
            Json::ReadColor(*confirm, "yesColor", confirmYesColor);
            Json::ReadColor(*confirm, "yesWaitColor", confirmYesWaitColor);
            Json::ReadColor(*confirm, "noColor", confirmNoColor);
            Json::Read(*confirm, "delay", confirmDelay);
        }
        if(const Json::Value* timing = Json::FindObject(doc, "timing")) {
            Json::Read(*timing, "payoutPopup", payoutPopupSeconds);
            Json::Read(*timing, "harvestPopup", harvestPopupSeconds);
            Json::Read(*timing, "registeredPopup", registeredPopupSeconds);
            Json::Read(*timing, "magicLearned", magicLearnedSeconds);
            Json::Read(*timing, "muteNotice", muteNoticeSeconds);
            Json::Read(*timing, "growNotice", growNoticeSeconds);
            Json::Read(*timing, "musicRestart", musicRestartDelay);
        }
        ReadSlot(doc, "slot", slot);
        ReadSlot(doc, "jackpotSlot", jackpotSlot);
        if(const Json::Value* lamp = Json::FindObject(doc, "slotLamps")) {
            Json::Read(*lamp, "offsetY", slotLampOffsetY);
            Json::Read(*lamp, "size", slotLampSize);
            Json::Read(*lamp, "gap", slotLampGap);
            Json::ReadColor(*lamp, "on", slotLampOn);
            Json::ReadColor(*lamp, "off", slotLampOff);
        }
        if(const Json::Value* slotFx = Json::FindObject(doc, "slotEffects")) {
            Json::Read(*slotFx, "flashSeconds", slotFlashSeconds);
            Json::Read(*slotFx, "flySize", slotFlySize);
            Json::Read(*slotFx, "flyArc", slotFlyArc);
        }
        if(const Json::Value* loading = Json::FindObject(doc, "loading")) {
            Json::Read(*loading, "minSeconds", loadingMinSeconds);
            Json::Read(*loading, "barWidth", loadingBarWidth);
            Json::Read(*loading, "barHeight", loadingBarHeight);
            Json::Read(*loading, "barCenterY", loadingBarCenterY);
            Json::Read(*loading, "framePadding", loadingFramePadding);
            Json::Read(*loading, "textY", loadingTextY);
            Json::ReadColor(*loading, "backColor", loadingBackColor);
            Json::ReadColor(*loading, "frameColor", loadingFrameColor);
            Json::ReadColor(*loading, "fillColor", loadingFillColor);
            ReadFont(*loading, "text", loadingText);
        }
        return true;
    }
}    // namespace FruitMagic

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

        //--------------------------------------------------------------
        //! 項目があれば傾けて置く板 {center, size, angle, skin} を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（無い要素はそのまま）
        //--------------------------------------------------------------
        void ReadBoard(const Json::Value& obj, const char* key, UiBoard& out) {
            if(const Json::Value* board = Json::FindObject(obj, key)) {
                Json::ReadVec(*board, "center", out.center);
                Json::ReadVec(*board, "size", out.size);
                Json::Read(*board, "angle", out.angle);
                Json::Read(*board, "skin", out.skin);
            }
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 画面の板の見た目を返します。
    //----------------------------------------------------------------------------
    const UiMenuStyle& UiConfig::MenuStyleOf(MenuKind menu) const {
        static const UiMenuStyle kFallback;
        const char*              name = "";
        switch(menu) {
            case MenuKind::Zukan: name = "zukan"; break;
            case MenuKind::Record: name = "record"; break;
            case MenuKind::Upgrade: name = "upgrade"; break;
            case MenuKind::Welcome: name = "welcome"; break;
            case MenuKind::Options: name = "options"; break;
            default: break;
        }
        auto it = menuStyles.find(name);
        return (it != menuStyles.end()) ? it->second : kFallback;
    }

    //----------------------------------------------------------------------------
    //! 板の絵を名前で引きます。
    //----------------------------------------------------------------------------
    const UiSkin& UiConfig::Skin(const std::string& name) const {
        static const UiSkin kFallback;
        auto it = skins.find(name);
        return (it != skins.end()) ? it->second : kFallback;
    }

    //----------------------------------------------------------------------------
    //! 既定の HUD の文字の置き方を入れます。
    //----------------------------------------------------------------------------
    UiConfig::UiConfig() {
        hudTexts["coins"]        = {hlslpp::float2(100.0f, 18.0f), 1.5f, hlslpp::float4(1.0f, 0.92f, 0.4f, 1.0f), UiAlign::Left, true};
        hudTexts["mana"]         = {hlslpp::float2(300.0f, 76.0f), 1.15f, hlslpp::float4(1.0f, 0.95f, 1.0f, 1.0f), UiAlign::Center, true, 60.0f};
        hudTexts["dropPopup"]    = {hlslpp::float2(282.0f, 10.0f), 1.0f, hlslpp::float4(0.6f, 1.0f, 0.6f, 1.0f), UiAlign::Left, true};
        hudTexts["fruitPoints"]  = {hlslpp::float2(106.0f, 98.0f), 1.2f, hlslpp::float4(1.0f, 0.6f, 0.7f, 1.0f), UiAlign::Left, true};
        hudTexts["harvestPopup"] = {hlslpp::float2(72.0f, 204.0f), 1.2f, hlslpp::float4(1.0f, 0.85f, 0.9f, 1.0f), UiAlign::Left, true};
        hudTexts["relief"]       = {hlslpp::float2(72.0f, 248.0f), 0.8f, hlslpp::float4(0.75f, 1.0f, 0.85f, 1.0f), UiAlign::Left};
        hudTexts["roulette"]     = {hlslpp::float2(640.0f, 126.0f), 0.85f, hlslpp::float4(1.0f, 0.95f, 0.6f, 1.0f), UiAlign::Center, true, 640.0f};
        hudTexts["controlsHint"] = {hlslpp::float2(24.0f, 700.0f), 0.58f, hlslpp::float4(1.0f, 1.0f, 1.0f, 0.85f), UiAlign::Left};
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

        if(const Json::Value* mana = Json::FindObject(doc, "manaOrb")) {
            Json::ReadVec(*mana, "center", manaOrbCenter);
            Json::Read(*mana, "size", manaOrbSize);
            Json::Read(*mana, "ringSize", manaRingSize);
            Json::ReadColor(*mana, "color", manaOrbColor);
            Json::ReadColor(*mana, "ringBackColor", manaRingBackColor);
            Json::ReadColor(*mana, "ringFillColor", manaRingFillColor);
        }

        //--------------------------------------------------------------
        // 板の絵（名前 → {texture, border, sliceScale, tint}）と、HUD の看板
        //--------------------------------------------------------------
        if(const Json::Value* skinList = Json::FindObject(doc, "skins")) {
            for(auto it = skinList->MemberBegin(); it != skinList->MemberEnd(); ++it) {
                if(!it->value.IsObject())
                    continue;
                UiSkin& skin = skins[it->name.GetString()];
                Json::Read(it->value, "texture", skin.texture);
                float border[4] = {0.0f, 0.0f, 0.0f, 0.0f};    // [左, 上, 右, 下]
                if(Json::ReadFloats(it->value, "border", border, 4))
                    skin.border = hlslpp::float4(border[0], border[1], border[2], border[3]);
                Json::Read(it->value, "sliceScale", skin.sliceScale);
                Json::ReadColor(it->value, "tint", skin.tint);
            }
        }
        if(const Json::Value* boards = Json::FindObject(doc, "hudBoards")) {
            ReadBoard(*boards, "coin", coinBoard);
            ReadBoard(*boards, "fp", fpBoard);
            Json::Read(*boards, "ropeTexture", ropeTexture);
            Json::Read(*boards, "ropeWidth", ropeWidth);
            Json::Read(*boards, "ropeSpread", ropeSpread);
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
            Json::Read(*magic, "centerX", magicCenterX);
            Json::Read(*magic, "baseY", magicBaseY);
            Json::Read(*magic, "spacing", magicSpacing);
            Json::Read(*magic, "curve", magicCurve);
            if(const Json::Value* sizes = Json::FindArray(*magic, "sizes")) {
                magicSizes.clear();
                for(const Json::Value& size : sizes->GetArray()) {
                    if(size.IsNumber())
                        magicSizes.push_back(std::max(8.0f, size.GetFloat()));
                }
            }
            Json::Read(*magic, "iconRatio", magicIconRatio);
            Json::Read(*magic, "ringRatio", magicRingRatio);
            Json::Read(*magic, "labelGap", magicLabelGap);
            Json::Read(*magic, "labelScale", magicLabelScale);
            Json::Read(*magic, "keyScale", magicKeyScale);
            if(const Json::Value* icons = Json::FindObject(*magic, "icons")) {
                for(auto it = icons->MemberBegin(); it != icons->MemberEnd(); ++it) {
                    if(it->value.IsString())
                        magicIcons[it->name.GetString()] = it->value.GetString();
                }
            }
            Json::Read(*magic, "lockedIcon", magicLockedIcon);
            Json::ReadColor(*magic, "ringColor", magicRingColor);
            Json::ReadColor(*magic, "labelOutline", magicLabelOutline);
            Json::ReadColor(*magic, "lockedColor", magicLockedColor);
            Json::ReadColor(*magic, "activeColor", magicActiveColor);
            Json::ReadColor(*magic, "noManaColor", magicNoManaColor);
            Json::ReadColor(*magic, "hoverColor", magicHoverColor);
            Json::ReadColor(*magic, "readyColor", magicReadyColor);
        }
        if(const Json::Value* buttons = Json::FindObject(doc, "menuButtons")) {
            auto readButton = [&](const char* key, UiRoundButton& out) {
                if(const Json::Value* button = Json::FindObject(*buttons, key)) {
                    Json::ReadVec(*button, "center", out.center);
                    Json::Read(*button, "size", out.size);
                    Json::ReadColor(*button, "color", out.color);
                    Json::Read(*button, "icon", out.icon);
                }
            };
            readButton("zukan", zukanButton);
            readButton("record", recordButton);
            readButton("upgrade", upgradeButton);
            readButton("options", optionsButton);
            Json::Read(*buttons, "iconRatio", menuIconRatio);
            Json::Read(*buttons, "labelScale", menuLabelScale);
            Json::Read(*buttons, "labelGap", menuLabelGap);
            Json::Read(*buttons, "alertSize", upgradeAlertSize);
            Json::Read(*buttons, "alertTexture", upgradeAlertTexture);
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
            auto readCard = [&](const char* key, UiSideCard& out) {
                if(const Json::Value* card = Json::FindObject(*side, key)) {
                    Json::Read(*card, "skin", out.skin);
                    Json::Read(*card, "angle", out.angle);
                    Json::Read(*card, "pin", out.pin);
                    Json::Read(*card, "pinOffsetX", out.pinOffsetX);
                }
            };
            readCard("leftCard", sideLeftCard);
            readCard("zukanCard", sideZukanCard);
            readCard("upgradeCard", sideUpgradeCard);
            readCard("fairyCard", sideFairyCard);
            Json::Read(*side, "pinTexture", sidePinTexture);
            Json::Read(*side, "pinSize", sidePinSize);
            Json::Read(*side, "bubbleTailTexture", sideBubbleTailTexture);
            Json::ReadVec(*side, "bubbleTailSize", sideBubbleTailSize);
            Json::Read(*side, "fairyTexture", sideFairyTexture);
            Json::Read(*side, "fairySize", sideFairySize);
            Json::ReadVec(*side, "fairyOffset", sideFairyOffset);
            Json::ReadColor(*side, "outlineColor", sideOutlineColor);
            Json::Read(*side, "outlineWidth", sideOutlineWidth);
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
            Json::ReadVec(*menu, "ribbonSize", menuRibbonSize);
            Json::Read(*menu, "ribbonOffsetY", menuRibbonOffsetY);
            Json::Read(*menu, "closeTexture", menuCloseTexture);
            Json::Read(*menu, "closeSize", menuCloseSize);
            Json::Read(*menu, "closeInset", menuCloseInset);
            Json::ReadColor(*menu, "closeColor", menuCloseColor);
            if(const Json::Value* styles = Json::FindObject(*menu, "styles")) {
                for(auto it = styles->MemberBegin(); it != styles->MemberEnd(); ++it) {
                    if(!it->value.IsObject())
                        continue;
                    UiMenuStyle& style = menuStyles[it->name.GetString()];
                    Json::Read(it->value, "skin", style.skin);
                    Json::Read(it->value, "coverSkin", style.coverSkin);
                    Json::ReadColor(it->value, "coverTint", style.coverTint);
                    Json::Read(it->value, "coverPadding", style.coverPadding);
                    Json::ReadColor(it->value, "ribbonTint", style.ribbonTint);
                }
            }
        }
        if(const Json::Value* zukan = Json::FindObject(doc, "zukan")) {
            Json::Read(*zukan, "columnsLeft", zukanColumnsLeft);
            Json::Read(*zukan, "columnsRight", zukanColumnsRight);
            Json::Read(*zukan, "headerOffsetY", zukanHeaderOffsetY);
            ReadFont(*zukan, "header", zukanHeader);
            Json::ReadVec(*zukan, "headerTabSize", zukanHeaderTabSize);
            Json::Read(*zukan, "headerTabAngle", zukanHeaderTabAngle);
            if(const Json::Value* colors = Json::FindArray(*zukan, "headerTabColors")) {
                zukanHeaderTabColors.clear();
                for(const Json::Value& color : colors->GetArray()) {
                    hlslpp::float4 value(1.0f, 1.0f, 1.0f, 1.0f);
                    Json::ReadColorValue(color, value);
                    zukanHeaderTabColors.push_back(value);
                }
            }
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
            Json::ReadVec(*upgrade, "costTagSize", upgradeCostTagSize);
            Json::Read(*upgrade, "costTagLeft", upgradeCostTagLeft);
            Json::Read(*upgrade, "costTagTextX", upgradeCostTagTextX);
            Json::Read(*upgrade, "costTagAngle", upgradeCostTagAngle);
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
            Json::Read(*upgrade, "seedRowsTop", upgradeSeedRowsTop);
            Json::Read(*upgrade, "moveY", upgradeMoveY);
            Json::ReadVec(*upgrade, "moveSize", upgradeMoveSize);
            Json::Read(*upgrade, "moveLabelScale", upgradeMoveLabelScale);
            Json::Read(*upgrade, "moveInfoX", upgradeMoveInfoX);
            ReadFont(*upgrade, "moveInfo", upgradeMoveInfo);
            Json::ReadColor(*upgrade, "moveColor", upgradeMoveColor);
            Json::ReadColor(*upgrade, "moveHoverColor", upgradeMoveHoverColor);
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
            Json::Read(*timing, "moveNotice", moveNoticeSeconds);
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
        if(const Json::Value* m = Json::FindObject(doc, "motion")) {
            if(const Json::Value* button = Json::FindObject(*m, "button")) {
                Json::Read(*button, "hoverScale", motion.buttonHoverScale);
                Json::Read(*button, "pressScale", motion.buttonPressScale);
                Json::Read(*button, "clickPunch", motion.buttonClickPunch);
                Json::Read(*button, "speed", motion.buttonSpeed);
            }
            if(const Json::Value* pulse = Json::FindObject(*m, "pulse")) {
                Json::Read(*pulse, "scale", motion.pulseScale);
                Json::Read(*pulse, "seconds", motion.pulseSeconds);
            }
            if(const Json::Value* pop = Json::FindObject(*m, "pop")) {
                Json::Read(*pop, "seconds", motion.popSeconds);
                Json::Read(*pop, "fromScale", motion.popFromScale);
            }
            if(const Json::Value* panel = Json::FindObject(*m, "panel")) {
                Json::Read(*panel, "seconds", motion.panelSeconds);
                Json::Read(*panel, "fromScale", motion.panelFromScale);
            }
            if(const Json::Value* slide = Json::FindObject(*m, "slide")) {
                Json::Read(*slide, "distance", motion.slideDistance);
                Json::Read(*slide, "seconds", motion.slideSeconds);
                Json::Read(*slide, "stagger", motion.slideStagger);
            }
            if(const Json::Value* count = Json::FindObject(*m, "countUp")) {
                Json::Read(*count, "speed", motion.countUpSpeed);
                Json::Read(*count, "punchScale", motion.punchScale);
                Json::Read(*count, "punchSeconds", motion.punchSeconds);
            }
            Json::Read(*m, "gaugeSpeed", motion.gaugeSpeed);
            Json::Read(*m, "fadeOutSeconds", motion.fadeOutSeconds);
            if(const Json::Value* flip = Json::FindObject(*m, "iconFlip")) {
                Json::Read(*flip, "seconds", motion.iconFlipSeconds);
                Json::Read(*flip, "interval", motion.iconFlipInterval);
            }
        }
        return true;
    }
}    // namespace FruitMagic

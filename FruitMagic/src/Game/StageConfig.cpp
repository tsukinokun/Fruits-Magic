//----------------------------------------------------------------------------
//! @file   StageConfig.cpp
//! @brief  台と屋台の見た目・光・カメラの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/StageConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //--------------------------------------------------------------
        //! 項目があれば輪郭の光り方 {color, intensity, glow} を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（無い要素はそのまま）
        //--------------------------------------------------------------
        void ReadGlow(const Json::Value& obj, const char* key, RimGlowStyle& out) {
            const Json::Value* glow = Json::FindObject(obj, key);
            if(!glow)
                return;
            Json::ReadColor(*glow, "color", out.color);
            Json::Read(*glow, "intensity", out.intensity);
            Json::Read(*glow, "glow", out.glow);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! レジストリに置いた見た目の設定を返します。
    //----------------------------------------------------------------------------
    const StageConfig& GetStageConfig(Tsukino::ECS::Registry& registry) {
        static const StageConfig kDefault;
        return registry.HasContext<StageConfig>() ? registry.GetContext<StageConfig>() : kDefault;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool StageConfig::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "StageConfig"))
            return false;

        if(const Json::Value* cabinet = Json::FindObject(doc, "cabinet")) {
            Json::ReadColor(*cabinet, "floor", floorColor);
            Json::ReadColor(*cabinet, "sideWall", sideWallColor);
            Json::ReadColor(*cabinet, "backPanel", backPanelColor);
            Json::ReadColor(*cabinet, "pusher", pusherColor);
            Json::ReadColor(*cabinet, "tray", trayColor);
            Json::ReadColor(*cabinet, "gutter", gutterColor);
        }
        if(const Json::Value* awning = Json::FindObject(doc, "awning")) {
            Json::Read(*awning, "stripes", awningStripes);
            Json::Read(*awning, "y", awningY);
            Json::Read(*awning, "z", awningZ);
            Json::Read(*awning, "halfDepth", awningHalfDepth);
            Json::Read(*awning, "tiltDegrees", awningTiltDegrees);
            Json::Read(*awning, "overhang", awningOverhang);
            Json::Read(*awning, "halfThickness", awningHalfThickness);
            Json::ReadColor(*awning, "redColor", awningRedColor);
            Json::ReadColor(*awning, "whiteColor", awningWhiteColor);
            Json::Read(*awning, "ballRadius", awningBallRadius);
            Json::Read(*awning, "ballDrop", awningBallDrop);
            awningStripes = std::max(1, awningStripes);
        }
        if(const Json::Value* post = Json::FindObject(doc, "post")) {
            Json::Read(*post, "inset", postInset);
            Json::Read(*post, "back", postBack);
            Json::Read(*post, "halfSize", postHalfSize);
            Json::ReadColor(*post, "color", postColor);
        }
        if(const Json::Value* ground = Json::FindObject(doc, "ground")) {
            Json::Read(*ground, "y", groundY);
            Json::Read(*ground, "halfSize", groundHalfSize);
            Json::ReadColor(*ground, "color", groundColor);
            groundHalfSize = std::max(0.0f, groundHalfSize);
        }
        if(auto it = doc.FindMember("props"); it != doc.MemberEnd() && it->value.IsArray()) {
            props.clear();
            for(const Json::Value& item : it->value.GetArray()) {
                if(!item.IsObject())
                    continue;
                StageProp prop;
                Json::Read(item, "model", prop.model);
                Json::ReadVec(item, "position", prop.position);
                Json::ReadVec(item, "rotation", prop.rotation);
                Json::ReadVec(item, "scale", prop.scale);
                ReadGlow(item, "glow", prop.glow);
                if(!prop.model.empty())
                    props.push_back(prop);
            }
        }
        if(const Json::Value* lantern = Json::FindObject(doc, "lantern")) {
            Json::Read(*lantern, "model", lanternModel);
            if(auto it = lantern->FindMember("positions"); it != lantern->MemberEnd() && it->value.IsArray()) {
                lanternPositions.clear();
                for(const Json::Value& item : it->value.GetArray()) {
                    if(!item.IsObject())
                        continue;
                    float x = 0.0f, y = 0.0f, z = 0.0f;
                    Json::Read(item, "x", x);
                    Json::Read(item, "y", y);
                    Json::Read(item, "z", z);
                    lanternPositions.push_back(hlslpp::float3(x, y, z));
                }
            }
            Json::ReadVec(*lantern, "rotation", lanternRotation);
            Json::ReadVec(*lantern, "scale", lanternScale);
            ReadGlow(*lantern, "glow", lanternGlow);
            Json::ReadVec(*lantern, "lightOffset", lanternLightOffset);
            Json::ReadColor(*lantern, "lightColor", lanternLightColor);
            Json::Read(*lantern, "lightIntensity", lanternLightIntensity);
            Json::Read(*lantern, "lightRange", lanternLightRange);
        }
        if(const Json::Value* sun = Json::FindObject(doc, "sun")) {
            Json::ReadVec(*sun, "direction", sunDirection);
            Json::ReadColor(*sun, "color", sunColor);
            Json::Read(*sun, "intensity", sunIntensity);
            Json::Read(*sun, "shadow", sunShadow);
        }
        if(const Json::Value* lamp = Json::FindObject(doc, "lamp")) {
            Json::ReadVec(*lamp, "position", lampPosition);
            Json::ReadColor(*lamp, "color", lampColor);
            Json::Read(*lamp, "intensity", lampIntensity);
            Json::Read(*lamp, "range", lampRange);
        }
        if(const Json::Value* particles = Json::FindObject(doc, "particles")) {
            Json::Read(*particles, "count", particleCount);
            Json::ReadVec(*particles, "volume", particleVolume);
            Json::ReadColor(*particles, "color", particleColor);
            Json::Read(*particles, "minSize", particleMinSize);
            Json::Read(*particles, "maxSize", particleMaxSize);
            Json::ReadVec(*particles, "drift", particleDrift);
            Json::Read(*particles, "sway", particleSway);
            Json::Read(*particles, "nearFade", particleNearFade);
            particleCount = std::max(0, particleCount);
        }
        if(const Json::Value* camera = Json::FindObject(doc, "camera")) {
            Json::ReadVec(*camera, "position", cameraPosition);
            Json::ReadVec(*camera, "lookAt", cameraLookAt);
            Json::Read(*camera, "near", cameraNear);
            Json::Read(*camera, "far", cameraFar);
        }
        if(const Json::Value* debug = Json::FindObject(doc, "debugCamera")) {
            Json::ReadVec(*debug, "position", debugCameraPosition);
            Json::ReadVec(*debug, "lookAt", debugCameraLookAt);
            Json::Read(*debug, "speed", debugCameraSpeed);
            Json::Read(*debug, "sprint", debugCameraSprint);
        }
        if(const Json::Value* marker = Json::FindObject(doc, "launchMarker")) {
            Json::Read(*marker, "halfThickness", launchMarkerHalfThickness);
            Json::Read(*marker, "opacity", launchMarkerOpacity);
        }
        if(const Json::Value* checker = Json::FindObject(doc, "checkerMarker")) {
            Json::Read(*checker, "y", checkerMarkerY);
            Json::Read(*checker, "offsetZ", checkerMarkerOffsetZ);
            Json::Read(*checker, "halfThickness", checkerMarkerHalfThickness);
            Json::Read(*checker, "halfDepth", checkerMarkerHalfDepth);
            ReadGlow(*checker, "glow", checkerMarkerGlow);
        }
        if(const Json::Value* edges = Json::FindObject(doc, "dropEdges")) {
            Json::ReadColor(*edges, "payoutColor", dropEdges.payoutColor);
            Json::ReadColor(*edges, "gutterColor", dropEdges.gutterColor);
            Json::Read(*edges, "halfWidth", dropEdges.halfWidth);
            Json::Read(*edges, "halfThickness", dropEdges.halfThickness);
            Json::Read(*edges, "topInset", dropEdges.topInset);
            Json::Read(*edges, "intensity", dropEdges.intensity);
            Json::Read(*edges, "glow", dropEdges.glow);
            Json::Read(*edges, "flashIntensity", dropEdges.flashIntensity);
            Json::Read(*edges, "flashGlow", dropEdges.flashGlow);
            Json::Read(*edges, "flashSeconds", dropEdges.flashSeconds);
            dropEdges.flashSeconds = std::max(0.05f, dropEdges.flashSeconds);
        }
        if(const Json::Value* prize = Json::FindObject(doc, "prize")) {
            Json::ReadColor(*prize, "coinColor", coinColor);
            Json::Read(*prize, "coinGlow", coinGlow);
            Json::Read(*prize, "rimPower", prizeRimPower);
        }
        if(const Json::Value* toon = Json::FindObject(doc, "toon")) {
            Json::Read(*toon, "enabled", toonEnabled);
            Json::Read(*toon, "threshold", toonThreshold);
            Json::Read(*toon, "smoothness", toonSmoothness);
            Json::ReadColor(*toon, "shadeColor", toonShadeColor);
            Json::Read(*toon, "coinSpecularSize", toonCoinSpecularSize);
        }
        if(const Json::Value* wall = Json::FindObject(doc, "wallMagic")) {
            Json::ReadColor(*wall, "color", wallColor);
            Json::Read(*wall, "opacity", wallOpacity);
            Json::Read(*wall, "rimIntensity", wallRimIntensity);
            Json::Read(*wall, "glow", wallGlow);
        }
        if(const Json::Value* swell = Json::FindObject(doc, "swellMagic")) {
            Json::ReadColor(*swell, "color", swellColor);
            Json::Read(*swell, "intensity", swellIntensity);
            Json::Read(*swell, "intensityPulse", swellIntensityPulse);
            Json::Read(*swell, "glow", swellGlow);
            Json::Read(*swell, "glowPulse", swellGlowPulse);
            Json::Read(*swell, "pulseSpeed", swellPulseSpeed);
        }

        //--------------------------------------------------------------
        // 屋台の色の組み合わせ。書き方は cabinet・awning・post と同じ（書いた色だけ上書きする）
        //--------------------------------------------------------------
        themes.clear();
        if(const Json::Value* list = Json::FindArray(doc, "themes")) {
            for(const Json::Value& item : list->GetArray()) {
                if(!item.IsObject())
                    continue;
                StageTheme theme;
                Json::Read(item, "name", theme.name);
                auto color = [](const Json::Value* obj, const char* key, std::optional<hlslpp::float3>& out) {
                    hlslpp::float3 value(0.0f, 0.0f, 0.0f);
                    if(obj && Json::ReadColor(*obj, key, value))
                        out = value;
                };
                const Json::Value* cabinet = Json::FindObject(item, "cabinet");
                color(cabinet, "floor", theme.floor);
                color(cabinet, "sideWall", theme.sideWall);
                color(cabinet, "backPanel", theme.backPanel);
                color(cabinet, "pusher", theme.pusher);
                color(cabinet, "tray", theme.tray);
                color(cabinet, "gutter", theme.gutter);
                const Json::Value* awning = Json::FindObject(item, "awning");
                color(awning, "redColor", theme.awningStripe);
                color(awning, "whiteColor", theme.awningBase);
                color(Json::FindObject(item, "post"), "color", theme.post);
                themes.push_back(theme);
            }
        }
        return true;
    }

    //----------------------------------------------------------------------------
    //! おひっこしの回数に合うテーマの色を、台・屋根・柱の色に上書きします。
    //----------------------------------------------------------------------------
    void StageConfig::ApplyTheme(int moveCount) {
        if(themes.empty())
            return;
        const StageTheme& theme = themes[static_cast<size_t>(std::max(0, moveCount)) % themes.size()];
        auto apply              = [](const std::optional<hlslpp::float3>& from, hlslpp::float3& to) {
            if(from)
                to = *from;
        };
        apply(theme.floor, floorColor);
        apply(theme.sideWall, sideWallColor);
        apply(theme.backPanel, backPanelColor);
        apply(theme.pusher, pusherColor);
        apply(theme.tray, trayColor);
        apply(theme.gutter, gutterColor);
        apply(theme.awningStripe, awningRedColor);
        apply(theme.awningBase, awningWhiteColor);
        apply(theme.post, postColor);
    }

    //----------------------------------------------------------------------------
    //! おひっこしの回数に合うテーマの名前を返します。
    //----------------------------------------------------------------------------
    std::wstring StageConfig::ThemeName(int moveCount) const {
        if(themes.empty())
            return std::wstring();
        return themes[static_cast<size_t>(std::max(0, moveCount)) % themes.size()].name;
    }
}    // namespace FruitMagic

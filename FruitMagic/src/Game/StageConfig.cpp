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
        if(const Json::Value* lantern = Json::FindObject(doc, "lantern")) {
            Json::ReadArray(*lantern, "x", lanternX);
            Json::Read(*lantern, "drop", lanternDrop);
            Json::Read(*lantern, "back", lanternBack);
            Json::ReadVec(*lantern, "halfSize", lanternHalfSize);
            Json::ReadColor(*lantern, "color", lanternColor);
            ReadGlow(*lantern, "glow", lanternGlow);
            Json::Read(*lantern, "stringOffset", lanternStringOffset);
            Json::ReadVec(*lantern, "stringHalfSize", lanternStringHalfSize);
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
        if(const Json::Value* prize = Json::FindObject(doc, "prize")) {
            Json::ReadColor(*prize, "coinColor", coinColor);
            Json::Read(*prize, "coinGlow", coinGlow);
            Json::Read(*prize, "rimPower", prizeRimPower);
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
        return true;
    }
}    // namespace FruitMagic

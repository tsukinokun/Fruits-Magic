//----------------------------------------------------------------------------
//! @file   EffectsConfig.cpp
//! @brief  光の粒と画面の光り方の設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/EffectsConfig.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool EffectsConfig::Load(const std::string& path) {
        m_presets.clear();

        Json::Document     doc;
        const Json::Value* presets = Json::ParseFile(path, doc, "EffectsConfig") ? Json::FindObject(doc, "presets") : nullptr;
        if(!presets) {
            Tsukino::Core::Log::Warn("EffectsConfig: cannot read \"presets\" from " + path + ". No sparkles will appear.");
            return false;
        }
        Json::Read(doc, "maxParticles", m_maxParticles);

        if(const Json::Value* system = Json::FindObject(doc, "system")) {
            Json::ReadVec(*system, "castPosition", motion.castPosition);
            Json::Read(*system, "dropEffectY", motion.dropEffectY);
            Json::Read(*system, "dropEffectAhead", motion.dropEffectAhead);
            Json::Read(*system, "idleInterval", motion.idleInterval);
            Json::Read(*system, "idleMinY", motion.idleMinY);
            Json::Read(*system, "idleRise", motion.idleRise);
            Json::Read(*system, "flashFadeSpeed", motion.flashFadeSpeed);
            Json::Read(*system, "drag", motion.drag);
            Json::Read(*system, "startHeight", motion.startHeight);
            Json::Read(*system, "upMin", motion.upMin);
            Json::Read(*system, "lifeMin", motion.lifeMin);
            Json::Read(*system, "sizeMin", motion.sizeMin);
            Json::Read(*system, "sizeRange", motion.sizeRange);
            Json::Read(*system, "shrinkMin", motion.shrinkMin);
            motion.idleInterval = std::max(0.05f, motion.idleInterval);
        }
        if(const Json::Value* style = Json::FindObject(doc, "popup")) {
            Json::Read(*style, "coinGatherSeconds", popup.coinGatherSeconds);
            Json::Read(*style, "maxPopups", popup.maxPopups);
            Json::Read(*style, "y", popup.y);
            Json::Read(*style, "rise", popup.rise);
            Json::Read(*style, "fadeStart", popup.fadeStart);
            Json::ReadColor(*style, "coinColor", popup.coinColor);
            Json::ReadColor(*style, "outlineColor", popup.outlineColor);
            Json::Read(*style, "outlineWidth", popup.outlineWidth);
            Json::Read(*style, "coinLife", popup.coinLife);
            Json::Read(*style, "coinScaleStep", popup.coinScaleStep);
            Json::Read(*style, "coinScaleMaxBonus", popup.coinScaleMaxBonus);
            Json::Read(*style, "fruitLife", popup.fruitLife);
            Json::Read(*style, "fruitLighten", popup.fruitLighten);
            Json::Read(*style, "fruitScale", popup.fruitScale);
            Json::Read(*style, "variantScale", popup.variantScale);
            popup.maxPopups = std::max(1, popup.maxPopups);
            popup.coinLife  = std::max(0.05f, popup.coinLife);
            popup.fruitLife = std::max(0.05f, popup.fruitLife);
        }
        m_maxParticles = std::max(0, m_maxParticles);

        for(auto it = presets->MemberBegin(); it != presets->MemberEnd(); ++it) {
            const Json::Value& p = it->value;
            if(!p.IsObject())
                continue;

            EffectPreset preset;
            std::string  texture;
            if(Json::Read(p, "texture", texture) && texture == "glow")
                preset.texture = SparkleTexture::Glow;
            Json::Read(p, "count", preset.count);
            Json::Read(p, "speed", preset.speed);
            Json::Read(p, "up", preset.up);
            Json::Read(p, "gravity", preset.gravity);
            Json::Read(p, "life", preset.life);
            Json::Read(p, "size", preset.size);
            Json::Read(p, "spread", preset.spread);
            preset.count = std::max(0, preset.count);
            preset.life  = std::max(0.05f, preset.life);

            if(const Json::Value* colors = Json::FindArray(p, "colors")) {
                for(const Json::Value& c : colors->GetArray()) {
                    hlslpp::float3 color(1.0f, 1.0f, 1.0f);
                    if(Json::ReadColorValue(c, color))
                        preset.colors.push_back(color);
                }
            }
            if(preset.colors.empty())
                preset.colors.push_back(hlslpp::float3(1.0f, 1.0f, 1.0f));

            hlslpp::float4 flash(1.0f, 1.0f, 1.0f, 0.3f);
            if(Json::ReadColor(p, "flash", flash))
                preset.flash = flash;

            m_presets[it->name.GetString()] = preset;
        }

        Tsukino::Core::Log::Info("EffectsConfig: loaded " + std::to_string(m_presets.size()) + " presets.");
        return true;
    }

    //----------------------------------------------------------------------------
    //! プリセットを探します。
    //----------------------------------------------------------------------------
    const EffectPreset* EffectsConfig::Find(const std::string& name) const {
        auto it = m_presets.find(name);
        return (it != m_presets.end()) ? &it->second : nullptr;
    }
}    // namespace FruitMagic

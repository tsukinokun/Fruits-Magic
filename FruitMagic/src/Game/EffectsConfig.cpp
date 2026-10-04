//----------------------------------------------------------------------------
//! @file   EffectsConfig.cpp
//! @brief  光の粒と画面の光り方の設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/EffectsConfig.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>

#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

#include <algorithm>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

        //--------------------------------------------------------------
        //! 項目があれば数値を読み込みます。
        //! @param  [in]     obj 読み込み元のオブジェクト
        //! @param  [in]     key 項目名
        //! @param  [in,out] out 読み込み先（項目が無ければそのまま）
        //--------------------------------------------------------------
        template <class T>
        void ReadNumber(const rj::Value& obj, const char* key, T& out) {
            auto it = obj.FindMember(key);
            if(it != obj.MemberEnd() && it->value.IsNumber())
                out = static_cast<T>(it->value.GetDouble());
        }

        //--------------------------------------------------------------
        //! {"r","g","b"(,"a")} を読みます。
        //! @param  [in] obj      読み込み元のオブジェクト
        //! @param  [in] fallback 項目が無いときの値
        //! @return 色
        //--------------------------------------------------------------
        hlslpp::float4 ReadColor(const rj::Value& obj, const hlslpp::float4& fallback) {
            float r = fallback.x, g = fallback.y, b = fallback.z, a = fallback.w;
            ReadNumber(obj, "r", r);
            ReadNumber(obj, "g", g);
            ReadNumber(obj, "b", b);
            ReadNumber(obj, "a", a);
            return hlslpp::float4(r, g, b, a);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool EffectsConfig::Load(const std::string& path) {
        m_presets.clear();

        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        auto presets = (text.empty() || doc.HasParseError() || !doc.IsObject()) ? doc.MemberEnd() : doc.FindMember("presets");
        if(presets == doc.MemberEnd() || !presets->value.IsObject()) {
            Tsukino::Core::Log::Warn("EffectsConfig: cannot read \"presets\" from " + path + ". No sparkles will appear.");
            return false;
        }
        ReadNumber(doc, "maxParticles", m_maxParticles);
        m_maxParticles = std::max(0, m_maxParticles);

        for(auto it = presets->value.MemberBegin(); it != presets->value.MemberEnd(); ++it) {
            const rj::Value& p = it->value;
            if(!p.IsObject())
                continue;

            EffectPreset preset;
            auto         texture = p.FindMember("texture");
            if(texture != p.MemberEnd() && texture->value.IsString() && std::string(texture->value.GetString()) == "glow")
                preset.texture = SparkleTexture::Glow;
            ReadNumber(p, "count", preset.count);
            ReadNumber(p, "speed", preset.speed);
            ReadNumber(p, "up", preset.up);
            ReadNumber(p, "gravity", preset.gravity);
            ReadNumber(p, "life", preset.life);
            ReadNumber(p, "size", preset.size);
            ReadNumber(p, "spread", preset.spread);
            preset.count = std::max(0, preset.count);
            preset.life  = std::max(0.05f, preset.life);

            auto colors = p.FindMember("colors");
            if(colors != p.MemberEnd() && colors->value.IsArray()) {
                for(const rj::Value& c : colors->value.GetArray()) {
                    if(c.IsObject()) {
                        const hlslpp::float4 color = ReadColor(c, hlslpp::float4(1, 1, 1, 1));
                        preset.colors.push_back(color.xyz);
                    }
                }
            }
            if(preset.colors.empty())
                preset.colors.push_back(hlslpp::float3(1.0f, 1.0f, 1.0f));

            auto flash = p.FindMember("flash");
            if(flash != p.MemberEnd() && flash->value.IsObject())
                preset.flash = ReadColor(flash->value, hlslpp::float4(1, 1, 1, 0.3f));

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

//----------------------------------------------------------------------------
//! @file   CollectionConfig.cpp
//! @brief  果物のバリエーションと図鑑ボーナスの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/CollectionConfig.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/external/rapidjson/document.h>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        namespace rj = CEREAL_RAPIDJSON_NAMESPACE;

        //--------------------------------------------------------------
        //! 「通常」だけの既定のバリエーションを返します。
        //! @return 通常のバリエーション
        //--------------------------------------------------------------
        VariantDef MakeNormalVariant() {
            VariantDef normal;
            normal.id = "normal";
            return normal;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! この果物をこのバリエーションで見せる色を返します。
    //----------------------------------------------------------------------------
    hlslpp::float3 VariantDef::ColorOf(const FruitDef& fruit) const {
        switch(colorMode) {
            case VariantColorMode::Shiny: return fruit.shinyColor;
            case VariantColorMode::Fixed: return color;
            case VariantColorMode::Base:
            default: return fruit.color;
        }
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。読めなかった場合は「通常」だけの1種類になります。
    //----------------------------------------------------------------------------
    bool CollectionConfig::Load(const std::string& path) {
        m_variants.clear();

        const std::string text = ReadDataText(path);
        rj::Document      doc;
        doc.Parse(text.c_str());
        auto variants = (text.empty() || doc.HasParseError() || !doc.IsObject()) ? doc.MemberEnd() : doc.FindMember("variants");
        if(variants == doc.MemberEnd() || !variants->value.IsArray()) {
            Tsukino::Core::Log::Warn("CollectionConfig: cannot read variants from " + path + ". Only the normal variant is used.");
            m_variants.push_back(MakeNormalVariant());
            return false;
        }

        for(const rj::Value& v : variants->value.GetArray()) {
            if(!v.IsObject())
                continue;

            VariantDef def;
            auto       id = v.FindMember("id");
            def.id        = (id != v.MemberEnd() && id->value.IsString()) ? id->value.GetString() : "variant" + std::to_string(m_variants.size());

            auto name = v.FindMember("name");
            if(name != v.MemberEnd() && name->value.IsString())
                def.name = Utf8ToWide(name->value.GetString());

            auto chance = v.FindMember("chance");
            if(chance != v.MemberEnd() && chance->value.IsNumber())
                def.chance = static_cast<float>(chance->value.GetDouble());

            auto multiplier = v.FindMember("valueMultiplier");
            if(multiplier != v.MemberEnd() && multiplier->value.IsNumber())
                def.valueMultiplier = static_cast<int>(multiplier->value.GetDouble());

            auto glow = v.FindMember("glow");
            if(glow != v.MemberEnd() && glow->value.IsNumber())
                def.glow = static_cast<float>(glow->value.GetDouble());

            // "base" / "shiny" はモード名、それ以外（"gold" や {r,g,b}）は固定色
            auto color = v.FindMember("color");
            if(color != v.MemberEnd()) {
                if(color->value.IsString() && std::string(color->value.GetString()) == "shiny") {
                    def.colorMode = VariantColorMode::Shiny;
                } else if(color->value.IsString() && std::string(color->value.GetString()) == "base") {
                    def.colorMode = VariantColorMode::Base;
                } else if(color->value.IsObject()) {
                    def.colorMode = VariantColorMode::Fixed;
                    auto r        = color->value.FindMember("r");
                    auto g        = color->value.FindMember("g");
                    auto b        = color->value.FindMember("b");
                    def.color     = hlslpp::float3((r != color->value.MemberEnd() && r->value.IsNumber()) ? static_cast<float>(r->value.GetDouble()) : 1.0f,
                                               (g != color->value.MemberEnd() && g->value.IsNumber()) ? static_cast<float>(g->value.GetDouble()) : 1.0f,
                                               (b != color->value.MemberEnd() && b->value.IsNumber()) ? static_cast<float>(b->value.GetDouble()) : 1.0f);
                } else {
                    def.colorMode = VariantColorMode::Fixed;    // "gold" など。色は既定の金色
                }
            }

            m_variants.push_back(def);
        }

        if(m_variants.empty())
            m_variants.push_back(MakeNormalVariant());

        auto bonus = doc.FindMember("manaBonusPerEntry");
        if(bonus != doc.MemberEnd() && bonus->value.IsNumber())
            m_manaBonusPerEntry = static_cast<float>(bonus->value.GetDouble());

        Tsukino::Core::Log::Info("CollectionConfig: loaded " + std::to_string(m_variants.size()) + " variants.");
        return true;
    }

    //----------------------------------------------------------------------------
    //! バリエーションを抽選します。
    //----------------------------------------------------------------------------
    int CollectionConfig::PickVariant(std::mt19937& rng) const {
        // 確率の低いものから順に判定する方が直感に合うよう、後ろ（レア）から判定する
        std::uniform_real_distribution<float> roll(0.0f, 1.0f);
        for(size_t i = m_variants.size(); i-- > 1;) {
            if(roll(rng) < m_variants[i].chance)
                return static_cast<int>(i);
        }
        return 0;
    }

    //----------------------------------------------------------------------------
    //! 果物名にバリエーション名を付けた表示名を返します（例: 「いちご（色違い）」）。
    //----------------------------------------------------------------------------
    std::wstring CollectionConfig::DisplayName(const FruitDef& fruit, int variantIndex) const {
        if(variantIndex <= 0 || variantIndex >= static_cast<int>(m_variants.size()) || m_variants[variantIndex].name.empty())
            return fruit.name;
        return fruit.name + L"（" + m_variants[variantIndex].name + L"）";
    }
}    // namespace FruitMagic

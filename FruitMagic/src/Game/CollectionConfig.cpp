//----------------------------------------------------------------------------
//! @file   CollectionConfig.cpp
//! @brief  果物のバリエーションと図鑑ボーナスの設定の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/CollectionConfig.hpp>

#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/Log.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
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

        Json::Document     doc;
        const Json::Value* variants = Json::ParseFile(path, doc, "CollectionConfig") ? Json::FindArray(doc, "variants") : nullptr;
        if(!variants) {
            Tsukino::Core::Log::Warn("CollectionConfig: cannot read variants from " + path + ". Only the normal variant is used.");
            m_variants.push_back(MakeNormalVariant());
            return false;
        }

        for(const Json::Value& v : variants->GetArray()) {
            if(!v.IsObject())
                continue;

            VariantDef def;
            if(!Json::Read(v, "id", def.id))
                def.id = "variant" + std::to_string(m_variants.size());
            Json::Read(v, "name", def.name);
            Json::Read(v, "chance", def.chance);
            Json::Read(v, "valueMultiplier", def.valueMultiplier);
            Json::Read(v, "glow", def.glow);

            // "base" / "shiny" はモード名、それ以外（"gold" や {r,g,b}）は固定色
            if(const Json::Value* color = Json::Find(v, "color")) {
                if(color->IsString() && std::string(color->GetString()) == "shiny") {
                    def.colorMode = VariantColorMode::Shiny;
                } else if(color->IsString() && std::string(color->GetString()) == "base") {
                    def.colorMode = VariantColorMode::Base;
                } else if(color->IsObject()) {
                    def.colorMode = VariantColorMode::Fixed;
                    def.color     = hlslpp::float3(1.0f, 1.0f, 1.0f);    // 書かれていない成分は 1
                    Json::ReadColorValue(*color, def.color);
                } else {
                    def.colorMode = VariantColorMode::Fixed;    // "gold" など。色は既定の金色
                }
            }

            m_variants.push_back(def);
        }

        if(m_variants.empty())
            m_variants.push_back(MakeNormalVariant());

        Json::Read(doc, "manaBonusPerEntry", m_manaBonusPerEntry);

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
    //! id からバリエーションの添字を探します。
    //----------------------------------------------------------------------------
    int CollectionConfig::FindIndex(const std::string& id) const {
        for(size_t i = 0; i < m_variants.size(); ++i) {
            if(m_variants[i].id == id)
                return static_cast<int>(i);
        }
        return -1;
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

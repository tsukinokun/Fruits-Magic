//----------------------------------------------------------------------------
//! @file   UiFonts.cpp
//! @brief  画面の文字に使うフォントの実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/UiFonts.hpp>

#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>

#include <string>

// 名前空間 : FruitMagic
namespace FruitMagic {

    namespace {

        //--------------------------------------------------------------
        //! フォントを1つ読み込みます。読み込めなければ Warn を出して空のハンドルを返します。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] path         フォントのファイル（.dfont。空なら読み込まない）
        //! @return フォントのハンドル
        //--------------------------------------------------------------
        Tsukino::Asset::AssetHandle LoadFont(Tsukino::Asset::AssetManager& assetManager, const std::string& path) {
            if(path.empty())
                return {};
            Tsukino::Asset::AssetHandle handle = assetManager.Load(Tsukino::Core::Path(path));
            if(!handle.IsValid()) {
                Tsukino::Core::Log::Warn("UiFonts: failed to load " + path + ". The engine default font is used.");
                return {};
            }
            return handle;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! フォントを読み込みます。
    //----------------------------------------------------------------------------
    void UiFonts::Load(Tsukino::Asset::AssetManager& assetManager, const UiConfig& ui) {
        regular = LoadFont(assetManager, ui.fontRegularPath);
        bold    = LoadFont(assetManager, ui.fontBoldPath);
    }

    //----------------------------------------------------------------------------
    //! レジストリに置いたフォントのうち、太さに合うものを返します。
    //----------------------------------------------------------------------------
    Tsukino::Asset::AssetHandle GetUiFont(Tsukino::ECS::Registry& registry, bool isBold) {
        if(!registry.HasContext<UiFonts>())
            return {};
        return registry.GetContext<UiFonts>().Get(isBold);
    }
}    // namespace FruitMagic

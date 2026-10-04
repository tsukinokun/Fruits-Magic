//----------------------------------------------------------------------------
//! @file   SaveData.hpp
//! @brief  セーブデータの保存と読み込み
//! @detail プレイヤーの資源（GameState）を JSON で保存します。果物・バリエーション・強化は
//!         添字ではなく id で保存するので、定義データを足したり並びが変わったりしても読み込めます。
//!         台の上の景品（物理の状態）は保存せず、起動のたびに初期配置から始めます。
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic::SaveData
namespace FruitMagic::SaveData {

    //! 既定のセーブファイルのパスを返します（アセットルートの Saves/save.json）。
    //! @return セーブファイルのパス
    std::string DefaultPath();

    //! 今の時刻を返します（1970年からの秒数）。
    //! @return 今の時刻（秒）
    long long NowSeconds();

    //! プレイヤーの資源を保存します。いったん一時ファイルに書いてから置き換えるので、途中で落ちても前のセーブは壊れません。
    //! @param  [in] registry レジストリ（GameState・FruitCatalog・CollectionConfig を参照する）
    //! @param  [in] path     セーブファイルのパス
    //! @return 保存できたら true
    bool Save(Tsukino::ECS::Registry& registry, const std::string& path);

    //! セーブファイルを読み込み、GameState に反映します。
    //! @param  [in]  registry レジストリ（GameState に書き込み、FruitCatalog・CollectionConfig を参照する）
    //! @param  [in]  path     セーブファイルのパス
    //! @param  [out] savedAt  保存した時刻（1970年からの秒数）
    //! @return 読み込めたら true（ファイルが無い・壊れている場合は false で、GameState は変えない）
    bool Load(Tsukino::ECS::Registry& registry, const std::string& path, long long& savedAt);
}    // namespace FruitMagic::SaveData

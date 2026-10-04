//----------------------------------------------------------------------------
//! @file   ZukanElementComponent.hpp
//! @brief  図鑑画面を構成する要素（パネル・文字・色見本）と、開閉ボタンのコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 図鑑画面の要素の種類です。
    enum class ZukanElementKind {
        Panel,         // 背景のパネル（スプライト）
        StaticText,    // 決まった文字（タイトル・列の見出し）
        RowName,       // 行頭の果物名（登録済みなら名前、未登録なら「？？？」）
        Swatch,        // 枠の色見本（スプライト。登録済みならその色）
        Count,         // 枠の収穫数（「×3」）
        Footer,        // 下部の集計（登録数・図鑑ボーナス）
    };

    //! 図鑑画面の要素です。ZukanSystem が開閉に合わせて表示を切り替え、内容を更新します。
    //! @note 画面スプライトはスケール 0 で描画されず、文字は空文字で描画されない（エンジンの描画システムの挙動）ので、
    //!       閉じるときはスケールを 0 に、文字を空にする。開くときに戻すスケールを覚えておく
    struct ZukanElementComponent {
        ZukanElementKind kind         = ZukanElementKind::StaticText;              // 種類
        hlslpp::float3   openScale    = hlslpp::float3(1.0f, 1.0f, 1.0f);          // 開いているときのスケール（スプライト用）
        std::wstring     text;                                                    // StaticText の文字
        int              fruitIndex   = -1;                                       // RowName / Swatch / Count の果物の添字
        int              variantIndex = -1;                                       // Swatch / Count のバリエーションの添字
    };

    //! 図鑑を開閉するボタンです。SpriteComponent と PointerTargetComponent と一緒に付けます。
    struct ZukanButtonComponent {
        Tsukino::ECS::Entity label = entt::null;    // ボタンの上の文字（FontComponent を持つエンティティ）
    };
}    // namespace FruitMagic::ECS

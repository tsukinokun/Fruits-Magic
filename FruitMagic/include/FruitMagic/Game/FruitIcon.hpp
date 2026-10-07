//----------------------------------------------------------------------------
//! @file   FruitIcon.hpp
//! @brief  画面に出す果物（UI の層に描く 3D の果物）を作る・入れ替える関数
//! @detail 図鑑・画面の横のパネル・「〇〇 ゲット！」・カットインが、色の四角の代わりに本物の果物を出すのに使います。
//!         置き台（FruitIconComponent と ScreenModelComponent を持つエンティティ）を作っておき、
//!         出す果物が変わったときに SetFruitIcon で見た目を作り直します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class Registry;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 画面に出す果物の置き台を作ります（中身はまだ空。SetFruitIcon で果物を入れる）。
    //! @param  [in] registry       レジストリ
    //! @param  [in] screenPosition 出す位置（画面ピクセル。anchor があるときは anchor からのずらし量）
    //! @param  [in] sortOrder      画面スプライト・文字との重ね順
    //! @param  [in] anchor         位置と切り取りの基準にする UI の部品（スクロールの中など。無ければ entt::null）
    //! @return 置き台のエンティティ
    Tsukino::ECS::Entity CreateFruitIconHolder(Tsukino::ECS::Registry& registry, const hlslpp::float2& screenPosition, int sortOrder,
                                               Tsukino::ECS::Entity anchor = entt::null);

    //! 置き台に出す果物を設定します。今と同じなら何もしません（毎フレーム呼んでよい）。
    //! @param  [in] registry     レジストリ
    //! @param  [in] holder       置き台
    //! @param  [in] fruitIndex   果物の添字（-1 なら空にする）
    //! @param  [in] variantIndex バリエーションの添字
    //! @param  [in] silhouette   黒いシルエットで出すか
    //! @param  [in] sizePixels   大きさ（果物の外形のいちばん長い向きのピクセル数）
    void SetFruitIcon(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity holder, int fruitIndex, int variantIndex, bool silhouette, float sizePixels);

    //! 置き台の果物を見せるか隠すかを切り替えます。
    //! @param  [in] registry レジストリ
    //! @param  [in] holder   置き台
    //! @param  [in] visible  見せるなら true
    void SetFruitIconVisible(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity holder, bool visible);
}    // namespace FruitMagic

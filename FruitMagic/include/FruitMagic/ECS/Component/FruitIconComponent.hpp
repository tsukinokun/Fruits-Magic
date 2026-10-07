//----------------------------------------------------------------------------
//! @file   FruitIconComponent.hpp
//! @brief  画面に出す果物（UI の層に描く 3D の果物）の置き台を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面に出す果物の置き台です。ScreenModelComponent と一緒に付け、果物の見た目を子に付けます。
    //! 作り方・中身の入れ替えは FruitIcon.hpp の関数で行い、回すのは FruitIconSystem です。
    struct FruitIconComponent {
        int                  fruitIndex   = -1;            // 出している果物の添字（-1 は何も出していない）
        int                  variantIndex = 0;             // 出している果物のバリエーションの添字
        bool                 silhouette   = false;         // 黒いシルエットで出しているか（図鑑のまだ取っていない枠）
        float                sizePixels   = 0.0f;          // 出している大きさ（果物の外形のいちばん長い向きのピクセル数）
        bool                 visible      = true;          // 見せるか（作り直した見た目にも引き継ぐ）
        float                spinSpeed    = 0.0f;          // 回る速さ（度/秒。0 なら FruitIconSystem は回さない）
        float                tilt         = 15.0f;         // 回すときに手前へ傾ける角度（度。上から少し見下ろす）
        float                spin         = 0.0f;          // 今の回転（度）
        Tsukino::ECS::Entity visual       = entt::null;    // 果物の見た目（置き台の子。PrizeFactory::CreateFruitVisual で作る）
    };
}    // namespace FruitMagic::ECS

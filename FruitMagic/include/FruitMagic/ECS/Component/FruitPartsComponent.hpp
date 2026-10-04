//----------------------------------------------------------------------------
//! @file   FruitPartsComponent.hpp
//! @brief  果物に付けた飾りのパーツ（ヘタ・葉など）を覚えておくコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 果物に付けた飾りのパーツです。パーツは果物を親にした見た目だけのエンティティで、
    //! 果物を消すときに PrizeFactory::DestroyPrize が一緒に消します。
    struct FruitPartsComponent {
        std::vector<Tsukino::ECS::Entity> parts;    // 飾りのパーツ
    };
}    // namespace FruitMagic::ECS

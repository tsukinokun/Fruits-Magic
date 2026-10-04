//----------------------------------------------------------------------------
//! @file   ZukanRegisteredEvent.hpp
//! @brief  図鑑の枠に初めて登録されたことを知らせるイベント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 果物 × バリエーションの枠を初めて収穫したことを知らせるイベントです。HarvestSystem が発行します。
    struct ZukanRegisteredEvent {
        int fruitIndex   = -1;    // FruitCatalog::Fruits() の添字
        int variantIndex = 0;     // CollectionConfig::Variants() の添字
    };
}    // namespace FruitMagic::ECS

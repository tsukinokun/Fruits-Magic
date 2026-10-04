//----------------------------------------------------------------------------
//! @file   MagicCastEvent.hpp
//! @brief  魔法が撃たれたことを知らせるイベント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法が撃たれたことを知らせるイベントです。MagicInputSystem がマナを引いてから発行し、
    //! 効果を実装するシステム（ShakeMagicSystem など）が自分の id の魔法だけを受け取ります。
    struct MagicCastEvent {
        int magicIndex = -1;    // MagicCatalog::Magics() の添字
    };
}    // namespace FruitMagic::ECS

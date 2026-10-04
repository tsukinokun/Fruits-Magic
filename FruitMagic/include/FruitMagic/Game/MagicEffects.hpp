//----------------------------------------------------------------------------
//! @file   MagicEffects.hpp
//! @brief  効果中の魔法が台に与えている影響
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! 効果中の魔法が台に与えている影響です。魔法の効果システムが書き、台を動かす側（PusherScene）が読みます。
    //! Registry のコンテキストに置きます。
    struct MagicEffects {
        float pusherAmplitudeBonus = 0.0f;    // プッシャーの振幅に足す量（cm）。魔法「ふくらむ」
    };

    //! 台のプッシャーのエンティティです。プッシャーと一緒に動く物（ジャックポット穴）が位置を読むのに使います。
    //! シーンが Registry のコンテキストに置きます。
    struct PusherRef {
        Tsukino::ECS::Entity entity{entt::null};    // プッシャー
    };
}    // namespace FruitMagic

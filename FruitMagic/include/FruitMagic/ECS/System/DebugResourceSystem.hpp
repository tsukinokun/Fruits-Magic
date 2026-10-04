//----------------------------------------------------------------------------
//! @file   DebugResourceSystem.hpp
//! @brief  （開発用）F2 キーでコインと果実を増やすシステム
//! @detail 強化や上位ランクの果物をすぐ確かめるためのものです。シーンは Debug ビルドでだけ登録します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! F2 キーで手持ちのコインと果実を 100 ずつ増やすシステムです。
    class DebugResourceSystem : public Tsukino::ECS::ISystem {
    public:

        //! F2 が押されたらコインと果実を増やします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

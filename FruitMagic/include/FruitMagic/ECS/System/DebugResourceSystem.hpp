//----------------------------------------------------------------------------
//! @file   DebugResourceSystem.hpp
//! @brief  （開発用）F2 キーでコイン・FP・星のたねを増やし、マナを満タンにするシステム
//! @detail 強化・魔法・上位ランクの果物をすぐ確かめるためのものです。シーンは Debug ビルドでだけ登録します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! F2 キーで手持ちのコインと FP を 100 ずつ、星のたねを 5 増やし、マナを満タンにするシステムです。
    class DebugResourceSystem : public Tsukino::ECS::ISystem {
    public:

        //! F2 が押されたらコイン・FP・星のたねを増やし、マナを満タンにします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   DebugTreeLevelSystem.hpp
//! @brief  （開発用）F2 キーで果樹の段階を切り替えるシステム
//! @detail 強化（M5）ができるまで、上位ランクの果物の出現を確かめるためのものです。
//!         シーンは Debug ビルドでだけ登録します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! F2 キーで果樹の段階を 0 → 最大 → 0 と切り替えるシステムです。
    class DebugTreeLevelSystem : public Tsukino::ECS::ISystem {
    public:

        //! F2 が押されたら果樹の段階を1つ上げます（最大の次は 0 に戻ります）。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

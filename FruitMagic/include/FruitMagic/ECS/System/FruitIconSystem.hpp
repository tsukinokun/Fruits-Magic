//----------------------------------------------------------------------------
//! @file   FruitIconSystem.hpp
//! @brief  画面に出す果物（FruitIconComponent）を回すシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面に出す果物を回すシステムです。spinSpeed が 0 より大きい置き台を、手前へ少し傾けたまま Y 軸で回します
    //! （spinSpeed が 0 の置き台の向きは、使う側が決める）。
    class FruitIconSystem : public Tsukino::ECS::ISystem {
    public:

        //! 置き台を回します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

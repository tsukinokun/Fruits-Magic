//----------------------------------------------------------------------------
//! @file   ZukanSystem.hpp
//! @brief  図鑑画面の開閉と表示内容の更新を行うシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 図鑑画面のシステムです。Tab キーか右上のボタンで開閉し、開いている間は
    //! 果物 × バリエーションの各枠の色見本・収穫数と、下部の集計を更新します。
    class ZukanSystem : public Tsukino::ECS::ISystem {
    public:

        //! 開閉の入力を読み、図鑑の要素を更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

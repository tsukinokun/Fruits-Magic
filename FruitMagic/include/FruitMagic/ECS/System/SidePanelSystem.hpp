//----------------------------------------------------------------------------
//! @file   SidePanelSystem.hpp
//! @brief  画面の左右のパネル（台のようす・進み具合とおすすめ）を更新するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面の左右のパネルを更新するシステムです。
    //! 左は最近とれた果物と台の上の果物、右は図鑑の進み具合・おすすめの強化・妖精とおるすばんを、
    //! SidePanelElementComponent の付いた文字と画面スプライトに書きます。
    //! 画面（図鑑・強化など）を開いている間は、パネルを隠します。
    class SidePanelSystem : public Tsukino::ECS::ISystem {
    public:

        //! パネルの文字と色を更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

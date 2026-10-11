//----------------------------------------------------------------------------
//! @file   SidePanelSystem.hpp
//! @brief  画面の左右のパネル（台のようす・進み具合とおすすめ）を更新するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <cstddef>

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

    private:

        bool        m_wasVisible     = false;    // 前のフレームにパネルを出していたか（出した瞬間に端からスライドインさせる）
        std::size_t m_recentAdded    = 0;        // 前のフレームまでに見た、最近とれた果物の記録の件数（増えたら先頭の行をポップ）
        float       m_zukanRatio     = -1.0f;    // 図鑑の進み具合のバーに出している割合（-1 はまだ出していない。実際の割合へ追いつかせる）
        int         m_lastRegistered = -1;       // 前のフレームの図鑑の登録数（増えたら数を弾ませる。-1 はまだ数えていない）
    };
}    // namespace FruitMagic::ECS

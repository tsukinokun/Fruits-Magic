//----------------------------------------------------------------------------
//! @file   MenuSystem.hpp
//! @brief  画面（図鑑・強化）の開閉と表示・非表示を切り替えるシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面の開閉のシステムです。開閉ボタンのクリックか、ボタンに割り当てたキーで画面を開閉し
    //! （別の画面を開くと前の画面は閉じる）、画面の要素（MenuPageComponent）の表示・非表示を切り替えます。
    //! 開いている画面の中身は、画面ごとのシステム（ZukanSystem・UpgradeSystem）が書きます。
    //! 画面の行のスクロール（MenuPageComponent を持つ ScrollViewComponent）は、開いている画面のものだけ受け付けさせ、開いたときは一番上へ戻します。
    class MenuSystem : public Tsukino::ECS::ISystem {
    public:

        //! 開閉の入力を読み、画面の要素の表示を切り替えます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

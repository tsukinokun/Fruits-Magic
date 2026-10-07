//----------------------------------------------------------------------------
//! @file   SlotMachineSystem.hpp
//! @brief  ルーレット（3リールのスロット）を表示するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! ルーレットを3リールのスロットとして見せるシステムです（見せるだけで、結果は RouletteSystem が決める）。
    //! RouletteState の止める絵柄と時刻に合わせて、リールを流し、少し行き過ぎて戻るように止めます。
    //! リーチの3列目はゆっくり流して縁を点滅させ、当たりの果物はスロットから台の上の出る位置へ飛ばします。
    //! ジャックポットチャンスの間は、真ん中の大きなスロットを出します。図鑑などの画面を開いている間は、絵柄（3D）を隠します。
    class SlotMachineSystem : public Tsukino::ECS::ISystem {
    public:

        //! スロットの表示を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        float m_time = 0.0f;    // 点滅に使う時間（秒）
    };
}    // namespace FruitMagic::ECS

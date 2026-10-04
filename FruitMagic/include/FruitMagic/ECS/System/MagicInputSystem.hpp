//----------------------------------------------------------------------------
//! @file   MagicInputSystem.hpp
//! @brief  数字キーと魔法ボタンで魔法を撃つシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 数字キー 1〜5 と画面下の魔法ボタンで魔法を撃つシステムです。
    //! 解放済みで、マナが足り、ほかの魔法の効果中でなければ、マナを引いて MagicCastEvent を発行します。
    class MagicInputSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を発行するイベントバス
        explicit MagicInputSystem(Tsukino::ECS::EventBus& eventBus);

        //! 入力を読み、撃てる魔法なら撃ちます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus& m_eventBus;    // 魔法の発動を発行するイベントバス
    };
}    // namespace FruitMagic::ECS

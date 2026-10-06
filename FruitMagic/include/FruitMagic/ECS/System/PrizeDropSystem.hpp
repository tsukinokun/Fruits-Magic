//----------------------------------------------------------------------------
//! @file   PrizeDropSystem.hpp
//! @brief  景品が台から落ちたことを判定するシステム
//! @detail 判定ラインより下に落ちた景品を、落ちた位置で「払い出し口」（手前の端から落ちた。幅のどこでも）か
//!         「横の溝」（側壁の無い所から横へ落ちた）に分類し、
//!         PrizeDroppedEvent を発行して破棄予約します。手持ちやマナへの反映は購読側の仕事です。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 景品が台から落ちたことを判定するシステムです。
    class PrizeDropSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus 判定結果を発行するイベントバス
        explicit PrizeDropSystem(Tsukino::ECS::EventBus& eventBus);

        //! 落ちた景品を判定します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus& m_eventBus;    // 判定結果を発行するイベントバス
    };
}    // namespace FruitMagic::ECS

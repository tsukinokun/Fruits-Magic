//----------------------------------------------------------------------------
//! @file   JackpotSystem.hpp
//! @brief  ジャックポット穴（プッシャー上面の、時々開く小さな穴）のシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! ジャックポット穴のシステムです。穴の目印をプッシャーの上面に沿って動かし、開いている間に
    //! 穴の上に来たコインを吸い込みます。吸い込むと「ジャックポット！」として、ルーレットの回数を増やし、
    //! コインを降らせます（JackpotConfig の spins / bonusCoins）。
    class JackpotSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus NoticeEvent を発行するイベントバス
        explicit JackpotSystem(Tsukino::ECS::EventBus& eventBus);

        //! 穴を動かし、コインを吸い込みます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus& m_eventBus;    // お知らせを出すイベントバス
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   CoinShowerSystem.hpp
//! @brief  コインのシャワーの依頼に従ってコインを降らせるシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <random>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! コインのシャワーのシステムです。CoinShowerState の依頼を、依頼の場所（ShowerPlace）のランダムな位置へ
    //! 1枚ずつ落とします（手持ちからは引かない）。手前側なら敷き詰めたコインのすぐ上から、
    //! 投入位置ならプレイヤーが入れるのと同じ高さから落とします。
    //! 台の上のコインが EconomyConfig::maxCoinsOnTable 枚以上の間は、減るまで待ちます。
    class CoinShowerSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus 演出（EffectEvent）を頼むイベントバス
        explicit CoinShowerSystem(Tsukino::ECS::EventBus& eventBus);

        //! 依頼を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus& m_eventBus;    // 演出を頼むイベントバス
        std::mt19937            m_rng;         // 落とす位置の乱数
    };
}    // namespace FruitMagic::ECS

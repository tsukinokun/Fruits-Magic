//----------------------------------------------------------------------------
//! @file   DropFeedbackSystem.hpp
//! @brief  落ちた物が払い出しか溝かをその場で見せるシステム
//! @detail 落ちた所に払い出し（金）・溝（青）の光の粒を出し、横へこぼれた物はそこの縁の光の帯（DropEdgeComponent）を一瞬強く光らせます。
//!         手前の角から落ちた物でも、どちらになったかがすぐ分かるようにします。
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 落ちた物が払い出しか溝かをその場で見せるシステムです。
    class DropFeedbackSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent の購読と EffectEvent の発行に使うイベントバス
        explicit DropFeedbackSystem(Tsukino::ECS::EventBus& eventBus);

        //! 落ちた所に粒を頼み、台の縁の帯を光らせます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus&        m_eventBus;          // EffectEvent を発行するイベントバス
        Tsukino::ECS::ScopedConnection m_dropConnection;    // 購読解除を自動で行う接続
        std::vector<PrizeDroppedEvent> m_pendingDrops;      // 次の Update で処理する落下
    };
}    // namespace FruitMagic::ECS

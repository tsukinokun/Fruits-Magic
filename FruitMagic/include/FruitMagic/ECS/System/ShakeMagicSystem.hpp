//----------------------------------------------------------------------------
//! @file   ShakeMagicSystem.hpp
//! @brief  魔法「ゆらゆら」（台を揺らして縁の物を落とす）の効果を実装するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

#include <random>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法「ゆらゆら」の効果です。id が "shake" の魔法が撃たれると、効果時間のあいだ一定間隔で
    //! 台の上の景品すべてに手前向きの衝撃を与え、カメラを小刻みに揺らします。
    //! 強さ・時間などは Magic.json の params（duration / interval / forward / side / up / cameraShake）で決まります。
    class ShakeMagicSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を購読するイベントバス
        explicit ShakeMagicSystem(Tsukino::ECS::EventBus& eventBus);

        //! 効果を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! 台の上の景品すべてに衝撃を1回与えます。
        //! @param  [in] registry レジストリ
        void Pulse(Tsukino::ECS::Registry& registry);

        //! 効果を終え、カメラを元の位置に戻します。
        //! @param  [in] registry レジストリ
        void Finish(Tsukino::ECS::Registry& registry);

        Tsukino::ECS::ScopedConnection m_castConnection;          // 購読解除を自動で行う接続
        int                            m_pendingCast  = -1;      // 次の Update で始める魔法の添字
        int                            m_activeMagic  = -1;      // 効果中の魔法の添字（-1 は効果中でない）
        float                          m_timer        = 0.0f;    // 効果の残り時間（秒）
        float                          m_pulseTimer   = 0.0f;    // 次の衝撃までの時間（秒）
        float                          m_elapsed      = 0.0f;    // 効果が始まってからの時間（秒）
        Tsukino::ECS::Entity           m_camera{entt::null};     // 揺らしているカメラ
        hlslpp::float3                 m_cameraBase = hlslpp::float3(0.0f, 0.0f, 0.0f);    // 揺らす前のカメラの位置
        std::mt19937                   m_rng;                     // 左右の向きを決める乱数
    };
}    // namespace FruitMagic::ECS

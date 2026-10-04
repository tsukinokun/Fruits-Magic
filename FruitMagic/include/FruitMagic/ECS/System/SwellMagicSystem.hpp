//----------------------------------------------------------------------------
//! @file   SwellMagicSystem.hpp
//! @brief  魔法「ふくらむ」（一定時間プッシャーの押し幅を大きくする）の効果を実装するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法「ふくらむ」の効果です。id が "swell" の魔法が撃たれると、効果時間のあいだ
    //! プッシャーの振幅に params の amplitude（cm）を足します（MagicEffects::pusherAmplitudeBonus）。
    //! 振幅は PusherScene が少しずつ近づけるので、伸びるのも戻るのもなめらかです。
    class SwellMagicSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を購読するイベントバス
        explicit SwellMagicSystem(Tsukino::ECS::EventBus& eventBus);

        //! 効果を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_castConnection;         // 購読解除を自動で行う接続
        int                            m_pendingCast = -1;      // 次の Update で始める魔法の添字
        int                            m_activeMagic = -1;      // 効果中の魔法の添字（-1 は効果中でない）
        float                          m_timer       = 0.0f;    // 効果の残り時間（秒）
    };
}    // namespace FruitMagic::ECS

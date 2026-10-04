//----------------------------------------------------------------------------
//! @file   MeteorMagicSystem.hpp
//! @brief  魔法「メテオコイン」（タダのコインを大量に降らせる）の効果を実装するシステム
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

    //! 魔法「メテオコイン」の効果です。id が "meteor" の魔法が撃たれると、params の count 枚のコインを
    //! duration 秒かけて台の手前側に降らせます（CoinShowerState に依頼し、CoinShowerSystem が降らせる）。
    class MeteorMagicSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を購読するイベントバス
        explicit MeteorMagicSystem(Tsukino::ECS::EventBus& eventBus);

        //! 効果を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_castConnection;         // 購読解除を自動で行う接続
        int                            m_pendingCast = -1;      // 次の Update で始める魔法の添字
        int                            m_activeMagic = -1;      // 降らせている途中の魔法の添字（-1 は効果中でない）
        float                          m_timer       = 0.0f;    // 降らせ終わるまでの残り時間（秒。ボタンの表示用）
    };
}    // namespace FruitMagic::ECS

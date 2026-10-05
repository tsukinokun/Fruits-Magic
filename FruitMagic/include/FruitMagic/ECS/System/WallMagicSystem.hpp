//----------------------------------------------------------------------------
//! @file   WallMagicSystem.hpp
//! @brief  魔法「かべ」（一定時間、溝の手前に斜めの壁を出して取りこぼしを防ぐ）の効果を実装するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

#include <random>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法「かべ」の効果です。id が "wall" の魔法が撃たれると、台の手前端の左右（溝の上）に
    //! 斜めの壁を床からせり上げ、押し出された物を中央の払い出し口へ寄せます（漏斗）。
    //! 効果時間（params の duration）が過ぎると床へ沈めて消します。壁の高さは params の height。
    class WallMagicSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を購読するイベントバス
        explicit WallMagicSystem(Tsukino::ECS::EventBus& eventBus);

        //! 効果を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! 壁を床の下に作ります（Kinematic。Update でせり上げる）。
        //! @param  [in] registry レジストリ
        void CreateWalls(Tsukino::ECS::Registry& registry);

        Tsukino::ECS::EventBus&           m_eventBus;               // 演出（EffectEvent）を頼むイベントバス
        Tsukino::ECS::ScopedConnection    m_castConnection;         // 購読解除を自動で行う接続
        int                               m_pendingCast = -1;      // 次の Update で始める魔法の添字
        int                               m_activeMagic = -1;      // 効果中の魔法の添字（-1 は効果中でない）
        float                             m_timer       = 0.0f;    // 効果の残り時間（秒）
        float                             m_elapsed     = 0.0f;    // 効果が始まってからの時間（秒）
        float                             m_height      = 10.0f;   // 壁の高さ（cm）
        float                             m_riseSeconds = 0.5f;    // 壁がせり上がる（沈む）のにかける時間（秒）
        float                             m_trailInterval = 0.08f; // 壁から粒を出す間隔（秒）
        std::vector<Tsukino::ECS::Entity> m_walls;                 // 出している壁
        std::vector<hlslpp::float4>       m_wallLines;             // 壁の床の線（奥側の x, z, 手前側の x, z）。粒を出す位置に使う
        float                             m_trailTimer  = 0.0f;    // 次に壁から粒を出すまでの時間（秒）
        std::mt19937                      m_rng;                   // 粒を出す位置の乱数
    };
}    // namespace FruitMagic::ECS

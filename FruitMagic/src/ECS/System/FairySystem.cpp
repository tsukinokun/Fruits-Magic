//----------------------------------------------------------------------------
//! @file   FairySystem.cpp
//! @brief  妖精の自動投入のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/FairySystem.hpp>

#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    FairySystem::FairySystem()
        : m_rng(std::random_device{}()) {
    }

    //----------------------------------------------------------------------------
    //! 間隔が来たらコインを入れます。
    //----------------------------------------------------------------------------
    void FairySystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<TableStats>() || !registry.HasContext<PrizeFactory>())
            return;

        const float interval = registry.GetContext<TableStats>().autoLaunchInterval;
        if(interval <= 0.0f) {
            // 強化していない間は時間をためない（買った直後に一気に入らないように）
            m_timer = 0.0f;
            return;
        }

        m_timer += deltaTime;
        if(m_timer < interval)
            return;
        // 重いフレームでも1フレームに1枚まで（まとめて入れると同じ所に重なる）
        m_timer = 0.0f;

        // プレイヤーと同じ投入位置の列（プッシャー上面の上）に入れる
        const float x = std::uniform_real_distribution<float>(-Layout::kLaunchLaneHalfWidth, Layout::kLaunchLaneHalfWidth)(m_rng);
        registry.GetContext<PrizeFactory>().CreateCoin(registry, hlslpp::float3(x, Layout::kLaunchY, Layout::kLaunchZ));
    }
}    // namespace FruitMagic::ECS

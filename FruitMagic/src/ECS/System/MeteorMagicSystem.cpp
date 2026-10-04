//----------------------------------------------------------------------------
//! @file   MeteorMagicSystem.cpp
//! @brief  魔法「メテオコイン」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/MeteorMagicSystem.hpp>

#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief この魔法の id（Magic.json の "id"）
        constexpr const char* kMagicId = "meteor";
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    MeteorMagicSystem::MeteorMagicSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは覚えるだけにして、効果は Update で始める
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) { m_pendingCast = e.magicIndex; });
    }

    //----------------------------------------------------------------------------
    //! 効果を進めます。
    //----------------------------------------------------------------------------
    void MeteorMagicSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MagicCatalog>() || !registry.HasContext<MagicState>() || !registry.HasContext<CoinShowerState>())
            return;

        const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();

        if(m_activeMagic < 0 && catalog.IsMagic(m_pendingCast, kMagicId)) {
            const MagicDef& def = catalog.Magics()[m_pendingCast];
            m_activeMagic       = m_pendingCast;
            m_timer             = def.Param("duration", 2.0f);
            registry.GetContext<CoinShowerState>().Add(static_cast<int>(def.Param("count", 30.0f)), m_timer);
        }
        m_pendingCast = -1;

        if(m_activeMagic < 0)
            return;

        m_timer -= deltaTime;
        registry.GetContext<MagicState>().SetRemaining(m_activeMagic, m_timer);
        if(m_timer <= 0.0f)
            m_activeMagic = -1;
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   SwellMagicSystem.cpp
//! @brief  魔法「ふくらむ」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/SwellMagicSystem.hpp>

#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicEffects.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/TableStats.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief この魔法の id（Magic.json の "id"）
        constexpr const char* kMagicId = "swell";
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    SwellMagicSystem::SwellMagicSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは覚えるだけにして、効果は Update で始める
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) { m_pendingCast = e.magicIndex; });
    }

    //----------------------------------------------------------------------------
    //! 効果を進めます。
    //----------------------------------------------------------------------------
    void SwellMagicSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MagicCatalog>() || !registry.HasContext<MagicState>() || !registry.HasContext<MagicEffects>())
            return;

        const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();
        MagicEffects&       effects = registry.GetContext<MagicEffects>();

        if(m_activeMagic < 0 && catalog.IsMagic(m_pendingCast, kMagicId)) {
            m_activeMagic = m_pendingCast;
            m_timer       = catalog.Magics()[m_activeMagic].Param("duration", 8.0f) * (registry.HasContext<TableStats>() ? registry.GetContext<TableStats>().magicDurationMultiplier : 1.0f);    // 強化「魔法の効果時間」で延びる
        }
        m_pendingCast = -1;

        if(m_activeMagic < 0)
            return;

        m_timer -= deltaTime;
        registry.GetContext<MagicState>().SetRemaining(m_activeMagic, m_timer);
        if(m_timer > 0.0f) {
            effects.pusherAmplitudeBonus = catalog.Magics()[m_activeMagic].Param("amplitude", 6.0f);
        } else {
            effects.pusherAmplitudeBonus = 0.0f;
            m_activeMagic                = -1;
        }
    }
}    // namespace FruitMagic::ECS

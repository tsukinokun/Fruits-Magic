//----------------------------------------------------------------------------
//! @file   SaveSystem.cpp
//! @brief  自動セーブのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/SaveSystem.hpp>

#include <FruitMagic/Game/OfflineReward.hpp>
#include <FruitMagic/Game/SaveData.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //----------------------------------------------------------------------------
    //! 間隔が来たら保存します。
    //----------------------------------------------------------------------------
    void SaveSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<OfflineConfig>())
            return;

        const OfflineConfig& config = registry.GetContext<OfflineConfig>();
        if(config.savePath.empty())
            return;

        m_timer += deltaTime;
        if(m_timer < config.autosaveSeconds)
            return;
        m_timer = 0.0f;

        SaveData::Save(registry, config.savePath);
    }
}    // namespace FruitMagic::ECS

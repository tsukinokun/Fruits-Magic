//----------------------------------------------------------------------------
//! @file   WallMagicSystem.cpp
//! @brief  魔法「かべ」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/WallMagicSystem.hpp>

#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>

#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief この魔法の id（Magic.json の "id"）
        constexpr const char* kMagicId = "wall";

        //! @brief 壁がせり上がる（沈む）のにかける時間（秒）。一瞬で出すと重なった景品を弾き飛ばす
        constexpr float kRiseSeconds = 0.5f;

        //! @brief 壁の厚みの半分（cm）
        constexpr float kHalfThickness = 0.6f;

        //! @brief 壁の奥側の端が、台の手前端からどれだけ奥にあるか（cm）。漏斗の傾きが決まる
        constexpr float kFunnelDepth = 14.0f;

        //! @brief 見た目の色（水色の半透明）
        const hlslpp::float3 kWallColor = hlslpp::float3(0.4f, 0.85f, 1.0f);

        //--------------------------------------------------------------
        //! 壁の中心の高さを、効果の経過に合わせて返します（せり上がり → そのまま → 沈む）。
        //! @param  [in] elapsed   効果が始まってからの時間（秒）
        //! @param  [in] remaining 効果の残り時間（秒）
        //! @param  [in] height    壁の高さ（cm）
        //! @return 中心の高さ
        //--------------------------------------------------------------
        float WallCenterY(float elapsed, float remaining, float height) {
            const float t = std::clamp(std::min(elapsed, remaining) / kRiseSeconds, 0.0f, 1.0f);
            // 床の下（上端が床面）から、下端が床面に来るまで
            return -height * 0.5f + height * t;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    WallMagicSystem::WallMagicSystem(Tsukino::ECS::EventBus& eventBus) {
        // ハンドラでは覚えるだけにして、効果は Update で始める
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) { m_pendingCast = e.magicIndex; });
    }

    //----------------------------------------------------------------------------
    //! 効果を進めます。
    //----------------------------------------------------------------------------
    void WallMagicSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<MagicCatalog>() || !registry.HasContext<MagicState>() || !registry.HasContext<PrizeFactory>())
            return;

        const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();

        if(m_activeMagic < 0 && catalog.IsMagic(m_pendingCast, kMagicId)) {
            const MagicDef& def = catalog.Magics()[m_pendingCast];
            m_activeMagic       = m_pendingCast;
            m_timer             = def.Param("duration", 10.0f);
            m_elapsed           = 0.0f;
            m_height            = std::max(1.0f, def.Param("height", 10.0f));
            CreateWalls(registry);
        }
        m_pendingCast = -1;

        if(m_activeMagic < 0)
            return;

        m_timer -= deltaTime;
        m_elapsed += deltaTime;
        registry.GetContext<MagicState>().SetRemaining(m_activeMagic, m_timer);

        //--------------------------------------------------------------
        // 壁の高さを動かす（Kinematic なので、物理が位置の差分から速度を求めて景品を押し上げる）
        //--------------------------------------------------------------
        const float y = WallCenterY(m_elapsed, std::max(0.0f, m_timer), m_height);
        for(Tsukino::ECS::Entity wall : m_walls) {
            if(auto* transform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(wall)) {
                transform->position = hlslpp::float3(float(transform->position.x), y, float(transform->position.z));
                transform->dirty    = true;
            }
        }

        if(m_timer <= 0.0f) {
            for(Tsukino::ECS::Entity wall : m_walls) {
                if(registry.IsValid(wall))
                    registry.QueueDestroy(wall);
            }
            m_walls.clear();
            m_activeMagic = -1;
        }
    }

    //----------------------------------------------------------------------------
    //! 壁を床の下に作ります。
    //----------------------------------------------------------------------------
    void WallMagicSystem::CreateWalls(Tsukino::ECS::Registry& registry) {
        PrizeFactory& factory = registry.GetContext<PrizeFactory>();

        for(float side : {-1.0f, 1.0f}) {
            //--------------------------------------------------------------
            // 側壁の奥側から、払い出し口の端（の少し内側）の手前端まで斜めに渡す
            //--------------------------------------------------------------
            const float backX  = side * Layout::kFieldHalfWidth;
            const float backZ  = Layout::kFieldFrontZ - kFunnelDepth;
            const float frontX = side * (Layout::kPayoutHalfWidth - 1.0f);
            const float frontZ = Layout::kFieldFrontZ + 0.5f;

            const float dx     = frontX - backX;
            const float dz     = frontZ - backZ;
            const float length = std::sqrt(dx * dx + dz * dz);

            const hlslpp::float3 center((backX + frontX) * 0.5f, WallCenterY(0.0f, 1.0f, m_height), (backZ + frontZ) * 0.5f);
            const hlslpp::float3 halfExtent(length * 0.5f, m_height * 0.5f, kHalfThickness);

            Tsukino::ECS::Entity wall = factory.CreateBox(registry, center, halfExtent, Tsukino::BuiltIn::ECS::RigidbodyType::Kinematic);

            // Y 軸まわりに回して、箱の長い辺（ローカル X）を (dx, dz) の向きに合わせる
            const float angle = std::atan2(-dz, dx);
            auto&       t     = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(wall);
            t.rotation        = hlslpp::quaternion(0.0f, std::sin(angle * 0.5f), 0.0f, std::cos(angle * 0.5f));
            t.dirty           = true;

            // 魔法の壁らしく、水色の半透明で光らせる
            registry.GetComponent<Tsukino::BuiltIn::ECS::ModelComponent>(wall).opacity = 0.5f;
            Tsukino::BuiltIn::ECS::RimGlowComponent& glow = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(wall);
            glow.active                                   = true;
            glow.rimColor                                 = kWallColor;
            glow.rimIntensity                             = 1.5f;
            glow.glow                                     = 0.6f;

            m_walls.push_back(wall);
        }
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   WallMagicSystem.cpp
//! @brief  魔法「かべ」の効果の実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/WallMagicSystem.hpp>

#include <FruitMagic/ECS/Event/EffectEvent.hpp>
#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicState.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/StageConfig.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

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

        //--------------------------------------------------------------
        //! 壁の中心の高さを、効果の経過に合わせて返します（せり上がり → そのまま → 沈む）。
        //! @param  [in] elapsed   効果が始まってからの時間（秒）
        //! @param  [in] remaining 効果の残り時間（秒）
        //! @param  [in] height    壁の高さ（cm）
        //! @param  [in] rise      せり上がる（沈む）のにかける時間（秒）
        //! @return 中心の高さ
        //--------------------------------------------------------------
        float WallCenterY(float elapsed, float remaining, float height, float rise) {
            const float t = std::clamp(std::min(elapsed, remaining) / std::max(rise, 0.01f), 0.0f, 1.0f);
            // 床の下（上端が床面）から、下端が床面に来るまで
            return -height * 0.5f + height * t;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    WallMagicSystem::WallMagicSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus)
        , m_rng(std::random_device{}()) {
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
            m_height            = std::max(def.Param("minHeight", 1.0f), def.Param("height", 10.0f));
            m_riseSeconds       = def.Param("riseSeconds", 0.5f);       // 一瞬で出すと重なった景品を弾き飛ばす
            m_trailInterval     = def.Param("trailInterval", 0.08f);
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
        const float y = WallCenterY(m_elapsed, std::max(0.0f, m_timer), m_height, m_riseSeconds);
        for(Tsukino::ECS::Entity wall : m_walls) {
            if(auto* transform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(wall)) {
                transform->position = hlslpp::float3(float(transform->position.x), y, float(transform->position.z));
                transform->dirty    = true;
            }
        }

        //--------------------------------------------------------------
        // 壁が立っている間は、壁のどこかから水色の粒を立ちのぼらせる
        //--------------------------------------------------------------
        m_trailTimer -= deltaTime;
        if(m_trailTimer <= 0.0f && !m_wallLines.empty() && y > -m_height * 0.25f) {
            m_trailTimer = m_trailInterval;
            std::uniform_real_distribution<float> unit(0.0f, 1.0f);
            const hlslpp::float4&                 line = m_wallLines[static_cast<size_t>(unit(m_rng) * static_cast<float>(m_wallLines.size())) % m_wallLines.size()];
            const float                           t    = unit(m_rng);
            m_eventBus.Publish(EffectEvent{"wallTrail", hlslpp::float3(float(line.x + (line.z - line.x) * t), y + m_height * 0.5f * unit(m_rng),
                                                                        float(line.y + (line.w - line.y) * t))});
        }

        if(m_timer <= 0.0f) {
            m_wallLines.clear();
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
        PrizeFactory&      factory = registry.GetContext<PrizeFactory>();
        const TableLayout& layout  = GetTableLayout(registry);
        const StageConfig& stage   = GetStageConfig(registry);
        const MagicDef&    def     = registry.GetContext<MagicCatalog>().Magics()[m_activeMagic];
        const float        frontOverhang = def.Param("frontOverhang", 0.5f);    // 壁の手前側の端を台の手前端からどれだけ外へ出すか
        const float        halfThickness = def.Param("halfThickness", 0.6f);    // 壁の厚みの半分

        //--------------------------------------------------------------
        // (startX, startZ) から (endX, endZ) まで、床の下に壁を1枚作る
        //--------------------------------------------------------------
        auto addWall = [&](float startX, float startZ, float endX, float endZ) {
            const float dx     = endX - startX;
            const float dz     = endZ - startZ;
            const float length = std::sqrt(dx * dx + dz * dz);

            const hlslpp::float3 center((startX + endX) * 0.5f, WallCenterY(0.0f, 1.0f, m_height, m_riseSeconds), (startZ + endZ) * 0.5f);
            const hlslpp::float3 halfExtent(length * 0.5f, m_height * 0.5f, halfThickness);

            Tsukino::ECS::Entity wall = factory.CreateBox(registry, center, halfExtent, Tsukino::BuiltIn::ECS::RigidbodyType::Kinematic);

            // Y 軸まわりに回して、箱の長い辺（ローカル X）を (dx, dz) の向きに合わせる
            const float angle = std::atan2(-dz, dx);
            auto&       t     = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(wall);
            t.rotation        = hlslpp::quaternion(0.0f, std::sin(angle * 0.5f), 0.0f, std::cos(angle * 0.5f));
            t.dirty           = true;

            // 魔法の壁らしく、水色の半透明で光らせる
            registry.GetComponent<Tsukino::BuiltIn::ECS::ModelComponent>(wall).opacity = stage.wallOpacity;
            Tsukino::BuiltIn::ECS::RimGlowComponent& glow = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(wall);
            glow.active                                   = true;
            glow.rimColor                                 = stage.wallColor;
            glow.rimIntensity                             = stage.wallRimIntensity;
            glow.glow                                     = stage.wallGlow;

            m_walls.push_back(wall);
            m_wallLines.push_back(hlslpp::float4(startX, startZ, endX, endZ));
        };

        // 左右の開いた所（側壁の手前端から台の手前端の少し外まで）に沿って立てる。
        // 台の端のすぐ外に立て、端に載っている景品を下から突き上げないようにする
        for(float side : {-1.0f, 1.0f}) {
            const float edgeX = side * (layout.fieldHalfWidth + halfThickness);
            addWall(edgeX, layout.SideWallFrontZ(), edgeX, layout.fieldFrontZ + frontOverhang);
        }
    }
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   RouletteSystem.cpp
//! @brief  ルーレットのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/RouletteSystem.hpp>

#include <FruitMagic/ECS/Event/CheckerEnteredEvent.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 回転中に表示を切り替える間隔（秒）
        constexpr float kFlipInterval = 0.08f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    RouletteSystem::RouletteSystem(Tsukino::ECS::EventBus& eventBus)
        : m_rng(std::random_device{}()) {
        // ハンドラでは数えるだけにして、レジストリへの反映は Update で行う
        m_enteredConnection = eventBus.Subscribe<CheckerEnteredEvent>([this](const CheckerEnteredEvent&) { ++m_pendingEntered; });
    }

    //----------------------------------------------------------------------------
    //! ルーレットを進めます。
    //----------------------------------------------------------------------------
    void RouletteSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<RouletteState>() || !registry.HasContext<FruitCatalog>() || !registry.HasContext<GameState>())
            return;

        RouletteState&       state   = registry.GetContext<RouletteState>();
        const FruitCatalog&  catalog = registry.GetContext<FruitCatalog>();
        const int            level   = registry.GetContext<GameState>().treeLevel;
        const RouletteConfig config  = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};

        //--------------------------------------------------------------
        // チェッカーに入った分をためる（上限を超えた分は捨てる）
        //--------------------------------------------------------------
        state.stock      = std::min(state.stock + m_pendingEntered, config.maxStock);
        m_pendingEntered = 0;

        switch(state.phase) {
            case RoulettePhase::Idle:
                if(state.stock <= 0)
                    break;

                //--------------------------------------------------------------
                // 回転開始。結果は最初に抽選しておき、回転は見せるだけにする
                //--------------------------------------------------------------
                state.stock -= 1;
                state.phase = RoulettePhase::Spinning;
                m_timer     = config.spinSeconds;
                m_flipTimer = 0.0f;

                m_resultFruit = -1;
                if(std::uniform_real_distribution<float>(0.0f, 1.0f)(m_rng) < config.hitChance) {
                    m_resultFruit = catalog.PickSpawnable(level, m_rng);
                }
                break;

            case RoulettePhase::Spinning:
                //--------------------------------------------------------------
                // 回転中は、出現できる果物とハズレを交互に見せる
                //--------------------------------------------------------------
                m_flipTimer -= deltaTime;
                if(m_flipTimer <= 0.0f) {
                    m_flipTimer        = kFlipInterval;
                    state.displayFruit = (state.displayFruit >= 0) ? -1 : catalog.PickSpawnable(level, m_rng);
                }

                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    state.phase        = RoulettePhase::Result;
                    state.displayFruit = m_resultFruit;
                    state.resultHit    = (m_resultFruit >= 0);
                    m_timer            = config.resultSeconds;

                    if(state.resultHit) {
                        SpawnFruit(registry, m_resultFruit);
                    }
                }
                break;

            case RoulettePhase::Result:
                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    state.phase        = RoulettePhase::Idle;
                    state.displayFruit = -1;
                    state.resultHit    = false;
                }
                break;
        }
    }

    //----------------------------------------------------------------------------
    //! 当たった果物を台に補充します。
    //----------------------------------------------------------------------------
    void RouletteSystem::SpawnFruit(Tsukino::ECS::Registry& registry, int fruitIndex) {
        if(!registry.HasContext<PrizeFactory>())
            return;

        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        if(fruitIndex < 0 || fruitIndex >= static_cast<int>(catalog.Fruits().size()))
            return;

        const FruitDef& def = catalog.Fruits()[fruitIndex];

        //--------------------------------------------------------------
        // プッシャー上面の投入列に、上面のすぐ上から置く
        // （高い所から落とすと、下の物を押し込んでめり込ませるため）
        //--------------------------------------------------------------
        const float halfWidth = (def.shape == FruitShape::Box) ? float(def.halfExtent.x) : def.radius;
        const float halfDepth = (def.shape == FruitShape::Box) ? float(def.halfExtent.z) : def.radius;
        const float range     = std::max(0.0f, Layout::kLaunchLaneHalfWidth - halfWidth - 1.0f);
        const float x         = std::uniform_real_distribution<float>(-range, range)(m_rng);
        const float y         = Layout::kPusherTopY + def.HalfHeightOfBounds() + 0.5f;

        // 大きな果物は背面パネルに重ならないよう、その分だけ手前に置く
        const float z = std::max(Layout::kLaunchZ, Layout::kBackPanelFrontZ + halfDepth + 0.5f);

        registry.GetContext<PrizeFactory>().CreateFruit(registry, def, fruitIndex, hlslpp::float3(x, y, z));
    }
}    // namespace FruitMagic::ECS

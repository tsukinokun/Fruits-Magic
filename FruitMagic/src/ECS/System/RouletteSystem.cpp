//----------------------------------------------------------------------------
//! @file   RouletteSystem.cpp
//! @brief  ルーレットのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/RouletteSystem.hpp>

#include <FruitMagic/ECS/Event/CheckerEnteredEvent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/JackpotConfig.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/PusherLayout.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 回転中に表示を切り替える間隔（秒）
        constexpr float kFlipInterval = 0.08f;

        //! @brief ジャックポットチャンスの抽選中に「JACKPOT」「ハズレ」を切り替える間隔（秒）。最後は少しゆっくりに
        constexpr float kJackpotFlipInterval = 0.12f;

        //! @brief ジャックポットのコインを降らせる時間（秒）
        constexpr float kJackpotShowerSeconds = 3.0f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    RouletteSystem::RouletteSystem(Tsukino::ECS::EventBus& eventBus)
        : m_eventBus(eventBus)
        , m_rng(std::random_device{}()) {
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
        // チェッカーに入った分をためる（上限を超えた分は捨てる）。
        // ジャックポットで上限を超えてたまっている分は減らさない
        //--------------------------------------------------------------
        state.stock      = std::max(state.stock, std::min(state.stock + m_pendingEntered, config.maxStock));
        m_pendingEntered = 0;

        switch(state.phase) {
            case RoulettePhase::Idle:
                if(state.stock <= 0)
                    break;

                state.stock -= 1;

                //--------------------------------------------------------------
                // まれにジャックポットチャンス。結果は最初に抽選しておき、抽選の様子は見せるだけにする
                //--------------------------------------------------------------
                if(registry.HasContext<JackpotConfig>()) {
                    const JackpotConfig&                  jackpot = registry.GetContext<JackpotConfig>();
                    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
                    if(roll(m_rng) < jackpot.chanceRate) {
                        state.phase          = RoulettePhase::JackpotSpin;
                        state.jackpotDisplay = false;
                        m_jackpotWin         = roll(m_rng) < jackpot.winRate;
                        m_timer              = jackpot.spinSeconds;
                        m_flipTimer          = 0.0f;
                        if(registry.HasContext<PlayStats>())
                            registry.GetContext<PlayStats>().jackpotChances += 1;
                        m_eventBus.Publish(NoticeEvent{L"ジャックポットチャンス！", jackpot.spinSeconds});
                        break;
                    }
                }

                //--------------------------------------------------------------
                // 回転開始。結果は最初に抽選しておき、回転は見せるだけにする
                //--------------------------------------------------------------
                state.phase = RoulettePhase::Spinning;
                m_timer     = config.spinSeconds;
                m_flipTimer = 0.0f;

                m_resultFruit   = -1;
                m_resultVariant = 0;
                if(std::uniform_real_distribution<float>(0.0f, 1.0f)(m_rng) < config.hitChance) {
                    m_resultFruit = catalog.PickSpawnable(level, m_rng);
                    if(registry.HasContext<CollectionConfig>())
                        m_resultVariant = registry.GetContext<CollectionConfig>().PickVariant(m_rng);
                }
                break;

            case RoulettePhase::Spinning:
                //--------------------------------------------------------------
                // 回転中は、出現できる果物とハズレを交互に見せる
                //--------------------------------------------------------------
                m_flipTimer -= deltaTime;
                if(m_flipTimer <= 0.0f) {
                    m_flipTimer          = kFlipInterval;
                    state.displayFruit   = (state.displayFruit >= 0) ? -1 : catalog.PickSpawnable(level, m_rng);
                    state.displayVariant = 0;
                }

                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    state.phase        = RoulettePhase::Result;
                    state.displayFruit   = m_resultFruit;
                    state.displayVariant = m_resultVariant;
                    state.resultHit      = (m_resultFruit >= 0);
                    m_timer            = config.resultSeconds;

                    if(state.resultHit) {
                        SpawnFruit(registry, m_resultFruit, m_resultVariant);
                    }
                    if(registry.HasContext<PlayStats>()) {
                        PlayStats& stats = registry.GetContext<PlayStats>();
                        stats.rouletteSpins += 1;
                        stats.rouletteHits += state.resultHit ? 1 : 0;
                    }
                }
                break;

            case RoulettePhase::Result:
            case RoulettePhase::JackpotResult:
                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    state.phase        = RoulettePhase::Idle;
                    state.displayFruit = -1;
                    state.resultHit    = false;
                    state.jackpotWin   = false;
                }
                break;

            case RoulettePhase::JackpotSpin:
                //--------------------------------------------------------------
                // 「JACKPOT」と「ハズレ」を交互に見せ、時間が来たら抽選済みの結果で止める
                //--------------------------------------------------------------
                m_flipTimer -= deltaTime;
                if(m_flipTimer <= 0.0f) {
                    m_flipTimer          = kJackpotFlipInterval;
                    state.jackpotDisplay = !state.jackpotDisplay;
                }

                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    const JackpotConfig jackpot = registry.HasContext<JackpotConfig>() ? registry.GetContext<JackpotConfig>() : JackpotConfig{};
                    state.phase                 = RoulettePhase::JackpotResult;
                    state.jackpotWin            = m_jackpotWin;
                    state.jackpotDisplay        = m_jackpotWin;
                    m_timer                     = jackpot.resultSeconds;
                    GrantJackpot(registry, m_jackpotWin);
                }
                break;
        }
    }

    //----------------------------------------------------------------------------
    //! 当たった果物を台に補充します。
    //----------------------------------------------------------------------------
    void RouletteSystem::SpawnFruit(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex) {
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

        // バリエーションの色と光り方（設定が無ければ通常の見た目）
        hlslpp::float3 color = def.color;
        float          glow  = 0.6f;
        if(registry.HasContext<CollectionConfig>()) {
            const auto& variants = registry.GetContext<CollectionConfig>().Variants();
            if(variantIndex >= 0 && variantIndex < static_cast<int>(variants.size())) {
                color = variants[variantIndex].ColorOf(def);
                glow  = variants[variantIndex].glow;
            }
        }

        registry.GetContext<PrizeFactory>().CreateFruit(registry, def, fruitIndex, variantIndex, color, glow, hlslpp::float3(x, y, z));
    }

    //----------------------------------------------------------------------------
    //! ジャックポットチャンスの結果の景品を出します。
    //----------------------------------------------------------------------------
    void RouletteSystem::GrantJackpot(Tsukino::ECS::Registry& registry, bool win) {
        const JackpotConfig jackpot = registry.HasContext<JackpotConfig>() ? registry.GetContext<JackpotConfig>() : JackpotConfig{};
        const int           coins   = win ? jackpot.bonusCoins : jackpot.consolationCoins;

        // コインは台の手前側に降らせる（タダ。手持ちからは引かない）
        if(registry.HasContext<CoinShowerState>())
            registry.GetContext<CoinShowerState>().Add(coins, win ? kJackpotShowerSeconds : 1.0f);

        if(!win) {
            m_eventBus.Publish(NoticeEvent{L"おしい…  コイン +" + std::to_wstring(coins), jackpot.resultSeconds});
            return;
        }

        // 当たり: ルーレットの回転も増やす（ためておける上限を超えてよい）
        if(registry.HasContext<RouletteState>())
            registry.GetContext<RouletteState>().stock += jackpot.spins;
        if(registry.HasContext<PlayStats>())
            registry.GetContext<PlayStats>().jackpots += 1;
        m_eventBus.Publish(NoticeEvent{L"ジャックポット！！  コイン +" + std::to_wstring(coins) + L"  ルーレット +" + std::to_wstring(jackpot.spins), 4.0f});
    }
}    // namespace FruitMagic::ECS

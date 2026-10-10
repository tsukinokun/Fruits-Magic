//----------------------------------------------------------------------------
//! @file   RouletteSystem.cpp
//! @brief  ルーレットのシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/RouletteSystem.hpp>

#include <FruitMagic/ECS/Event/CheckerEnteredEvent.hpp>
#include <FruitMagic/ECS/Event/EffectEvent.hpp>
#include <FruitMagic/ECS/Event/NoticeEvent.hpp>
#include <FruitMagic/Game/CoinShowerState.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/JackpotConfig.hpp>
#include <FruitMagic/Game/PlayStats.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/TableLayout.hpp>
#include <FruitMagic/Game/TableStats.hpp>
#include <FruitMagic/Game/Texts.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <algorithm>
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
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
        RouletteConfig       config  = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};
        const JackpotConfig  jackpot = registry.HasContext<JackpotConfig>() ? registry.GetContext<JackpotConfig>() : JackpotConfig{};

        // ためられる数・果物の当たり率・コイン当たりの枚数・色違いの出やすさは強化で変わる（TableStats）
        float variantChance = 1.0f;
        if(registry.HasContext<TableStats>()) {
            const TableStats& stats = registry.GetContext<TableStats>();
            config.maxStock         = stats.rouletteMaxStock;
            config.hitChance        = stats.rouletteHitChance;
            config.coinAmount       = stats.rouletteCoinAmount;
            variantChance           = stats.variantChanceMultiplier;
        }

        //--------------------------------------------------------------
        // チェッカーに入った分をためる（上限を超えた分は捨てる）。
        // ジャックポットで上限を超えてたまっている分は減らさない
        //--------------------------------------------------------------
        state.stock      = std::max(state.stock, std::min(state.stock + m_pendingEntered, config.maxStock));
        m_pendingEntered = 0;

        //--------------------------------------------------------------
        // 当たった果物がスロットから台へ飛んでいる。着いたら本物を台に出す（段階とは別に進む）
        //--------------------------------------------------------------
        if(state.flyFruit >= 0) {
            state.flyTime += deltaTime;
            if(state.flyTime >= config.flySeconds) {
                SpawnFruit(registry, state.flyFruit, state.flyVariant, state.flyTarget);
                state.flyFruit = -1;
            }
        }

        // 止まった列の数を数える（リールの表示と効果音が見る）
        auto countStopped = [&]() {
            int stopped = 0;
            for(float stopTime : state.reelStopTimes)
                stopped += (state.spinTime >= stopTime) ? 1 : 0;
            return stopped;
        };

        switch(state.phase) {
            case RoulettePhase::Idle: {
                if(state.stock <= 0)
                    break;

                state.stock -= 1;
                state.spinId += 1;
                state.spinTime     = 0.0f;
                state.reelsStopped = 0;
                state.resultHit    = false;
                state.resultCoins  = 0;
                state.jackpotWin   = false;

                //--------------------------------------------------------------
                // まれにジャックポットチャンス。結果は最初に抽選しておき、大きなスロットはそれに合わせて止める。
                // いつも2列目まで当たりの絵柄をそろえてリーチにし、3列目で決まる
                //--------------------------------------------------------------
                if(registry.HasContext<JackpotConfig>()) {
                    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
                    if(roll(m_rng) < jackpot.chanceRate) {
                        const int symbol    = catalog.FindIndex(jackpot.symbolFruit);
                        const int winSymbol = (symbol >= 0) ? symbol : kCoinSymbol;
                        const int loseSymbol = (winSymbol == kCoinSymbol) ? 0 : kCoinSymbol;
                        m_jackpotWin         = roll(m_rng) < jackpot.winRate;

                        state.phase         = RoulettePhase::JackpotSpin;
                        state.jackpotReels  = true;
                        state.reelVariant   = 0;
                        state.reelSymbols   = {winSymbol, winSymbol, m_jackpotWin ? winSymbol : loseSymbol};
                        state.reelStopTimes = jackpot.reelStopSeconds;
                        state.reach         = true;
                        if(registry.HasContext<PlayStats>())
                            registry.GetContext<PlayStats>().jackpotChances += 1;
                        m_eventBus.Publish(NoticeEvent{GetTexts(registry).Get("notice.jackpotChance"), jackpot.reelStopSeconds[kReelCount - 1]});
                        m_eventBus.Publish(EffectEvent{"jackpotChance", jackpot.chanceEffectPosition});
                        break;
                    }
                }

                //--------------------------------------------------------------
                // 回転開始。結果は最初に抽選しておき、リールはそれに合わせて止める
                //--------------------------------------------------------------
                state.phase        = RoulettePhase::Spinning;
                state.jackpotReels = false;

                m_resultFruit   = -1;
                m_resultVariant = 0;
                m_resultCoins   = 0;
                if(std::uniform_real_distribution<float>(0.0f, 1.0f)(m_rng) < config.hitChance) {
                    m_resultFruit = catalog.PickSpawnable(level, m_rng);
                    if(registry.HasContext<CollectionConfig>())
                        m_resultVariant = registry.GetContext<CollectionConfig>().PickVariant(m_rng, variantChance);
                } else if(std::uniform_real_distribution<float>(0.0f, 1.0f)(m_rng) < config.coinChance) {
                    // 果物が外れても、時々コインが当たる（手持ちが尽きにくいように）
                    m_resultCoins = config.coinAmount;
                }

                // 各列に止める絵柄（当たりはそろえ、ハズレはそろわない並び）
                if(m_resultFruit >= 0) {
                    state.reelSymbols = {m_resultFruit, m_resultFruit, m_resultFruit};
                } else if(m_resultCoins > 0) {
                    state.reelSymbols = {kCoinSymbol, kCoinSymbol, kCoinSymbol};
                } else {
                    std::vector<int> symbols = catalog.SpawnableFruits(level);
                    symbols.push_back(kCoinSymbol);
                    state.reelSymbols = PickMissSymbols(symbols, config.reachMissChance);
                }

                state.reelVariant = (m_resultFruit >= 0) ? m_resultVariant : 0;

                // 1・2列目がそろったらリーチ。3列目を長く回す
                state.reelStopTimes = config.reelStopSeconds;
                state.reach         = state.reelSymbols[0] == state.reelSymbols[1];
                if(state.reach)
                    state.reelStopTimes[kReelCount - 1] += config.reachExtraSeconds;
                break;
            }

            case RoulettePhase::Spinning:
                state.spinTime += deltaTime;
                state.reelsStopped = countStopped();
                if(state.reelsStopped < kReelCount)
                    break;

                //--------------------------------------------------------------
                // 全部止まった。結果を出す（当たりの果物は、スロットから台へ飛び終わってから台に出す）
                //--------------------------------------------------------------
                state.phase          = RoulettePhase::Result;
                state.displayFruit   = m_resultFruit;
                state.displayVariant = m_resultVariant;
                state.resultHit      = (m_resultFruit >= 0);
                state.resultCoins    = state.resultHit ? 0 : m_resultCoins;
                m_timer              = config.resultSeconds;

                if(state.resultHit) {
                    // 前の果物がまだ飛んでいたら、先に出してしまう（重なった分を落とさない）
                    if(state.flyFruit >= 0)
                        SpawnFruit(registry, state.flyFruit, state.flyVariant, state.flyTarget);
                    state.flyFruit   = m_resultFruit;
                    state.flyVariant = m_resultVariant;
                    state.flyTime    = 0.0f;
                    state.flyTarget  = SpawnPosition(registry, m_resultFruit);
                }
                // コインは投入位置の列のどこかに落とす（手持ちには入れない）
                if(state.resultCoins > 0 && registry.HasContext<CoinShowerState>())
                    registry.GetContext<CoinShowerState>().Add(state.resultCoins, config.coinShowerSeconds, ShowerPlace::Launch);
                if(registry.HasContext<PlayStats>()) {
                    PlayStats& stats = registry.GetContext<PlayStats>();
                    stats.rouletteSpins += 1;
                    stats.rouletteHits += state.resultHit ? 1 : 0;
                }
                break;

            case RoulettePhase::Result:
            case RoulettePhase::JackpotResult:
                state.spinTime += deltaTime;
                m_timer -= deltaTime;
                if(m_timer <= 0.0f) {
                    state.phase        = RoulettePhase::Idle;
                    state.displayFruit = -1;
                    state.resultHit    = false;
                    state.resultCoins  = 0;
                    state.jackpotWin   = false;
                    state.jackpotReels = false;
                    state.reach        = false;
                }
                break;

            case RoulettePhase::JackpotSpin:
                state.spinTime += deltaTime;
                state.reelsStopped = countStopped();
                if(state.reelsStopped < kReelCount)
                    break;

                //--------------------------------------------------------------
                // 大きなスロットが止まった。抽選済みの結果で景品を出す
                //--------------------------------------------------------------
                state.phase      = RoulettePhase::JackpotResult;
                state.jackpotWin = m_jackpotWin;
                m_timer          = jackpot.resultSeconds;
                GrantJackpot(registry, m_jackpotWin);
                break;
        }
    }

    //----------------------------------------------------------------------------
    //! ハズレの並び（3つがそろわない）を決めます。
    //----------------------------------------------------------------------------
    std::array<int, kReelCount> RouletteSystem::PickMissSymbols(const std::vector<int>& symbols, float reachMissChance) {
        std::array<int, kReelCount> result = {kCoinSymbol, kCoinSymbol, kCoinSymbol};
        if(symbols.size() < 2)
            return result;    // 絵柄が1種類しかなければ、そろわない並びは作れない（来ない想定）

        std::uniform_int_distribution<size_t> pick(0, symbols.size() - 1);
        auto pickOther = [&](int except) {
            int symbol = except;
            while(symbol == except)
                symbol = symbols[pick(m_rng)];
            return symbol;
        };

        const int first = symbols[pick(m_rng)];
        if(std::uniform_real_distribution<float>(0.0f, 1.0f)(m_rng) < reachMissChance) {
            // ハズレのリーチ: 2列そろえて、3列目だけ外す
            result = {first, first, pickOther(first)};
        } else {
            // 2列目から外す（3列目は何でもよい）
            result = {first, pickOther(first), symbols[pick(m_rng)]};
        }
        return result;
    }

    //----------------------------------------------------------------------------
    //! 当たった果物を台に置く位置を決めます。
    //----------------------------------------------------------------------------
    hlslpp::float3 RouletteSystem::SpawnPosition(Tsukino::ECS::Registry& registry, int fruitIndex) {
        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        const TableLayout&  layout  = GetTableLayout(registry);
        if(fruitIndex < 0 || fruitIndex >= static_cast<int>(catalog.Fruits().size()))
            return hlslpp::float3(0.0f, layout.PusherTopY(), layout.LaunchZ());

        const FruitDef&      def    = catalog.Fruits()[fruitIndex];
        const RouletteConfig config = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};

        //--------------------------------------------------------------
        // プッシャー上面の投入列に、上面のすぐ上から置く
        // （高い所から落とすと、下の物を押し込んでめり込ませるため）
        //--------------------------------------------------------------
        const float halfWidth = (def.shape == FruitShape::Box) ? float(def.halfExtent.x) : def.radius;
        const float halfDepth = (def.shape == FruitShape::Box) ? float(def.halfExtent.z) : def.radius;
        // 端に置くと押し出されて左右の溝へ落ちやすい（計測では収穫より溝落ちが多かった）ので、払い出し口の幅に収める
        const float range     = std::max(0.0f, layout.payoutHalfWidth - config.fruitSpawnMargin - halfWidth);
        const float x         = std::uniform_real_distribution<float>(-range, range)(m_rng);
        const float y         = layout.PusherTopY() + def.HalfHeightOfBounds() + config.fruitSpawnLift;

        // 大きな果物は背面パネルに重ならないよう、その分だけ手前に置く
        const float z = std::max(layout.LaunchZ(), layout.BackPanelFrontZ() + halfDepth + config.fruitSpawnBackMargin);
        return hlslpp::float3(x, y, z);
    }

    //----------------------------------------------------------------------------
    //! 当たった果物を台に補充します。
    //----------------------------------------------------------------------------
    void RouletteSystem::SpawnFruit(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex, const hlslpp::float3& position) {
        if(!registry.HasContext<PrizeFactory>())
            return;

        const FruitCatalog& catalog = registry.GetContext<FruitCatalog>();
        if(fruitIndex < 0 || fruitIndex >= static_cast<int>(catalog.Fruits().size()))
            return;

        const FruitDef& def = catalog.Fruits()[fruitIndex];

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

        registry.GetContext<PrizeFactory>().CreateFruit(registry, def, fruitIndex, variantIndex, color, glow, position);
    }

    //----------------------------------------------------------------------------
    //! ジャックポットチャンスの結果の景品を出します。
    //----------------------------------------------------------------------------
    void RouletteSystem::GrantJackpot(Tsukino::ECS::Registry& registry, bool win) {
        const JackpotConfig jackpot = registry.HasContext<JackpotConfig>() ? registry.GetContext<JackpotConfig>() : JackpotConfig{};
        const int           coins   = win ? jackpot.bonusCoins : jackpot.consolationCoins;

        // コインは投入位置の列のどこかに落とす（タダ。手持ちからは引かない）
        if(registry.HasContext<CoinShowerState>())
            registry.GetContext<CoinShowerState>().Add(coins, win ? jackpot.winShowerSeconds : jackpot.loseShowerSeconds, ShowerPlace::Launch);

        if(!win) {
            m_eventBus.Publish(NoticeEvent{GetTexts(registry).Format("notice.jackpotLose", {{"n", std::to_wstring(coins)}}), jackpot.resultSeconds});
            return;
        }

        // 当たり: ルーレットの回転も増やす（ためておける上限を超えてよい）
        if(registry.HasContext<RouletteState>())
            registry.GetContext<RouletteState>().stock += jackpot.spins;
        m_eventBus.Publish(EffectEvent{"jackpot", jackpot.winEffectPosition});
        if(registry.HasContext<PlayStats>())
            registry.GetContext<PlayStats>().jackpots += 1;
        m_eventBus.Publish(NoticeEvent{GetTexts(registry).Format("notice.jackpotWin", {{"coins", std::to_wstring(coins)}, {"spins", std::to_wstring(jackpot.spins)}}), jackpot.winNoticeSeconds});
    }
}    // namespace FruitMagic::ECS

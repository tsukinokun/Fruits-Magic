//----------------------------------------------------------------------------
//! @file   BalanceProbeSystem.cpp
//! @brief  （バランスの計測用）プレイの集計をログに出すシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/BalanceProbeSystem.hpp>

#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/PlayStats.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/Log.hpp>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief ログを出す間隔（ゲーム内の秒）
        constexpr float kReportInterval = 60.0f;

        //--------------------------------------------------------------
        //! 秒を「m:ss」にします。
        //! @param  [in] seconds 秒
        //! @return 表示用の文字列
        //--------------------------------------------------------------
        std::string Clock(float seconds) {
            const int total = static_cast<int>(seconds);
            const int sec   = total % 60;
            return std::to_string(total / 60) + (sec < 10 ? ":0" : ":") + std::to_string(sec);
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    BalanceProbeSystem::BalanceProbeSystem(Tsukino::ECS::EventBus& eventBus) {
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            const bool payout = e.zone == DropZone::Payout;
            if(e.kind == PrizeKind::Coin)
                (payout ? m_coinPayout : m_coinGutter) += 1;
            else
                (payout ? m_fruitPayout : m_fruitGutter) += 1;
        });
        m_reportTimer = kReportInterval;
    }

    //----------------------------------------------------------------------------
    //! 集計をログに出します。
    //----------------------------------------------------------------------------
    void BalanceProbeSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<GameState>() || !registry.HasContext<PlayStats>())
            return;

        const GameState& state = registry.GetContext<GameState>();
        const PlayStats& stats = registry.GetContext<PlayStats>();
        m_elapsed += deltaTime;

        //--------------------------------------------------------------
        // 強化の購入と図鑑の登録（魔法の解放の目安）は起きた時刻を出す
        //--------------------------------------------------------------
        for(const auto& [id, level] : state.upgradeLevels) {
            if(m_lastUpgrades[id] != level) {
                Tsukino::Core::Log::Info("BALANCE " + Clock(m_elapsed) + " upgrade " + id + " Lv" + std::to_string(level));
                m_lastUpgrades[id] = level;
            }
        }
        const int registered = state.RegisteredCount();
        if(registered != m_lastRegistered && registry.HasContext<MagicCatalog>()) {
            const MagicCatalog& catalog = registry.GetContext<MagicCatalog>();
            for(int i = 0; i < static_cast<int>(catalog.Magics().size()); ++i) {
                if(!catalog.IsUnlocked(i, m_lastRegistered) && catalog.IsUnlocked(i, registered))
                    Tsukino::Core::Log::Info("BALANCE " + Clock(m_elapsed) + " magic " + catalog.Magics()[i].id + " unlocked (zukan " + std::to_string(registered) + ")");
            }
            m_lastRegistered = registered;
        }

        //--------------------------------------------------------------
        // 一定間隔の集計
        //--------------------------------------------------------------
        m_reportTimer -= deltaTime;
        if(m_reportTimer > 0.0f)
            return;
        m_reportTimer += kReportInterval;

        const int coinsIn = stats.coinsLaunched + stats.fairyCoins + stats.showerCoins;
        const int ratio   = (stats.coinsLaunched > 0) ? m_coinPayout * 100 / stats.coinsLaunched : 0;
        Tsukino::Core::Log::Info("BALANCE " + Clock(m_elapsed) + " coins=" + std::to_string(state.coins) + " fruit=" + std::to_string(state.fruitPoints) +
                                 " mana=" + std::to_string(state.mana) + " | in launched=" + std::to_string(stats.coinsLaunched) +
                                 " fairy=" + std::to_string(stats.fairyCoins) + " shower=" + std::to_string(stats.showerCoins) + " (total " +
                                 std::to_string(coinsIn) + ") | out payout=" + std::to_string(m_coinPayout) + " gutter=" + std::to_string(m_coinGutter) +
                                 " payout/launched=" + std::to_string(ratio) + "% | fruits harvest=" + std::to_string(m_fruitPayout) +
                                 " lost=" + std::to_string(m_fruitGutter) + " zukan=" + std::to_string(registered) + " | roulette " +
                                 std::to_string(stats.rouletteHits) + "/" + std::to_string(stats.rouletteSpins) + " jackpot=" + std::to_string(stats.jackpots) +
                                 " magic=" + std::to_string(stats.magicsCast) + " tree=" + std::to_string(state.treeLevel));
    }
}    // namespace FruitMagic::ECS

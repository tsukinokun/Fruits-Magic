//----------------------------------------------------------------------------
//! @file   BalanceProbeSystem.hpp
//! @brief  （バランスの計測用）プレイの集計を一定間隔でログに出すシステム
//! @detail 環境変数 FRUITMAGIC_AUTOPLAY=1 で起動したときだけシーンが登録します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <map>
#include <string>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! プレイの集計をログに出すシステムです。60 秒ごとに「BALANCE」で始まる行を出し、
    //! 強化を買ったときと魔法を覚えたときはその時刻も出します。
    class BalanceProbeSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit BalanceProbeSystem(Tsukino::ECS::EventBus& eventBus);

        //! 集計をログに出します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::ScopedConnection m_dropConnection;          // 購読解除を自動で行う接続
        float                          m_elapsed      = 0.0f;     // 起動してからのゲーム内時間（秒）
        float                          m_reportTimer  = 0.0f;     // 次のログまでの時間（秒）
        int                            m_coinPayout   = 0;        // 払い出し口に落ちたコイン
        int                            m_coinGutter   = 0;        // 溝に落ちたコイン
        int                            m_fruitPayout  = 0;        // 収穫した果物
        int                            m_fruitGutter  = 0;        // 溝に落ちた果物
        std::map<std::string, int>     m_lastUpgrades;            // 前に見た強化のレベル
        int                            m_lastRegistered = 0;      // 前に見た図鑑の登録数
    };
}    // namespace FruitMagic::ECS

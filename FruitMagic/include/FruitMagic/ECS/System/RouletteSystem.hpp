//----------------------------------------------------------------------------
//! @file   RouletteSystem.hpp
//! @brief  チェッカーに入ったコインでルーレットを回し、当たりで果物を補充するシステム（まれにジャックポットチャンス）
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <random>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! ルーレットを回し、当たりで果物を補充するシステムです。
    //! 回すたびに JackpotConfig::chanceRate で「ジャックポットチャンス」になり、2段目の抽選に当たると
    //! コインのシャワーとルーレットの追加の回転、外れると残念賞のコインが出ます。
    class RouletteSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus CheckerEnteredEvent の購読と NoticeEvent の発行に使うイベントバス
        explicit RouletteSystem(Tsukino::ECS::EventBus& eventBus);

        //! ルーレットを進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! 当たった果物を台に補充します。
        //! @param  [in] registry     レジストリ
        //! @param  [in] fruitIndex   補充する果物の添字
        //! @param  [in] variantIndex 補充する果物のバリエーションの添字
        void SpawnFruit(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex);

        //! ジャックポットチャンスの結果の景品を出します。
        //! @param  [in] registry レジストリ
        //! @param  [in] win      当たりか
        void GrantJackpot(Tsukino::ECS::Registry& registry, bool win);

        Tsukino::ECS::EventBus&        m_eventBus;                 // お知らせを出すイベントバス

        Tsukino::ECS::ScopedConnection m_enteredConnection;        // 購読解除を自動で行う接続
        int                            m_pendingEntered = 0;       // 次の Update でストックに積む数
        float                          m_timer          = 0.0f;    // 今の段階の残り時間（秒）
        float                          m_flipTimer      = 0.0f;    // 回転中の表示を切り替えるまでの時間（秒）
        int                            m_resultFruit    = -1;      // 抽選済みの結果（-1 はハズレ）
        int                            m_resultVariant  = 0;       // 抽選済みの結果のバリエーション
        bool                           m_jackpotWin     = false;   // 抽選済みのジャックポットチャンスの結果
        std::mt19937                   m_rng;                      // 抽選用の乱数
    };
}    // namespace FruitMagic::ECS

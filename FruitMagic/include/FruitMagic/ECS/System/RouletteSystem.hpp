//----------------------------------------------------------------------------
//! @file   RouletteSystem.hpp
//! @brief  チェッカーに入ったコインでルーレットを回し、当たりで果物を補充するシステム（まれにジャックポットチャンス）
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <FruitMagic/Game/RouletteState.hpp>

#include <hlsl++.h>

#include <array>
#include <random>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! ルーレット（3リールのスロット）を回し、当たりで果物を補充するシステムです。
    //! 結果は回し始めに抽選し、各列に止める絵柄（果物の当たりはその果物が3つ、コインはコインが3つ、ハズレはそろわない並び）と
    //! 止まる時刻を決めて RouletteState に書きます。表示は SlotMachineSystem が、それに合わせてリールを止めます。
    //! 1・2列目がそろうとリーチで、3列目を長く回します。当たった果物は、スロットから台へ飛び終わってから台に出します。
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

        //! 当たった果物を台に置く位置を決めます。
        //! @param  [in] registry   レジストリ
        //! @param  [in] fruitIndex 果物の添字
        //! @return 置く位置（ワールド座標）
        hlslpp::float3 SpawnPosition(Tsukino::ECS::Registry& registry, int fruitIndex);

        //! 当たった果物を台に補充します。
        //! @param  [in] registry     レジストリ
        //! @param  [in] fruitIndex   補充する果物の添字
        //! @param  [in] variantIndex 補充する果物のバリエーションの添字
        //! @param  [in] position     置く位置（SpawnPosition で決めたもの）
        void SpawnFruit(Tsukino::ECS::Registry& registry, int fruitIndex, int variantIndex, const hlslpp::float3& position);

        //! ハズレの並び（3つがそろわない）を決めます。ときどき2列だけそろえて「ハズレのリーチ」にします。
        //! @param  [in] symbols         使える絵柄（果物の添字と kCoinSymbol）
        //! @param  [in] reachMissChance ハズレのリーチにする確率
        //! @return 各列の絵柄
        std::array<int, kReelCount> PickMissSymbols(const std::vector<int>& symbols, float reachMissChance);

        //! ジャックポットチャンスの結果の景品を出します。
        //! @param  [in] registry レジストリ
        //! @param  [in] win      当たりか
        void GrantJackpot(Tsukino::ECS::Registry& registry, bool win);

        Tsukino::ECS::EventBus&        m_eventBus;                 // お知らせを出すイベントバス

        Tsukino::ECS::ScopedConnection m_enteredConnection;        // 購読解除を自動で行う接続
        int                            m_pendingEntered = 0;       // 次の Update でストックに積む数
        float                          m_timer          = 0.0f;    // 結果を出しておく残り時間（秒）
        int                            m_resultFruit    = -1;      // 抽選済みの結果（-1 はハズレ）
        int                            m_resultVariant  = 0;       // 抽選済みの結果のバリエーション
        bool                           m_jackpotWin     = false;   // 抽選済みのジャックポットチャンスの結果
        int                            m_resultCoins    = 0;       // 抽選済みのコイン当たりの枚数（0 は無し）
        std::mt19937                   m_rng;                      // 抽選用の乱数
    };
}    // namespace FruitMagic::ECS

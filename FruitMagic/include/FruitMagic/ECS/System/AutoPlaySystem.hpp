//----------------------------------------------------------------------------
//! @file   AutoPlaySystem.hpp
//! @brief  （バランスの計測用）自動で遊ぶシステム
//! @detail 環境変数 FRUITMAGIC_AUTOPLAY=1 で起動したときだけシーンが登録します。セーブは読み書きしません。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <random>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 自動で遊ぶシステムです。普通のプレイヤーの遊び方をまねて、
    //! 0.6 秒ごとにランダムな位置へコインを入れ、撃てる魔法があれば撃ち、買える強化は安い順に買います。
    //! 強化を買っても投入用のコインが残るよう、手持ちを少し残します。
    class AutoPlaySystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent を発行するイベントバス
        explicit AutoPlaySystem(Tsukino::ECS::EventBus& eventBus);

        //! 投入・魔法・強化を行います。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus& m_eventBus;              // 魔法を撃つイベントバス
        std::mt19937            m_rng;                   // 投入位置の乱数
        float                   m_launchTimer  = 0.0f;   // 次の投入までの時間（秒）
        float                   m_decideTimer  = 0.0f;   // 次に魔法・強化を考えるまでの時間（秒）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   GrowMagicSystem.hpp
//! @brief  魔法「おおきくなーれ」（台の上の果物1つを巨大化する）の効果を実装するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 魔法「おおきくなーれ」の効果です。id が "grow" の魔法が撃たれると、台の上の果物のうち
    //! 最も手前（落ちそうな）1つを、大きさ × scale・重さ × scale^2・価値 × valueMultiplier の果物に作り直します。
    //! 大きくした果物はもう一度は大きくしません。台の上に果物が無ければマナを返します。
    class GrowMagicSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus MagicCastEvent の購読と NoticeEvent の発行に使うイベントバス
        explicit GrowMagicSystem(Tsukino::ECS::EventBus& eventBus);

        //! 撃たれていれば果物を大きくします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        Tsukino::ECS::EventBus&        m_eventBus;              // お知らせを出すイベントバス
        Tsukino::ECS::ScopedConnection m_castConnection;        // 購読解除を自動で行う接続
        int                            m_pendingCast = -1;     // 次の Update で効果を出す魔法の添字
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   PopupSystem.hpp
//! @brief  払い出し口に落ちた物の「+1」「いちご！」をふわっと浮かせて消すシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 落ちたときのポップのシステムです。払い出し口に落ちた位置に、コインは「+1」、果物は名前を出し、
    //! 上へ浮かせながら薄くして消します。続けて落ちたコインはまとめて「+3」のように出します。
    class PopupSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit PopupSystem(Tsukino::ECS::EventBus& eventBus);

        //! ポップを出して動かします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! 払い出し口に落ちた物です。
        struct Drop {
            bool  fruit        = false;    // 果物か（false はコイン）
            int   fruitIndex   = -1;       // 果物の添字
            int   variantIndex = 0;        // 果物のバリエーションの添字
            float x            = 0.0f;     // 落ちた左右位置
        };

        Tsukino::ECS::ScopedConnection m_dropConnection;        // 購読解除を自動で行う接続
        std::vector<Drop>              m_pending;               // 次の Update で出すポップ
        int                            m_coinCount = 0;         // まとめ中のコインの枚数
        float                          m_coinXSum  = 0.0f;      // まとめ中のコインの左右位置の合計（平均を出す）
        float                          m_coinTimer = 0.0f;      // まとめ中のコインを出すまでの時間（秒）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   PopupSystem.hpp
//! @brief  払い出し口に落ちた物の「+1」「いちご！」をふわっと浮かせて消すシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>

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
    //! 左右の溝に落ちた物は、溝ごとにまとめて増えたマナ（「マナ +9」）を溝の上に出します。
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

        //! 片側の溝でまとめ中のマナです。
        struct GutterGather {
            int   count = 0;       // まとめ中の数（0 ならまとめていない）
            float mana  = 0.0f;    // 増えたマナの合計（倍率を掛けた値）
            float zSum  = 0.0f;    // 落ちた前後位置の合計（平均を出す）
            float timer = 0.0f;    // 出すまでの時間（秒）
        };

        Tsukino::ECS::ScopedConnection m_dropConnection;        // 購読解除を自動で行う接続
        std::vector<Drop>              m_pending;               // 次の Update で出すポップ
        std::vector<PrizeDroppedEvent> m_pendingGutter;         // 次の Update でまとめる溝に落ちた物
        int                            m_coinCount = 0;         // まとめ中のコインの枚数
        float                          m_coinXSum  = 0.0f;      // まとめ中のコインの左右位置の合計（平均を出す）
        float                          m_coinTimer = 0.0f;      // まとめ中のコインを出すまでの時間（秒）
        GutterGather                   m_gutter[2];             // 溝ごとのまとめ中のマナ（[0] が x の負の側）
    };
}    // namespace FruitMagic::ECS

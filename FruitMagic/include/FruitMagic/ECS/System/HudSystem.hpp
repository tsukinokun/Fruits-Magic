//----------------------------------------------------------------------------
//! @file   HudSystem.hpp
//! @brief  HUD（手持ち枚数・マナ・払い出し・収穫・ルーレット・魔法ボタン）を更新するシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

#include <string>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {
    struct GameState;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! HUD（手持ち枚数・払い出し表示）を更新するシステムです。
    class HudSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent を購読するイベントバス
        explicit HudSystem(Tsukino::ECS::EventBus& eventBus);

        //! HUD のテキストを更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! マナゲージの中身のバーの幅を、今のマナに合わせます。
        //! @param  [in] registry レジストリ
        //! @param  [in] state    プレイヤーの資源
        void UpdateManaGauge(Tsukino::ECS::Registry& registry, const GameState& state);

        //! おすそわけ待ちのリングを、待っている間だけ出し、次の1枚までの進み具合で塗ります。
        //! @param  [in] registry レジストリ
        void UpdateReliefGauge(Tsukino::ECS::Registry& registry);

        //! 魔法ボタンの色と文字を、解放状態・マナ・効果中かに合わせます。
        //! @param  [in] registry レジストリ
        //! @param  [in] state    プレイヤーの資源
        void UpdateMagicButtons(Tsukino::ECS::Registry& registry, const GameState& state);

        //! 図鑑の登録数で新しく解放された魔法があれば、お知らせを出します。
        //! @param  [in] registry レジストリ
        //! @param  [in] state    プレイヤーの資源
        void CheckMagicUnlocks(Tsukino::ECS::Registry& registry, const GameState& state);

        Tsukino::ECS::ScopedConnection m_dropConnection;          // 購読解除を自動で行う接続
        int                            m_recentPayout = 0;        // 表示中の「払い出し」の合計
        int                            m_recentGutter = 0;        // 表示中の「溝に落ちた」数
        float                          m_popupTimer   = 0.0f;     // 払い出し表示を消すまでの残り時間（秒）
        float                          m_payoutPopupSeconds     = 1.2f;    // 払い出し表示を出しておく時間（Ui.json。Update で覚える）
        float                          m_harvestPopupSeconds    = 2.0f;    // 収穫表示を出しておく時間（同上）
        float                          m_registeredPopupSeconds = 3.5f;    // 「図鑑に登録！」を出しておく時間（同上）
        Tsukino::ECS::ScopedConnection m_zukanConnection;         // 図鑑登録の購読解除を自動で行う接続
        int                            m_harvestFruit = -1;       // 表示中の収穫した果物の添字（-1 は表示なし）
        int                            m_harvestVariant = 0;      // 表示中の収穫した果物のバリエーションの添字
        bool                           m_harvestIsNew = false;    // 表示中の収穫が図鑑への初登録か
        float                          m_harvestTimer = 0.0f;     // 収穫表示を消すまでの残り時間（秒）
        Tsukino::ECS::ScopedConnection m_noticeConnection;        // お知らせの購読解除を自動で行う接続
        std::wstring                   m_noticeText;              // 表示中のお知らせ
        float                          m_noticeTimer  = 0.0f;     // お知らせを消すまでの残り時間（秒）
        int                            m_lastUnlocked = -1;       // 前のフレームの解放済みの魔法の数（-1 はまだ数えていない）
    };
}    // namespace FruitMagic::ECS

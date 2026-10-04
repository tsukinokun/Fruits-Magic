//----------------------------------------------------------------------------
//! @file   HudSystem.hpp
//! @brief  HUD（手持ち枚数・マナ・払い出し・収穫・ルーレット・魔法ボタン）を更新するシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>

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

        //! 魔法ボタンの色と文字を、解放状態・マナ・効果中かに合わせます。
        //! @param  [in] registry レジストリ
        //! @param  [in] state    プレイヤーの資源
        void UpdateMagicButtons(Tsukino::ECS::Registry& registry, const GameState& state);

        Tsukino::ECS::ScopedConnection m_dropConnection;          // 購読解除を自動で行う接続
        int                            m_recentPayout = 0;        // 表示中の「払い出し」の合計
        int                            m_recentGutter = 0;        // 表示中の「溝に落ちた」数
        float                          m_popupTimer   = 0.0f;     // 払い出し表示を消すまでの残り時間（秒）
        int                            m_harvestFruit = -1;       // 表示中の収穫した果物の添字（-1 は表示なし）
        float                          m_harvestTimer = 0.0f;     // 収穫表示を消すまでの残り時間（秒）
    };
}    // namespace FruitMagic::ECS

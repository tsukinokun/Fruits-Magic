//----------------------------------------------------------------------------
//! @file   UpgradeSystem.hpp
//! @brief  台の強化の購入と、強化の効果を台に反映するシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {
    struct GameState;     // 前方宣言
    struct UpgradeDef;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 台の強化のシステムです。強化画面が開いている間、購入ボタンのクリックで
    //! コインと果実を払ってレベルを上げ、画面の文字・ボタンの色を更新します。
    //! 効果の値が何に効くかは強化の id で決まります（"pusherStroke" → プッシャーの振幅 など）。
    class UpgradeSystem : public Tsukino::ECS::ISystem {
    public:

        //! 購入の入力を読み、強化画面を更新します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

        //! 今の強化のレベルから、台の性能（TableStats）と果樹の段階（GameState::treeLevel）を計算し直します。
        //! @param  [in] registry レジストリ（UpgradeCatalog・GameState・TableStats を参照する）
        //! @note   起動時（セーブの読み込み後）と購入のたびに呼びます
        static void ApplyUpgrades(Tsukino::ECS::Registry& registry);

        //! 強化の次のレベルを今の手持ちで買えるかを返します（強化画面と、画面の横のおすすめが使う）。
        //! @param  [in] def   強化の定義
        //! @param  [in] state プレイヤーの資源
        //! @return 買えれば true（最大レベルなら false）
        static bool CanAfford(const UpgradeDef& def, const GameState& state);

        //! 強化を1段階買います（強化画面の購入ボタンと、バランス計測の自動プレイが使う）。
        //! @param  [in] registry     レジストリ
        //! @param  [in] upgradeIndex 強化の添字
        //! @return 買えたら true（最大レベル・手持ち不足なら false）
        static bool Purchase(Tsukino::ECS::Registry& registry, int upgradeIndex);
    };
}    // namespace FruitMagic::ECS

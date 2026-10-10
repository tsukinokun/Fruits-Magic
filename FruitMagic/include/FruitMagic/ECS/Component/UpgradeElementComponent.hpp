//----------------------------------------------------------------------------
//! @file   UpgradeElementComponent.hpp
//! @brief  強化画面の中身が変わる要素（強化の名前・効果・価格・購入ボタン・手持ち）のコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/UpgradeCatalog.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 強化画面の要素の種類です。
    enum class UpgradeElementKind {
        Name,         // 強化の名前とレベル（「果樹   Lv 1 / 3」）。説明文は決まった文字なので MenuPageComponent::text で出す
        Effect,       // 効果の今の値と次の値（「12cm → 14cm」）
        Cost,         // 次のレベルの価格（「コイン 40」「FP 5」）
        BuyButton,    // 購入ボタン（スプライト。PointerTargetComponent と一緒に付ける）
        BuyLabel,     // 購入ボタンの文字（「強化する」「足りない」「最大」）
        Wallet,       // 手持ちのコインと FP
        Tab,          // タブのボタン（スプライト。PointerTargetComponent と一緒に付ける。currency のタブに切り替える）
        TabGroup,     // タブの中身の親（UIVisibilityComponent を持つ。currency のタブを選んでいる間だけ見せる）
    };

    //! 強化画面の中身が変わる要素です。MenuPageComponent と一緒に付け、強化画面が開いている間 UpgradeSystem が更新します
    //! （TabGroup は画面の部品ではないので MenuPageComponent を付けない）。
    struct UpgradeElementComponent {
        UpgradeElementKind kind         = UpgradeElementKind::Name;    // 種類
        int                upgradeIndex = -1;                          // 強化の添字（UpgradeCatalog::Upgrades()）。Wallet・Tab・TabGroup は -1
        UpgradeCurrency    currency     = UpgradeCurrency::Coins;     // Tab・TabGroup の受け持つタブ（払うもの）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   HudTextComponent.hpp
//! @brief  HUD のテキストが何を表示するかを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! HUD に表示する内容の種類です。
    enum class HudTextKind {
        Coins,           // 手持ちのコイン枚数
        Mana,            // マナ（今の量 / 上限）
        DropPopup,       // 直近のコインの払い出し（+1 / 溝）
        FruitPoints,     // 手持ちのフルーツポイント（FP）
        HarvestPopup,    // 直近に収穫した果物（「いちご ゲット！」）
        Roulette,        // ルーレット（回転中の表示・結果・ストック）
        ControlsHint,    // 操作説明
        Relief,          // おすそわけ待ち（手持ちが少ない間だけ、次の1枚までの秒数）
        Notice,          // お知らせ（「新しい魔法を覚えた！」「ジャックポット！」など。NoticeEvent で出す）
    };

    //! HUD のテキストです。FontComponent と一緒に付け、HudSystem が文字列を更新します。
    struct HudTextComponent {
        HudTextKind kind = HudTextKind::Coins;    // 表示する内容
    };

    //! 「〇〇 ゲット！」の左に出す果物の置き台です。FruitIconComponent と一緒に付け、HudSystem が中身と表示を決めます。
    struct HarvestIconComponent {
        float sizePixels = 36.0f;    // 出す大きさ（果物の外形のいちばん長い向きのピクセル数）
    };
}    // namespace FruitMagic::ECS

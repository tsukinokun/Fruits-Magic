//----------------------------------------------------------------------------
//! @file   SlotMachineComponents.hpp
//! @brief  ルーレット（3リールのスロット）の表示の部品を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>

#include <hlsl++.h>

#include <vector>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! スロットの板・窓・縁・玉の種類です。
    enum class SlotPart {
        Panel,     // 板
        Window,    // リールの窓の地
        Flash,     // 窓の周りの光る縁（index は列）
        Lamp,      // 「のこり」の玉（index は何個目か。ふだんのスロットだけ）
    };

    //! スロットの板・窓・縁・玉です（画面スプライト）。SlotMachineSystem が出し入れと色を決めます。
    //! @note 画面スプライトはスケール 0 で描画されないので、出すときのスケールを覚えておく
    struct SlotPartComponent {
        bool           jackpot    = false;                                // ジャックポットの大きなスロットの部品か
        SlotPart       part       = SlotPart::Panel;                      // 種類
        int            index      = 0;                                    // 列・玉の番号
        hlslpp::float3 shownScale = hlslpp::float3(1.0f, 1.0f, 1.0f);     // 出すときのスケール
        hlslpp::float4 color      = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 基本の色（光り方の強さで透明度を変える）
    };

    //! リールの止まり方の段階です。
    enum class SlotReelMotion {
        Stopped,     // 止まっている
        Spinning,    // 一定の速さで回っている
        Stopping,    // 狙いの絵柄に向かって減速している（少し行き過ぎて戻る）
    };

    //! リール1列です。窓（UIClipComponent を持つエンティティ）に付けます。
    //! 絵柄は並び（strip）の順に縦に並んで流れ、窓の外は切れます。絵柄は FruitIcon の置き台で、作り直さずに位置だけを動かします。
    struct SlotReelComponent {
        bool                              jackpot = false;           // ジャックポットの大きなスロットのリールか
        int                               reel    = 0;               // 列（0 が左）
        std::vector<Tsukino::ECS::Entity> anchors;                   // 絵柄の位置の部品（窓の子。置き台の anchor）
        std::vector<Tsukino::ECS::Entity> icons;                     // 絵柄の置き台（FruitIcon）
        std::vector<int>                  strip;                     // 並び（果物の添字か kCoinSymbol）。空なら次の Update で作る
        int                               stripLevel   = -1;         // 並びを作ったときの果樹の段階（変わったら作り直す）
        float                             scroll       = 0.0f;       // 流れた量（ピクセル。増えると絵柄が下へ動く）
        float                             speed        = 0.0f;       // 今の速さ（ピクセル/秒）
        SlotReelMotion                    motion       = SlotReelMotion::Stopped;    // 止まり方の段階
        int                               spinId       = 0;          // 見ている回転の番号（RouletteState::spinId と違えば新しい回転）
        float                             stopFrom     = 0.0f;       // 減速を始めたときの流れた量
        float                             stopDistance = 0.0f;       // 減速を始めてから止まるまでに流れる量
        float                             stopStart    = 0.0f;       // 減速を始めた時刻（回し始めてからの秒）
        int                               variantIcon  = -1;         // 色違いの色で出している絵柄（次の回転で通常に戻す。-1 は無し）
        float                             flash        = 0.0f;       // 縁が光っている残り時間（秒）
    };

    //! 当たりの果物がスロットから台へ飛ぶときの置き台（FruitIcon）の目印です。
    struct SlotFlyerComponent {
        int            spinId = 0;                                   // 飛ばしている回転の番号
        hlslpp::float2 from   = hlslpp::float2(0.0f, 0.0f);         // 飛び始める所（UI の座標）
    };
}    // namespace FruitMagic::ECS

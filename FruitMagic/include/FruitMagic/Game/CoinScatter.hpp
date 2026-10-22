//----------------------------------------------------------------------------
//! @file   CoinScatter.hpp
//! @brief  起動時に台へ置くコインの散らばらせ方（位置と向きを決めるだけ。物理や ECS には触れない）
//! @detail 本物のプッシャーのように、ところどころ重なり、手前ほどコインが多く、手前の縁から少しはみ出したコインがある並びを作ります。
//!         置いた後は PusherScene が物理を少し動かして落ち着かせます。
//!         - 床の層（押す力を伝える役）: 隙間なく詰め、1行ごとに半分ずらし（れんが積み）、位置と向きを少し揺らし、奥ほど抜けを作る。
//!           手前の列のいくつかは縁から少しはみ出させる（重心は台の上）。
//!           ※ ポアソン円盤サンプリングでばらばらに置くと詰まり方が足りず、押す力が手前の端まで伝わらないので使わない
//!         - 重なり（見た目の主役）: 床のコインを手前ほど選ばれやすく選び、少しずらして傾けて乗せる（まれにもう1段）。
//!           重ねるコインどうしは、回した四角の分離軸の判定で重ならないようにする
//!         設定は Table.json の initialPrizes（TableLayout）。
//----------------------------------------------------------------------------
#pragma once
#include <hlsl++.h>

#include <random>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {
    struct TableLayout;    // 前方宣言

    //! コイン1枚の置き場所です。
    struct CoinPlacement {
        hlslpp::float3     position = hlslpp::float3(0.0f, 0.0f, 0.0f);             // 中心のワールド座標
        hlslpp::quaternion rotation = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);   // 向き
    };
}    // namespace FruitMagic

// 名前空間 : FruitMagic::CoinScatter
namespace FruitMagic::CoinScatter {

    //! プッシャーの前の床に置くコイン（縁のはみ出し・床の層・重なり）の置き場所を作ります。
    //! @param  [in] layout   台の寸法と散らばらせ方の設定
    //! @param  [in] coinHalf コインの半分の大きさ（x と z は同じとして扱う）
    //! @param  [in] rng      乱数
    //! @return 置き場所の一覧（床の層 → 重なりの順）
    std::vector<CoinPlacement> ScatterFloor(const TableLayout& layout, const hlslpp::float3& coinHalf, std::mt19937& rng);

    //! プッシャーの上面に置くコインの置き場所を作ります（重ならないよう、ばらばらの位置と向きで）。
    //! @param  [in] layout   台の寸法と散らばらせ方の設定
    //! @param  [in] coinHalf コインの半分の大きさ
    //! @param  [in] rng      乱数
    //! @return 置き場所の一覧
    std::vector<CoinPlacement> ScatterPusherTop(const TableLayout& layout, const hlslpp::float3& coinHalf, std::mt19937& rng);
}    // namespace FruitMagic::CoinScatter

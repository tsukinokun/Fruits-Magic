//----------------------------------------------------------------------------
//! @file   CoinLauncherSystem.hpp
//! @brief  プレイヤーの入力でコインを投入するシステム
//! @detail ←→キーかマウスの左右で投入位置を選び、Space か左クリックで手持ちのコインを1枚投入します。
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! プレイヤーの入力でコインを投入するシステムです。
    class CoinLauncherSystem : public Tsukino::ECS::ISystem {
    public:

        //! 投入位置の更新とコインの投入を行います。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        int m_lastMouseX = -1;    // 前フレームのマウスX（動いたときだけマウスに追従させる）
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   FairySystem.hpp
//! @brief  妖精が一定間隔でコインを入れるシステム（妖精の自動投入の強化）と、手持ちが尽きかけたときのおすそわけ
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <random>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 妖精の自動投入のシステムです。TableStats::autoLaunchInterval 秒ごとに、
    //! 投入できる範囲のランダムな位置へコインを1枚入れます。妖精のコインは手持ちから引きません（放置の土台）。
    //! また、手持ちが EconomyConfig::reliefBelow 枚より少ない間は、reliefSeconds ごとに手持ちへ1枚くれます（詰み防止）。
    class FairySystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        FairySystem();

        //! 間隔が来たらコインを入れます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        std::mt19937 m_rng;             // 投入位置の乱数
        float        m_timer = 0.0f;    // 前の投入からの経過時間（秒）
        float        m_reliefTimer = 0.0f;    // 前のおすそわけからの経過時間（秒）
    };
}    // namespace FruitMagic::ECS

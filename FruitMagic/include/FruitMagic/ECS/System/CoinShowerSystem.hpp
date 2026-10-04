//----------------------------------------------------------------------------
//! @file   CoinShowerSystem.hpp
//! @brief  コインのシャワーの依頼に従ってコインを降らせるシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <random>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! コインのシャワーのシステムです。CoinShowerState の依頼を、プッシャーが届かない台の手前側の
    //! ランダムな位置へ、敷き詰めたコインのすぐ上から1枚ずつ落とします（手持ちからは引かない）。
    //! 台の上のコインが EconomyConfig::maxCoinsOnTable 枚以上の間は、減るまで待ちます。
    class CoinShowerSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        CoinShowerSystem();

        //! 依頼を進めます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        std::mt19937 m_rng;    // 落とす位置の乱数
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   RecordSystem.hpp
//! @brief  記録画面の数を書くシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 記録画面のシステムです。記録画面が開いている間（開閉は MenuSystem）、
    //! 収穫の記録（GameState の収穫数など）と、これまでの累計（GameState::stats）を書きます。
    class RecordSystem : public Tsukino::ECS::ISystem {
    public:

        //! 記録画面が開いていれば、数を書きます。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   SaveSystem.hpp
//! @brief  一定間隔で自動セーブするシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 自動セーブのシステムです。OfflineConfig::autosaveSeconds ごとに OfflineConfig::savePath へ保存します。
    //! 終了時の保存はシーンの OnExit で行います（強制終了されても直前の自動セーブまでは残る）。
    class SaveSystem : public Tsukino::ECS::ISystem {
    public:

        //! 間隔が来たら保存します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:
        float m_timer = 0.0f;    // 前の保存からの経過時間（秒）
    };
}    // namespace FruitMagic::ECS

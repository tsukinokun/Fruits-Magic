//----------------------------------------------------------------------------
//! @file   EffectsSystem.hpp
//! @brief  光の粒（キラキラ）・画面の光・ぽよんの演出を出して動かすシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/ECS/Event/EffectEvent.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>

#include <random>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {
    struct EffectPreset;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 演出のシステムです。
    //! - EffectEvent（ほかのシステムからの依頼）と、魔法・色違いの収穫・図鑑登録の出来事で、プリセットの光の粒を出す
    //! - 台の上の色違い・金色の果物から、時々小さな粒をこぼす
    //! - 光の粒を動かし、寿命で消す。画面の光を弱めていく。ぽよんの動きを進める
    //! - 魔法「ふくらむ」の間、プッシャーの輪郭をピンクに脈打たせる
    class EffectsSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus 出来事を購読するイベントバス
        explicit EffectsSystem(Tsukino::ECS::EventBus& eventBus);

        //! 演出を出して動かします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! プリセットの粒をまとめて出し、画面を光らせます。
        //! @param  [in] registry レジストリ
        //! @param  [in] preset   プリセット
        //! @param  [in] position 出す位置
        void Burst(Tsukino::ECS::Registry& registry, const EffectPreset& preset, const hlslpp::float3& position);

        //! 光の粒を動かし、寿命が来たものを消します。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 経過時間（秒）
        void UpdateParticles(Tsukino::ECS::Registry& registry, float deltaTime);

        Tsukino::ECS::ScopedConnection m_effectConnection;      // EffectEvent の購読
        Tsukino::ECS::ScopedConnection m_castConnection;        // MagicCastEvent の購読
        Tsukino::ECS::ScopedConnection m_dropConnection;        // PrizeDroppedEvent の購読
        Tsukino::ECS::ScopedConnection m_zukanConnection;       // ZukanRegisteredEvent の購読
        std::vector<EffectEvent>       m_pending;               // 次の Update で出す粒
        hlslpp::float3                 m_lastFruitDrop = hlslpp::float3(0.0f, -15.0f, 44.0f);    // 直近に収穫した果物の落ちた位置（図鑑登録の演出に使う）
        Tsukino::Asset::AssetHandle    m_sparkleTexture;        // 星形の画像
        Tsukino::Asset::AssetHandle    m_glowTexture;           // ぼかした円の画像
        bool                           m_texturesLoaded = false;    // 画像を読み込んだか
        int                            m_particleCount  = 0;        // 今出ている粒の数
        float                          m_idleTimer      = 0.0f;     // 次に色違いの果物から粒をこぼすまでの時間（秒）
        float                          m_time           = 0.0f;     // 脈打つ光に使う経過時間（秒）
        std::mt19937                   m_rng;                       // 粒のばらつき
    };
}    // namespace FruitMagic::ECS

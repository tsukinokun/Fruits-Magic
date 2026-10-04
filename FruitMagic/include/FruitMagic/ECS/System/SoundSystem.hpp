//----------------------------------------------------------------------------
//! @file   SoundSystem.hpp
//! @brief  出来事に合わせて効果音を鳴らすシステム
//----------------------------------------------------------------------------
#pragma once
#include <FruitMagic/Game/RouletteState.hpp>
#include <FruitMagic/Game/MenuState.hpp>

#include <Tsukino/Core/ECS/System/ISystem.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>
#include <Tsukino/Engine/Asset/AssetHandle.hpp>

#include <string>
#include <unordered_map>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 効果音のシステムです。鳴らす音のファイル・音量・最短の間隔は Assets/Data/Sounds.json で決めます。
    //! - 出来事（落下・図鑑登録・魔法）はイベントで、投入・強化・画面の開閉・ルーレットの進み具合は状態の変化で知る
    //! - 同じ音は minInterval 秒より短い間隔では鳴らさない（コインがまとめて落ちても数回だけ鳴る）
    //! - M キーで消音を切り替える
    class SoundSystem : public Tsukino::ECS::ISystem {
    public:

        //! コンストラクタです。
        //! @param  [in] eventBus 出来事の購読と、消音の切り替えのお知らせに使うイベントバス
        //! @param  [in] path     設定ファイル（Sounds.json）
        SoundSystem(Tsukino::ECS::EventBus& eventBus, const std::string& path);

        //! 出来事に合わせて効果音を鳴らします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! 効果音1つの設定です。
        struct Sound {
            std::string                 file;                 // ファイル（リポジトリルート相対）
            float                       volume      = 1.0f;   // 音量（0〜1）
            float                       minInterval = 0.05f;  // 同じ音を鳴らす最短の間隔（秒）
            Tsukino::Asset::AssetHandle handle;               // 読み込んだ音
            float                       cooldown    = 0.0f;   // 次に鳴らせるまでの時間（秒）
        };

        //! 設定ファイルを読み込みます。
        //! @param  [in] path 設定ファイル
        void Load(const std::string& path);

        //! 音を鳴らします（間隔が短すぎる・読み込めていない音は鳴らさない）。
        //! @param  [in] registry レジストリ
        //! @param  [in] name     音の名前（Sounds.json の "sounds" のキー）
        void Play(Tsukino::ECS::Registry& registry, const std::string& name);

        Tsukino::ECS::EventBus&                m_eventBus;          // 消音の切り替えのお知らせを出すイベントバス
        Tsukino::ECS::ScopedConnection         m_dropConnection;    // PrizeDroppedEvent の購読
        Tsukino::ECS::ScopedConnection         m_zukanConnection;   // ZukanRegisteredEvent の購読
        Tsukino::ECS::ScopedConnection         m_castConnection;    // MagicCastEvent の購読
        std::unordered_map<std::string, Sound> m_sounds;            // 名前ごとの効果音
        std::vector<std::string>               m_pending;           // 次の Update で鳴らす音
        float                                  m_masterVolume = 0.8f;    // 全体の音量
        bool                                   m_muted        = false;   // 消音中か
        bool                                   m_loaded       = false;   // 音を読み込んだか（最初の Update で読む）

        //--------------------------------------------------------------
        // 状態の変化で知る出来事のための、前のフレームの値
        //--------------------------------------------------------------
        int           m_lastLaunched      = 0;                     // プレイヤーが投入したコインの数
        int           m_lastUpgradeLevels = -1;                    // 強化のレベルの合計（-1 はまだ見ていない）
        MenuKind      m_lastMenu          = MenuKind::None;        // 開いている画面
        RoulettePhase m_lastPhase         = RoulettePhase::Idle;   // ルーレットの段階
        int           m_lastDisplayFruit  = -1;                    // ルーレットが表示している果物
        bool          m_lastJackpotDisplay = false;                // ジャックポットの抽選の表示
    };
}    // namespace FruitMagic::ECS

//----------------------------------------------------------------------------
//! @file   CutInSystem.hpp
//! @brief  果物が取れたときのカットイン（帯・果物の 3D モデル・名前）を出すシステム
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <Tsukino/Core/ECS/Event/ScopedConnection.hpp>
#include <Tsukino/Core/ECS/System/ISystem.hpp>

#include <deque>
#include <vector>

// 名前空間 : Tsukino::ECS
namespace Tsukino::ECS {
    class EventBus;    // 前方宣言
}

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 果物が取れたときのカットインを出すシステムです。
    //! 図鑑に初めて載ったとき（ZukanRegisteredEvent）と、色違い・金色が払い出し口に落ちたとき（PrizeDroppedEvent）に、
    //! 画面の中ほどへ帯を出し、果物の 3D モデル（ScreenModelComponent で UI の層に描く）と名前を見せます。
    //! 続けて取れたときは順番待ちにします。オプションの Settings::showCutIn がオフなら出しません。遊びは止めません。
    class CutInSystem : public Tsukino::ECS::ISystem {
    public:

        //! 出すカットイン1つです。
        struct Request {
            int  fruitIndex   = -1;       // 果物の添字
            int  variantIndex = 0;        // バリエーションの添字
            bool registered   = false;    // 図鑑に初めて載ったか（見出しが「図鑑に登録！」になる）
        };

        //! コンストラクタです。
        //! @param  [in] eventBus PrizeDroppedEvent と ZukanRegisteredEvent を購読するイベントバス
        explicit CutInSystem(Tsukino::ECS::EventBus& eventBus);

        //! 順番待ちを進め、カットインの部品を動かします。
        //! @param  [in] registry  レジストリ
        //! @param  [in] deltaTime 前フレームからの経過時間（秒）
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;

    private:

        //! カットインを1つ始めます（果物の見た目を作り、文字を書く）。
        //! @param  [in] registry レジストリ
        //! @param  [in] request  出すカットイン
        //! @return 始められたら true（果物の定義が無いなどで出せなければ false）
        bool Start(Tsukino::ECS::Registry& registry, const Request& request);

        //! 今のカットインを終えます（果物の見た目を消す）。
        //! @param  [in] registry レジストリ
        void Finish(Tsukino::ECS::Registry& registry);

        Tsukino::ECS::ScopedConnection m_dropConnection;     // PrizeDroppedEvent の購読
        Tsukino::ECS::ScopedConnection m_zukanConnection;    // ZukanRegisteredEvent の購読
        std::vector<Request>           m_incoming;           // このフレームに届いた分（同じ果物はまとめる）
        std::deque<Request>            m_queue;              // 順番待ち
        bool                           m_playing = false;    // カットインを出しているか
        float                          m_time    = 0.0f;     // 出し始めてからの時間（秒）
        float                          m_spin    = 0.0f;     // 果物の回転（度）
        Tsukino::ECS::Entity           m_fruit   = entt::null;    // 果物を出している置き台（FruitIconComponent）
    };
}    // namespace FruitMagic::ECS

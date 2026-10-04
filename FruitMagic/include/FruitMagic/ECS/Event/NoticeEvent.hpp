//----------------------------------------------------------------------------
//! @file   NoticeEvent.hpp
//! @brief  画面上部に短いお知らせを出すイベント
//----------------------------------------------------------------------------
#pragma once
#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! 画面上部にお知らせ（「ジャックポット！」「果物がない…」など）を出すイベントです。HudSystem が表示します。
    //! 続けて届いたら新しい方に差し替わります。
    struct NoticeEvent {
        std::wstring text;              // 表示する文字
        float        seconds = 2.5f;    // 表示しておく時間（秒）
    };
}    // namespace FruitMagic::ECS

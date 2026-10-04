//----------------------------------------------------------------------------
//! @file   JackpotHoleComponent.hpp
//! @brief  ジャックポット穴の目印を表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! ジャックポット穴です。このコンポーネントを持つエンティティが穴の目印として、プッシャーの上面を
    //! プッシャーと一緒に前後し、左右にも往復します。大きさ・開閉の時間は JackpotConfig から読みます。
    struct JackpotHoleComponent {
        float moveTime = 0.0f;    // 左右の往復に使う経過時間（秒）
        float openTime = 0.0f;    // 開閉の周期の中の経過時間（秒）。閉じている時間 → 開いている時間の順
        bool  open     = false;   // 今開いているか
    };
}    // namespace FruitMagic::ECS

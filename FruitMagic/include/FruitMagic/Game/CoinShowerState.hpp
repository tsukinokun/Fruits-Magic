//----------------------------------------------------------------------------
//! @file   CoinShowerState.hpp
//! @brief  コインのシャワー（タダのコインを台の手前に降らせる）の依頼
//----------------------------------------------------------------------------
#pragma once
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! コインのシャワーの依頼です。魔法「メテオコイン」とジャックポットが積み、CoinShowerSystem が少しずつ降らせます。
    //! Registry のコンテキストに置きます。
    struct CoinShowerState {
        //! 1回分の依頼です。
        struct Request {
            int   remaining = 0;       // まだ降らせていない枚数
            float interval  = 0.1f;    // 1枚ごとの間隔（秒）
            float timer     = 0.0f;    // 次の1枚までの時間（秒）
        };

        std::vector<Request> requests;    // 降らせている途中の依頼

        //! 依頼を積みます。
        //! @param  [in] count   枚数
        //! @param  [in] seconds 全部を降らせ終えるまでの時間（秒）
        void Add(int count, float seconds) {
            if(count <= 0)
                return;
            Request r;
            r.remaining = count;
            r.interval  = (seconds > 0.0f) ? seconds / static_cast<float>(count) : 0.0f;
            requests.push_back(r);
        }
    };
}    // namespace FruitMagic

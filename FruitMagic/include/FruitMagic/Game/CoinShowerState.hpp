//----------------------------------------------------------------------------
//! @file   CoinShowerState.hpp
//! @brief  コインのシャワー（タダのコインを台に降らせる）の依頼
//----------------------------------------------------------------------------
#pragma once
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! コインを降らせる場所です。
    enum class ShowerPlace {
        Front,     // プッシャーが届かない台の手前側（魔法「メテオコイン」）
        Launch,    // 投入位置を動かせる範囲のどこか（ルーレット・ジャックポット。プレイヤーが入れたのと同じように落ちる）
    };

    //! コインのシャワーの依頼です。魔法「メテオコイン」・ジャックポット・ルーレットのコイン当たりが積み、
    //! CoinShowerSystem が少しずつ降らせます。Registry のコンテキストに置きます。
    struct CoinShowerState {
        //! 1回分の依頼です。
        struct Request {
            int         remaining = 0;                    // まだ降らせていない枚数
            float       interval  = 0.1f;                 // 1枚ごとの間隔（秒）
            float       timer     = 0.0f;                 // 次の1枚までの時間（秒）
            ShowerPlace place     = ShowerPlace::Front;    // 降らせる場所
        };

        std::vector<Request> requests;    // 降らせている途中の依頼

        //! 依頼を積みます。
        //! @param  [in] count   枚数
        //! @param  [in] seconds 全部を降らせ終えるまでの時間（秒）
        //! @param  [in] place   降らせる場所
        void Add(int count, float seconds, ShowerPlace place = ShowerPlace::Front) {
            if(count <= 0)
                return;
            Request r;
            r.remaining = count;
            r.interval  = (seconds > 0.0f) ? seconds / static_cast<float>(count) : 0.0f;
            r.place     = place;
            requests.push_back(r);
        }
    };
}    // namespace FruitMagic

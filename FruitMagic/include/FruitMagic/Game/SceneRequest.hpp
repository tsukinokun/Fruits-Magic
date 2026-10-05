//----------------------------------------------------------------------------
//! @file   SceneRequest.hpp
//! @brief  シーンそのものへの頼みごと（最初からやり直す・ゲームを終了する）
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! シーンへの頼みごとです。オプション画面（OptionsSystem）が立て、PusherScene::OnUpdate が
    //! そのフレームのシステムの更新が終わった後に実行します。Registry のコンテキストに置きます。
    struct SceneRequest {
        bool restart = false;    // セーブデータを消して最初からやり直す
        bool quit    = false;    // セーブしてゲームを終了する
    };
}    // namespace FruitMagic

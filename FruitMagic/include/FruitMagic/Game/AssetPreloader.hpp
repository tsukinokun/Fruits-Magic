//----------------------------------------------------------------------------
//! @file   AssetPreloader.hpp
//! @brief  アセットを裏スレッドで先読みするクラス
//! @detail ゲームで使うモデル・テクスチャ・効果音を裏スレッドで AssetManager に読み込みます。
//!         AssetManager は同じパスを2回目からキャッシュで返すため、先読みが済んでいれば
//!         シーンの初期化での Load は待たされません。
//----------------------------------------------------------------------------
#pragma once
#include <atomic>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

// 名前空間 : Tsukino::Asset
namespace Tsukino::Asset {
    class AssetManager;    // 前方宣言
}

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! アセットを裏スレッドで先読みするクラスです。
    //! - 読み込むのは AssetPaths のアセットと、定義データ（果物・効果音）に書かれたアセット
    //! - 破棄・Stop で読み込みを打ち切り、スレッドの終わりを待つ（1件の読み込みの途中では止まらない）
    class AssetPreloader {
    public:

        //! コンストラクタです。
        AssetPreloader() = default;

        //! デストラクタです。読み込み中なら打ち切って終わりを待ちます。
        ~AssetPreloader();

        AssetPreloader(const AssetPreloader&)            = delete;
        AssetPreloader& operator=(const AssetPreloader&) = delete;

        //! 裏スレッドで先読みを始めます。
        //! @param  [in] assetManager 読み込み先のアセットマネージャー（このオブジェクトより長く生きていること）
        //! @param  [in] dataRoot     定義データのフォルダ（Assets/Data）
        void Start(Tsukino::Asset::AssetManager& assetManager, const std::string& dataRoot);

        //! 読み込みを打ち切り、裏スレッドの終わりを待ちます。
        void Stop();

        //! 進み具合を返します。
        //! @return 0〜1。読み込む数が決まる前は 0
        float Progress() const;

        //! 先読みが終わったかを返します。
        //! @return 全て読み終えたら true（読めなかったものがあっても true）
        bool IsFinished() const { return m_finished.load(); }

    private:

        //! 読み込むアセットのパスを集めます。
        //! @param  [in] dataRoot 定義データのフォルダ
        //! @return リポジトリルート相対のパス（重複なし）
        static std::vector<std::string> CollectPaths(const std::string& dataRoot);

        //! 裏スレッドの本体です。
        //! @param  [in] stopToken    打ち切りの要求
        //! @param  [in] assetManager 読み込み先のアセットマネージャー
        //! @param  [in] dataRoot     定義データのフォルダ
        void Run(std::stop_token stopToken, Tsukino::Asset::AssetManager& assetManager, const std::string& dataRoot);

        std::atomic<int>  m_loaded{0};          // 読み終えた数
        std::atomic<int>  m_total{0};           // 読み込む数（0 はまだ決まっていない）
        std::atomic<bool> m_finished{false};    // 読み終えたか
        std::jthread      m_thread;             // 読み込みのスレッド。上のメンバを使うので最後に置き、最初に破棄する
    };
}    // namespace FruitMagic

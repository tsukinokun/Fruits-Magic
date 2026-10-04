# FruitMagic ロードマップ

企画は [GameDesign.md](GameDesign.md)。各マイルストーンは「遊べる状態」で終え、着手時に個別の実装計画を立てる。

| # | マイルストーン | 内容 | 完了の目安 |
|---|---|---|---|
| M1 ✅ | 投入できる | コイン手持ち、レーン選択＋投入、払い出し判定（落下口センサー）、所持枚数のHUD | コインを入れて増減する |
| M2 ✅ | 果物が出る | 果物定義 JSON（`Assets/Data/Fruits/*.json`）の読み込み、定義からの生成と出現抽選、チェッカー（払い出し口で動く穴）＋ルーレットによる補充、収穫判定、仮の色分け | JSON を1つ足すだけで新しい果物が出る |
| M3 ✅ | 魔法1つ | マナゲージ、溝落ちによるマナ獲得、魔法ボタン（画面下）＋数字キー、ゆらゆら魔法 | 能動的に崩せる |
| M4 ✅ | 図鑑 | 収穫記録、図鑑画面、色違い抽選 | 集める目的ができる |
| M5 ✅ | 台の強化 | 強化画面と4種の強化（プッシャーの押し幅・果樹・妖精の自動投入・チェッカーの穴）、果実（強化の通貨） | 台が育つ |
| M6 ✅ | セーブ/放置 | セーブ・ロード（自動セーブ＋終了時）、オフライン報酬と「おかえり」画面、おるすばん時間の強化 | 閉じても進む |
| M7 ✅ | 魔法拡張・ギミック | 魔法4つ追加（図鑑の登録数で解放）、ジャックポットチャンス、自動プレイでの計測とバランス調整 | 中盤の厚み |
| M8 | 見た目・演出 | 果物モデル、筐体、エフェクト、サウンド | ポップな見た目 |
| M9 | 転生 | 転生ボタン（中ランク収穫で解放）、星のたねの計算、リセット処理、永続強化の画面。M6 のセーブが前提 | 周回するほど速くなる |

## 技術メモ（M1〜で使うエンジン機能）
- 落下判定: `CollisionComponent.isSensor` の箱を落下口と側溝に置き、`CollisionEnterEvent`（EventBus）で検出
- HUD: `FontComponent`＋`FontRendererSystem`、画像は `SpriteRenderSystem`
- 入力: `InputSystem`（`IsKeyPressed` / `GetMousePosition`）
- データ: 果物・強化の定義は JSON（cereal）で外出し。果物定義は `Assets/Data/Fruits/*.json`、ランクは `Assets/Data/FruitRanks.json` に置き、起動時にフォルダを走査して読み込む
- 物理の単位: このゲームは 1unit=1cm。`PhysicsSystem::SetUnitsPerMeter(100)` で Jolt の重力と接触の許容値を cm に合わせている（`PusherScene::OnInitialize`）。これが無いと重力が 1/100 になり、コインが床やプッシャーにめり込んで下へ潜る
- 物理は1フレーム1ステップ。プッシャーの移動と物理には同じ（上限付きの）経過時間を渡す
- 投入位置は常にプッシャー上面の上（`PusherLayout.hpp` の `kLaunchZ` は背面パネルとプッシャー前面の中間から計算）
- 背面パネルはプッシャーの中へ食い込ませて隙間を作らない。隙間があると挟まれたコインが下をくぐって奥へ消える
- コリジョン表示: Debug ビルドは起動時からワイヤーフレーム表示、F5 で切り替え
- 果物の見た目: エンジンのモデルには描画時に色を変える手段が無いので、`PrizeFactory::GetTintedModel` でマテリアルの基本色だけを差し替えたモデルの複製をアセットとして登録して使う（メッシュは共有）
- モデルの大きさ: `MeshData::bounds` はノードの拡縮を掛けた後の値で、回転は掛かっていない（Block.fbx は Z-up）。描画時の大きさはノードの回転・移動だけを掛けて求める（`PrizeFactory.cpp` の `MeasureModelHalfExtent`）
- 開発用キー（Debug のみ）: F2 でコインと果実を +100・マナ満タン、F5 でコリジョン表示
- 画面スプライト（魔法ボタン・マナゲージ）: エンジンはメインでないカメラを画面スプライトの描画に使う。正射影の2Dカメラを置き、Debug のデバッグカメラより先に作る（後から作ったメインでないカメラが上書きするため）
- 魔法: 名前・コスト・キー・解放・数値は `Assets/Data/Magic.json`、効果の中身は id ごとのシステム（`shake` → `ShakeMagicSystem`）。マナの獲得量は `Assets/Data/Mana.json`
- 図鑑・バリエーション: バリエーション（通常・色違い・金色）と確率・価値の倍率・図鑑ボーナスは `Assets/Data/Collection.json`。果物やバリエーションを足すと図鑑の行・列も増える。Tab か右上のボタンで開閉
- 定義データの JSON は先頭の BOM を許す（`ReadDataText`）。メモ帳で保存しても読める
- 強化: 名前・説明・段階ごとの価格（コイン・果実）と効果の値は `Assets/Data/Upgrades.json`。値が何に効くかは id ごとに `UpgradeSystem::ApplyUpgrades` が決める（`pusherStroke` / `treeLevel` / `fairy` / `checkerWidth`）。計算結果は `TableStats`（果樹の段階だけ `GameState::treeLevel`）に入り、各システムが読む。レベルは `GameState::upgradeLevels`（id → レベル）に持つので、M6 のセーブはこれを保存して読み込み後に `ApplyUpgrades` を呼ぶ
- 押し幅: 最も引っ込んだ位置（`kPusherMinFrontZ`）は変えず、振幅が増えた分だけ前へ出す（投入位置が変わらないように）。プッシャーの奥行は振幅の上限 `kPusherMaxAmplitude` でも後端が背面パネルの奥に残る長さにしてある。振幅は 4cm/秒で少しずつ変える（一気に変えると景品を弾き飛ばす）
- セーブ: `Saves/save.json`（アセットルート基準。Debug はリポジトリ直下で、git には入れない）。`SaveData::Save/Load` が GameState を JSON で読み書きする。果物・バリエーション・強化は id で保存するので、定義データを足しても古いセーブが読める（消えた果物は警告して読み飛ばす）。台の上の景品は保存せず、毎回初期配置から始める。30 秒ごとの自動セーブ（`SaveSystem`）と終了時（`PusherScene::OnExit`）に保存し、一時ファイルに書いてから置き換える。壊れたセーブは `save.json.broken` に写してから新規で始める。セーブの形式を変えたら `version` を上げる
- オフライン報酬: 物理は回さず期待値で計算する（`GrantOfflineReward`）。妖精が入れたはずのコイン（留守の秒数 ÷ 妖精の間隔）× 戻る割合がコイン、× 果物の割合が収穫数で、果物は今の果樹の段階で抽選して価値を果実に足す（図鑑には載せない）。上限時間は「おるすばん時間」の強化、割合と最短時間・自動セーブ間隔は `Assets/Data/Offline.json`。テストではセーブの `savedAt` を過去にずらす
- 魔法の追加: `Magic.json` に1つ足し（`unlockZukan` で解放に必要な図鑑の登録数）、効果は id ごとのシステムを1つ書く（`SwellMagicSystem` などが見本。`MagicCatalog::IsMagic` で自分の魔法かを調べ、効果中は `MagicState::SetRemaining` で残り時間を書く）。違う魔法は同時に効果中になれる
- 台への影響: 魔法の効果は `MagicEffects`（プッシャーの振幅のボーナス）に書き、`PusherScene::OnUpdate` が強化の値と足して使う。振幅の上限は `kPusherMaxAmplitude`（24cm）。コインを降らせるのは `CoinShowerState` に依頼する（メテオコインとジャックポットで共用。台の上のコインが `Economy.json` の `maxCoinsOnTable` 以上なら待つ）
- 画面の左右: カメラは手前から奥を見ているので、画面の右はワールドの -X（`Layout::kScreenRightX`）。プレイヤーの入力を左右に対応させるときは必ずこれを掛ける
- ジャックポット: ルーレットを回すたびに `Jackpot.json` の `chanceRate` でチャンス、`winRate` で当たり。物理の穴は簡単に入りすぎたのでやめた（上面のコインは背面パネルに掻かれて必ず前端を通る）
- バランスの計測: 環境変数 `FRUITMAGIC_AUTOPLAY=1` で Release を起動すると、セーブを使わずに自動で遊び（`AutoPlaySystem`）、1分ごとに `BALANCE` 行をログへ出す（`BalanceProbeSystem`）。計測中はウィンドウを最小化して起動する（前に出ていると閉じられたりキー入力が入ったりする）。エンジンは入力を低レベルフックで読むので、`PostMessage` のキー・マウスは `IsKeyDown` やマウス位置には届かない
- 計測結果（M7 調整後、Release 10 分）: 払い出し率（払い出し ÷ 投入）は 72%→86%、手持ちは 15〜37 枚で推移、強化は 0:00 妖精・0:21 押し幅・3:14 チェッカー・7:17 果樹、収穫 36／溝落ち 20、ふくらむ解放 8:26、ジャックポットチャンス 6 回で当たり 1 回
- 画面（図鑑・強化）: 開閉と表示切替は `MenuSystem`（同時に開くのは1つ）。要素に `MenuPageComponent` を付けると開閉に合わせて表示され、中身は `ZukanSystem` / `UpgradeSystem` が書く。新しい画面は `MenuKind` を足し、`MenuButtonComponent` のボタンを置く

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
| M8 ✅ | 見た目・演出 | 球・箱を組み合わせた果物の飾りと屋台、落ちたときのポップ、光の粒と画面の光、合成した効果音 | ポップな見た目 |
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
- マテリアル（エンジン。Unity と同じ作り）
  - `.tmat`（Unity の `.mat`）は「キー = 値」で書くマテリアルのファイル（`ShadingModel`・`BaseColor`・`Metallic`・`Roughness`・`Specular`・`Emissive`・`AlphaCutoff`・`AlbedoMap` など。テクスチャはアセットのルートからのパス）。キャッシュは元ファイルから作り直されるので、`Cache/` を消しても残る
  - `ModelComponent::materials`（Unity の `MeshRenderer.sharedMaterials`）: スロットごとにマテリアルを差し替える。コインは `Table.json` の `coin.material`（`Assets/Materials/CoinGold.tmat`。金属）
  - `MaterialPropertyBlockComponent`（Unity の `MaterialPropertyBlock`）: エンティティごとに色などを上書きする。アセットは複製しない。果物・箱・球の色は `PrizeFactory::SetColor` がこれで付ける（基本色 = モデルの基本色 × 色。Block.fbx・Ball.fbx の基本色は 0.8）
- モデルの大きさ: `MeshData::bounds` はノードの拡縮を掛けた後の値で、回転は掛かっていない（Block.fbx は Z-up）。描画時の大きさはノードの回転・移動だけを掛けて求める（`PrizeFactory.cpp` の `MeasureModelHalfExtent`）
- 作り込んだモデル（Kenney）: 原点が底にあり大きさもまちまちなので、`PrizeFactory::AttachModelVisual` で子のエンティティに置き、外接の箱の中心を当たり判定の中心へずらし、当たり判定に収まる大きさへ拡大する（果物は縦横比を保つ。コインは軸ごと）。親（当たり判定）は拡縮しない。果物 JSON の `modelRotation`・`modelScale`・`tintBase`（false ならモデル自身の色のまま。色違い・金色は掛ける）。glb のテクスチャは `Textures/colormap.png` を指しているが、エンジンの取り込みはモデルの隣の同名ファイルを探すので、glb と同じフォルダに置く
- コインは見た目だけ丸く（`Table.json` の `coin.model`）、当たり判定は箱のまま。円柱の当たり判定を試したところ、プッシャーの前でコインが乗り上げて重なり、押す力が手前へ伝わらなかった（自動プレイで払い出し/投入 100% → 8%）
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
- 果物の飾り: 果物 JSON の `"parts"`（`shape` sphere/box・`offset`・`size`・`rotation`・`color`・`glow`）。位置と大きさは果物の半サイズに対する比率で、果物を親（`TransformComponent::parent`）にした見た目だけのエンティティ。親のスケールが「見た目の半サイズ ÷ モデルの半サイズ」なので、ローカルはモデルの半サイズで書く（`PrizeFactory::AttachParts`）。エンジンは親を消しても子を消さないので、景品は必ず `PrizeFactory::DestroyPrize` で消す
- 演出: 光の粒はワールド空間の加算スプライト（`SpriteSpace::World` はカメラを向く板。大きさは「画像のピクセル数 × スケール」cm）。粒の数・色・速さ・画面の光は `Assets/Data/Effects.json` のプリセットで、出したいシステムが `EffectEvent{プリセット名, 位置}` を発行する（魔法は `cast_<id>`、バリエーションは `variant_<id>` を自動で探す）。落ちたときのポップは `WorldAnchorComponent`（ワールドの一点に追従する画面の文字）
- カットイン: 図鑑に初めて載ったとき・色違いと金色が取れたときに、画面の中ほどの帯に果物の 3D モデルと名前を出す（`CutInSystem`。帯の大きさ・時間は `Ui.json` の `cutIn`、文言は `Texts.json` の `cutin`、オプションでオン/オフ）。3D モデルを UI の層に描くのはエンジンの `ScreenModelComponent`（付けたエンティティと子のモデルを、画面スプライト・文字と同じ層に `sortOrder` の順で描く。位置は画面ピクセル、大きさは `pixelsPerUnit`。影と点光源は効かない）。果物の見た目だけは `PrizeFactory::CreateFruitVisual`
- 画面の果物: 図鑑・左のパネル・「〇〇 ゲット！」・カットインの果物は、色の四角ではなく本物の 3D の果物（`Game/FruitIcon.hpp` の `CreateFruitIconHolder`・`SetFruitIcon`。中身が変わったときだけ作り直し、`FruitIconSystem` が回す）。図鑑のまだ取っていない枠は黒いシルエット。スクロールの中の果物は、エンジンの `ScreenModelComponent::anchor` に行の中の部品を指定して、行と一緒に動かし、枠で切り取る（重ね順の違うモデルどうしは奥行きを分けて描くので、互いに隠れない）
- 効果音: `Tools/GenerateSounds.py` で合成した WAV（`Assets/Sounds`）。エンジンが初回に XWB へ変換してキャッシュする。出来事と音の対応・音量・最短の間隔は `Assets/Data/Sounds.json`、鳴らすのは `SoundSystem`（M で消音）。光の画像は `Tools/GenerateTextures.py`
- BGM: `Sounds.json` の `"music"`。mp3 はエンジンが WAV 経由で XWB に変換する（重いのでロード画面で先に変換する。`SoundSystem::ListSoundFiles` が BGM も返す）。エンジンは再生中の音量を変えられないので、BGM の音量を変えたら少し待ってから止めて流し直す（曲は頭からになる。待ち時間は `Ui.json` の `timing.musicRestart`）
- 側壁と横の溝: 側壁は `TableLayout::SideWallFrontZ()`（手前端 − `openLength`）まで。プッシャーが最も前に出た位置（`PusherMaxFrontZ()`）より手前まで要る（`Validate` で確かめる）。落ちた場所は `PrizeDropSystem` が、落ちた時点で手前の端（z − fieldFrontZ）と横の端（|x| − fieldHalfWidth）のどちらをより大きく越えたかで決める（手前なら幅のどこでも取得、横なら溝）。`payoutHalfWidth` は判定に使わず、チェッカーとルーレットの補充の範囲だけ。魔法「かべ」は開いた所に沿った壁だけを立てる
- フォント: `UiFonts`（コンテキスト。ふつう・太字のハンドル）を `LoadingScene` で先に読み、`PusherScene` はキャッシュから取る。文字を作る所（HUD・魔法ボタン・`createText`・ポップ・ロード画面）で `FontComponent::fontHandle` に入れる。どれを太字にするかは `UiText` / `UiFont` の `bold`（`Ui.json`）。`.dfont` の `Size` はエンジンの標準フォントと同じ 32（変えると全部の文字の大きさが変わる）。エンジンは文字を実際に描く大きさでラスタライズする（グレースケールのアンチエイリアス）ので、`scale` を変えても文字はにじまない。文字は `FontComponent::maxWidth` で枠に収める（超えたらエンジンが縮めて描く）。ゲーム側は置く枠から幅を決めて渡す（`createText` / `createFontText` の最後の引数、余白は `Ui.json` の `outline.textPadding`。HUD は `hud.<種類>.maxWidth`、無ければ画面の端まで）。`.dfont` を書き換えたときは `Cache/Assets/Fonts/` の同名ファイルを消すと作り直される
- オプション: `MenuKind::Options`・`OptionsSystem`。設定（音量・消音・操作説明の表示）は `Settings`（`Saves/settings.json`。セーブとは別）。中身は `createScrollList` のスクロール領域に並べ、「データを消して最初から」は最初に見える範囲の下に置く。データ消去の確認ウィンドウは `OptionsDialogComponent`（MenuPageComponent は付けず OptionsSystem が表示を切り替える）で、何回目かは `OptionsState`。確認中は MenuSystem が Esc で確認だけ閉じ、ほかの開閉とスクロールを止める。「最初から」と「終了」は `SceneRequest` に立て、`PusherScene::OnUpdate` がシステムの更新後に実行する（最初から＝セーブを消して `ChangeScene`、終了＝`WM_CLOSE`）。シーンを変えるときは `OnExit` で音を全部止める（BGM が重ならないように）
- 画面（図鑑・強化）: 開閉と表示切替は `MenuSystem`（同時に開くのは1つ）。要素に `MenuPageComponent` を付けると開閉に合わせて表示され、中身は `ZukanSystem` / `UpgradeSystem` が書く。新しい画面は `MenuKind` を足し、`MenuButtonComponent` のボタンを置く

# FruitMagic — AI エージェント向けガイド

フルーツを景品にしたコインプッシャーゲーム。自作エンジン TsukinoEngine を
`External/TsukinoEngine` にサブモジュールとして取り込んでいる。

- 応答は日本語
- **エンジン（`External/TsukinoEngine/`）のファイルは変更しない。** 必要ならエンジン側リポジトリで直す
- エンジンの使い方・禁止事項は `External/TsukinoEngine/CLAUDE.md` を先に読む
  （特に `External/TsukinoEngine/External/` は読まない・grep しない）

## 企画

- `Docs/GameDesign.md` — 企画書（コンセプト・コアループ・果物・魔法・強化・放置）
- `Docs/Roadmap.md` — マイルストーン（M1〜M8）。新しい機能に着手するときはまずここを確認

## エンジン API の引き方

| ファイル | 内容 |
|---|---|
| `External/TsukinoEngine/Docs/api-index.md` | 全公開型 → ヘッダパスの索引。**まずここ** |
| `External/TsukinoEngine/Docs/api/<Module>.md` | モジュール単位の公開API |
| `External/TsukinoEngine/Docs/components.md` | 組み込みコンポーネントのフィールド一覧 |
| `External/TsukinoEngine/Tsukino.Sandbox/src/Scene/` | シーンの書き方の実例 |

## コーディング規約

`External/TsukinoEngine/CODING_GUIDELINES.md` に従う。コメント書式は
`tsukino-doc-comment` スキル（`External/TsukinoEngine/.claude/skills/`）に従う。
ゲーム側のコードは名前空間 `FruitMagic`、ヘッダは `FruitMagic/include/FruitMagic/...`、
ソースは `FruitMagic/src/...`。

## 構成

| パス | 内容 |
|---|---|
| `premake5.lua` | ワークスペース定義。エンジンの `tsukino_link()` 等を呼ぶだけ |
| `FruitMagic/src/WinMain.cpp` | エントリポイント |
| `FruitMagic/*/Scene/PusherScene.*` | プッシャー台のシーン |
| `Assets/` | ゲームのアセット。パスはリポジトリルート相対（例 `Assets/Models/Block.fbx`） |
| `Assets/Data/` | 果物・ランク・ルーレット・魔法・図鑑・強化・放置・ジャックポット・コインのやりくり（Economy）・演出（Effects）・効果音（Sounds）の定義 JSON。果物は `Fruits/<id>.json` を足すだけで増える（コード変更不要）。台の寸法と景品の物理（Table）・屋台の見た目と光とカメラ（Stage）・UI の配置と色と表示時間（Ui）・画面の文言（Texts。`{n}` などを差し込む）もここ。調整値はコードに直書きせず JSON に置く（読み込みは `Game/JsonReader.hpp`）。Table.json の寸法が矛盾していると Warn を出して既定の台に戻る |

| `Assets/Fonts/` | 画面の文字のフォント（M PLUS Rounded 1c の Medium・ExtraBold。SIL OFL 1.1、`OFL.txt` を一緒に置く）と、それを指す `.dfont`。どの文字を太字にするかは `Ui.json` の `"bold"`。元のフォントをそのまま入れてある（使う字だけに減らすと、字が増えるたびに作り直して git の履歴が増えるため） |
| `Assets/Models/Kenney/<キット>/` | Kenney の CC0 素材（果物・コイン・ちょうちん・電飾）。使う glb と、それが参照するテクスチャ `colormap.png`（キットごとに同じ名前なのでフォルダを分ける）、ライセンス文 `License.txt`。果物は JSON の `"model"`、コインは `Table.json` の `coin.model`、飾りは `Stage.json` の `props`・`lantern` で指定する |
| `Assets/Materials/` | マテリアルのファイル `.tmat`（エンジンの形式。「キー = 値」で書く）。モデルのマテリアルを差し替えるときに使う（例: 金属のコイン `CoinGold.tmat`） |
| `Assets/Sounds/`・`Assets/Textures/` | 効果音（WAV）と光の画像。どちらも `Tools/GenerateSounds.py`・`Tools/GenerateTextures.py` で合成したもの（作り直すときはスクリプトを直して実行） |
| `Saves/save.json` | セーブデータ（実行時に作られる。git 管理外）。消すと最初から |
| `Saves/settings.json` | オプションの設定（音量・消音・操作説明の表示）。セーブとは別なので「データを消して最初から」でも残る |

単位はエンジン規約どおり 1unit ≒ 1cm。

## ビルド・実行

```
build.bat            Debug をビルド（premake 再生成込み。成功時 0 行）
build.bat Release
open.bat             Visual Studio で開く
```

MSBuild を直接叩かない。実行ファイルは `bin/<Config>/FruitMagic.exe`。
Debug はリポジトリルートを作業ディレクトリにして起動する（アセットをルート相対で読むため）。

## バランスの計測

`FRUITMAGIC_AUTOPLAY=1` を付けて `bin/Release/FruitMagic.exe` を（作業ディレクトリを `bin/Release` にして、最小化で）起動すると、
セーブを使わずに自動で遊び、`bin/Release/Logs/FruitMagic.log` に1分ごとの `BALANCE` 行を出す。数値を変えたら回して比べる。

## ログ

`Logs/FruitMagic.log` に出る（WinMain で `Log::SetLogFile` 済み）。
実行結果の確認や「Prefab file not found」等の警告確認はこのファイルを見る。

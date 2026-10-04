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
| `Assets/Data/` | 果物・ランク・ルーレットの定義 JSON。果物は `Fruits/<id>.json` を足すだけで増える（コード変更不要） |

単位はエンジン規約どおり 1unit ≒ 1cm。

## ビルド・実行

```
build.bat            Debug をビルド（premake 再生成込み。成功時 0 行）
build.bat Release
open.bat             Visual Studio で開く
```

MSBuild を直接叩かない。実行ファイルは `bin/<Config>/FruitMagic.exe`。
Debug はリポジトリルートを作業ディレクトリにして起動する（アセットをルート相対で読むため）。

## ログ

`Logs/FruitMagic.log` に出る（WinMain で `Log::SetLogFile` 済み）。
実行結果の確認や「Prefab file not found」等の警告確認はこのファイルを見る。

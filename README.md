# FruitMagic

フルーツを景品にしたコインプッシャーゲームです。
エンジンには [TsukinoEngine](https://github.com/tsukinokun/TsukinoEngine)（C++20 / DirectX 11）を
サブモジュールとして使用しています。

## 必要要件

- Windows 10 / 11（x64）
- Visual Studio 2022（**C++ によるデスクトップ開発**ワークロード）
- Git

## セットアップ

```bash
git config --global core.longpaths true
git clone --recurse-submodules <このリポジトリのURL>
```

クローン済みの場合、またはサブモジュールを取り直したい場合は `setup.bat` を実行します
（サブモジュールの取得と Visual Studio ソリューションの生成を行います）。

```bat
setup.bat
```

## ビルドと実行

```bat
build.bat            :: Debug ビルド（成功時は何も出力しない）
build.bat Release    :: Release ビルド
open.bat             :: .build\FruitMagic.sln を生成して Visual Studio で開く
```

- 実行ファイルは `bin\Debug\FruitMagic.exe` / `bin\Release\FruitMagic.exe`
- Debug ビルドはリポジトリルートを作業ディレクトリとして起動してください
  （Visual Studio から実行する場合は自動でそう設定されます）
- Debug ビルドはエンジンのアセットを絶対パスで参照するため、ソリューションを生成したマシンでのみ動作します。
  Release ビルドは `bin\Release` 以下だけで動作します
- ログは `Logs\FruitMagic.log` に出力されます

ソースファイルを追加・削除した場合は `build.bat`（または `open.bat`）を実行すると
プロジェクトファイルが再生成されます。

## ディレクトリ構成

```
FruitMagic/
├─ Assets/                  ゲームのアセット
├─ FruitMagic/
│  ├─ include/FruitMagic/   ヘッダ
│  └─ src/                  ソース
├─ External/TsukinoEngine/  エンジン（サブモジュール）
└─ premake5.lua             ビルド定義
```

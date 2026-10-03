----------------------------------------
-- FruitMagic（フルーツを景品にしたコインプッシャー）のビルド定義
--
-- TsukinoEngine はサブモジュール（External/TsukinoEngine）として取り込み、
-- include・link・配布物の設定はエンジン側のヘルパーに任せる。
-- 書き写すと NDEBUG や JPH_DEBUG_RENDERER がエンジンと食い違っても気付けないため
----------------------------------------
include "External/TsukinoEngine/Tools/premake/tsukino.lua"

workspace "FruitMagic"
    startproject "FruitMagic"
    location ".build"
    tsukino_workspace_defaults()

-- エンジン本体（ゲーム側から include すると Sandbox は自動的にスキップされる）
include "External/TsukinoEngine"

----------------------------------------
-- ゲーム本体（実行ファイル）
----------------------------------------
project "FruitMagic"
    location ".build/FruitMagic"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"

    filter "action:vs*"
        buildoptions { "/permissive-" }
    filter {}

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    files {
        "FruitMagic/src/**.cpp",
        "FruitMagic/include/**.hpp",
    }

    includedirs { "FruitMagic/include" }

    tsukino_link()             -- エンジンの include・lib・NuGet
    tsukino_release_payload()  -- 組み込みアセット・ツール・ライセンス条文、実行時の作業ディレクトリ

    -- ゲーム自身のアセット。Debug はリポジトリルートを作業ディレクトリにして直接参照し、
    -- Release は exe の隣へコピーする
    filter "configurations:Release"
        postbuildcommands {
            "{COPYDIR} %{wks.location}/../Assets %{cfg.targetdir}/Assets",
        }
    filter {}

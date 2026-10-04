"""演出用の光の画像（Assets/Textures/Sparkle.png・Glow.png）を作るスクリプト。

外部ライブラリを使わず、標準ライブラリだけで PNG を書き出す。
画像を作り直したいときは、リポジトリのルートで次を実行する:

    python Tools/GenerateTextures.py

加算合成で使うので、色（RGB）も透明度と同じだけ暗くしてある（縁が黒くならないように）。
"""
import math
import os
import struct
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "Assets", "Textures")
SIZE = 64


def write_png(path, size, pixel):
    """pixel(x, y) -> (r, g, b, a)（0〜255）で RGBA の PNG を書く。"""
    raw = bytearray()
    for y in range(size):
        raw.append(0)  # 各行のフィルタ種別（なし）
        for x in range(size):
            raw.extend(pixel(x, y))

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def to_rgba(intensity):
    """明るさ（0〜1）を、加算合成向けの白い RGBA にする。"""
    v = max(0, min(255, int(round(intensity * 255))))
    return (v, v, v, v)


def glow(x, y):
    """ぼかした円。中心が明るく、縁へなめらかに消える。"""
    c = (SIZE - 1) / 2.0
    d = math.hypot(x - c, y - c) / c
    return to_rgba(max(0.0, 1.0 - d) ** 2)


def sparkle(x, y):
    """4本の光の筋を持つ星（キラッ）。中心の小さな円と、細く伸びる十字の筋を重ねる。"""
    c = (SIZE - 1) / 2.0
    dx, dy = (x - c) / c, (y - c) / c
    d = math.hypot(dx, dy)
    core = max(0.0, 1.0 - d * 3.0) ** 1.5
    # 十字の筋。軸からの距離が近いほど明るく、中心から離れるほど細く暗く
    ray_h = max(0.0, 1.0 - abs(dy) * 14.0) * max(0.0, 1.0 - abs(dx))
    ray_v = max(0.0, 1.0 - abs(dx) * 14.0) * max(0.0, 1.0 - abs(dy))
    return to_rgba(min(1.0, core + ray_h ** 1.2 + ray_v ** 1.2))


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    write_png(os.path.join(OUT_DIR, "Glow.png"), SIZE, glow)
    write_png(os.path.join(OUT_DIR, "Sparkle.png"), SIZE, sparkle)
    print("wrote Glow.png, Sparkle.png to", OUT_DIR)


if __name__ == "__main__":
    main()

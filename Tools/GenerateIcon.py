"""ゲームのアイコン（Assets/Icon/App.ico）を作るスクリプト。

外部ライブラリを使わず、標準ライブラリだけで ICO を書き出す。
アイコンを作り直したいときは、リポジトリのルートで次を実行する:

    python Tools/GenerateIcon.py

屋台の夜の色の角丸の板に、りんごと金色のコインを重ねた絵。
小さいサイズで線がつぶれないよう、サイズごとに 4 倍の解像度で描いてから縮める。
ICO の中身は各サイズの PNG（Windows Vista 以降はこれで読める）。
"""
import math
import os
import struct
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "Assets", "Icon")
SIZES = (16, 24, 32, 48, 64, 256)
SUPERSAMPLE = 4


def smooth_edge(d, px):
    """符号付き距離 d（内側が負）から、1 ピクセル幅でぼかした塗りの割合を返す。"""
    return max(0.0, min(1.0, 0.5 - d / px))


def rounded_box(x, y, cx, cy, half, radius):
    """角丸の正方形までの符号付き距離。"""
    qx = abs(x - cx) - (half - radius)
    qy = abs(y - cy) - (half - radius)
    outside = math.hypot(max(qx, 0.0), max(qy, 0.0))
    return outside + min(max(qx, qy), 0.0) - radius


def circle(x, y, cx, cy, r):
    return math.hypot(x - cx, y - cy) - r


def ellipse(x, y, cx, cy, rx, ry, angle):
    """回転した楕円までのおおよその符号付き距離（葉っぱ用）。"""
    c, s = math.cos(angle), math.sin(angle)
    lx = (x - cx) * c + (y - cy) * s
    ly = -(x - cx) * s + (y - cy) * c
    k = math.hypot(lx / rx, ly / ry)
    return (k - 1.0) * min(rx, ry)


def lerp(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def over(dst, color, alpha):
    """dst（r, g, b, a。0〜1）の上に color を alpha で重ねる。"""
    r, g, b, a = dst
    out_a = alpha + a * (1.0 - alpha)
    if out_a <= 0.0:
        return (0.0, 0.0, 0.0, 0.0)
    mix = lambda c, d: (c * alpha + d * a * (1.0 - alpha)) / out_a
    return (mix(color[0], r), mix(color[1], g), mix(color[2], b), out_a)


def shade(x, y, px):
    """座標 (x, y)（0〜1、左上原点）の色を返す。px は 1 ピクセルの幅。"""
    col = (0.0, 0.0, 0.0, 0.0)

    # 背景: 夜の屋台の色の角丸の板（上が明るい紫、下が濃い紺）
    d = rounded_box(x, y, 0.5, 0.5, 0.48, 0.12)
    a = smooth_edge(d, px)
    if a > 0.0:
        glow = max(0.0, 1.0 - math.hypot(x - 0.42, y - 0.40) / 0.6)
        base = lerp((0.10, 0.08, 0.25), (0.42, 0.20, 0.45), 1.0 - y)
        col = over(col, lerp(base, (1.0, 0.62, 0.30), glow * 0.45), a)

    # コイン（右下、りんごの後ろ）: 金の縁と内側の輪
    d = circle(x, y, 0.70, 0.71, 0.21)
    a = smooth_edge(d, px)
    if a > 0.0:
        t = max(0.0, min(1.0, (x - y + 0.6) / 1.2))
        col = over(col, lerp((0.75, 0.48, 0.08), (1.0, 0.86, 0.35), t), a)
        inner = smooth_edge(circle(x, y, 0.70, 0.71, 0.155), px)
        col = over(col, lerp((0.95, 0.70, 0.18), (1.0, 0.92, 0.55), t), inner)
        ring = smooth_edge(abs(circle(x, y, 0.70, 0.71, 0.155)) - 0.012, px)
        col = over(col, (0.70, 0.43, 0.06), ring * 0.8)

    # りんご: 2 つの円を重ねた形（上のくぼみ付き）
    body = min(circle(x, y, 0.37, 0.52, 0.25), circle(x, y, 0.53, 0.52, 0.25))
    body = max(body, -circle(x, y, 0.45, 0.24, 0.07))
    a = smooth_edge(body, px)
    if a > 0.0:
        light = max(0.0, 1.0 - math.hypot(x - 0.34, y - 0.40) / 0.42)
        col = over(col, lerp((0.62, 0.05, 0.10), (0.98, 0.25, 0.22), light), a)
        # ハイライト
        hl = smooth_edge(ellipse(x, y, 0.30, 0.42, 0.045, 0.085, -0.5), px)
        col = over(col, (1.0, 0.85, 0.80), hl * 0.75)

    # へた
    stem = max(abs(x - 0.455 - (0.22 - y) * 0.25) - 0.018, abs(y - 0.21) - 0.07)
    a = smooth_edge(stem, px)
    if a > 0.0:
        col = over(col, (0.35, 0.20, 0.10), a)

    # 葉っぱ
    d = ellipse(x, y, 0.57, 0.18, 0.10, 0.045, -0.45)
    a = smooth_edge(d, px)
    if a > 0.0:
        t = max(0.0, min(1.0, (0.20 - y) / 0.1 + 0.5))
        col = over(col, lerp((0.15, 0.55, 0.20), (0.45, 0.85, 0.30), t), a)

    return col


def render(size):
    """size × size の RGBA（0〜255）の画素列を返す。"""
    n = size * SUPERSAMPLE
    px = 1.0 / n
    pixels = bytearray()
    for y in range(size):
        for x in range(size):
            acc = [0.0, 0.0, 0.0, 0.0]
            for sy in range(SUPERSAMPLE):
                for sx in range(SUPERSAMPLE):
                    fx = (x * SUPERSAMPLE + sx + 0.5) * px
                    fy = (y * SUPERSAMPLE + sy + 0.5) * px
                    r, g, b, a = shade(fx, fy, px)
                    # 縮めるときは透明度を掛けた色で平均する（縁が黒くならないように）
                    acc[0] += r * a
                    acc[1] += g * a
                    acc[2] += b * a
                    acc[3] += a
            count = SUPERSAMPLE * SUPERSAMPLE
            a = acc[3] / count
            if a > 0.0:
                rgb = [acc[i] / acc[3] for i in range(3)]
            else:
                rgb = [0.0, 0.0, 0.0]
            pixels.extend(int(round(max(0.0, min(1.0, v)) * 255)) for v in (*rgb, a))
    return pixels


def encode_png(size, pixels):
    raw = bytearray()
    stride = size * 4
    for y in range(size):
        raw.append(0)  # 各行のフィルタ種別（なし）
        raw.extend(pixels[y * stride:(y + 1) * stride])

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    return png


def write_ico(path, images):
    """images: [(size, png バイト列)] から ICO を書く。"""
    header = struct.pack("<HHH", 0, 1, len(images))
    offset = 6 + 16 * len(images)
    entries = b""
    data = b""
    for size, png in images:
        dim = 0 if size >= 256 else size  # 256 は 0 と書く決まり
        entries += struct.pack("<BBBBHHII", dim, dim, 0, 0, 1, 32, len(png), offset + len(data))
        data += png
    with open(path, "wb") as f:
        f.write(header + entries + data)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    images = [(size, encode_png(size, render(size))) for size in SIZES]
    path = os.path.join(OUT_DIR, "App.ico")
    write_ico(path, images)
    print("wrote", path)


if __name__ == "__main__":
    main()

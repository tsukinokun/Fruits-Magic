"""演出用の光の画像（Assets/Textures/Sparkle.png・Glow.png）と、UI のリング（Ring.png）、
お金のマーク（CoinIcon.png・FpIcon.png）を作るスクリプト。

外部ライブラリを使わず、標準ライブラリだけで PNG を書き出す。
画像を作り直したいときは、リポジトリのルートで次を実行する:

    python Tools/GenerateTextures.py

光の画像は加算合成で使うので、色（RGB）も透明度と同じだけ暗くしてある（縁が黒くならないように）。
リングは通常の半透明合成で使うので、色は白のまま透明度だけで形を作る（色は SpriteComponent::tintColor で付ける）。
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


# リングの外側・内側の半径（中心から縁までを 1 とした割合）。太さは1辺の約18%
RING_OUTER = 0.97
RING_INNER = 0.61


def ring(x, y):
    """白いリング（円形のゲージの枠と中身に使う）。縁は1ピクセル幅でなめらかにする。"""
    c = (SIZE - 1) / 2.0
    r = math.hypot(x - c, y - c)
    # 外側・内側の縁からの距離（ピクセル。内側が正）
    coverage = min(RING_OUTER * c - r, r - RING_INNER * c) + 0.5
    a = max(0, min(255, int(round(max(0.0, min(1.0, coverage)) * 255))))
    return (255, 255, 255, a)


# ---------------------------------------------------------------------------
# お金のマーク（コイン・FP）。HUD で数の左に出す。通常の半透明合成で使うので色はそのまま入れる。
# 形は「中心を (0,0)、縁までを 1」の座標で、点ごとに上の層から色を決め、1ピクセルを4×4で平均してなめらかにする
# ---------------------------------------------------------------------------
ICON_SIZE = 128
ICON_SAMPLES = 4


def write_icon(path, shade):
    """shade(u, v) -> (r, g, b, a)（0〜1）の絵を、なめらかにして PNG に書く。v は下向き。"""
    def pixel(x, y):
        acc = [0.0, 0.0, 0.0, 0.0]
        for sy in range(ICON_SAMPLES):
            for sx in range(ICON_SAMPLES):
                u = ((x + (sx + 0.5) / ICON_SAMPLES) / ICON_SIZE) * 2.0 - 1.0
                v = ((y + (sy + 0.5) / ICON_SAMPLES) / ICON_SIZE) * 2.0 - 1.0
                r, g, b, a = shade(u, v)
                acc[0] += r * a
                acc[1] += g * a
                acc[2] += b * a
                acc[3] += a
        n = ICON_SAMPLES * ICON_SAMPLES
        a = acc[3] / n
        if a <= 0.0:
            return (0, 0, 0, 0)
        # 色は透明度で割り戻す（縁の色が黒く濁らないように）
        return tuple(max(0, min(255, int(round(c / acc[3] * 255)))) for c in acc[:3]) + (max(0, min(255, int(round(a * 255)))),)
    write_png(path, ICON_SIZE, pixel)


def rgb(r, g, b):
    return (r / 255.0, g / 255.0, b / 255.0)


def star_contains(u, v, radius, inner_ratio=0.45):
    """中心の5つの角の星の中か。"""
    angle = math.atan2(u, -v)    # 真上を 0 にする
    r = math.hypot(u, v)
    sector = (2.0 * math.pi) / 10.0
    i = math.floor(angle / sector)
    # 角（外側）と谷（内側）を交互に結んだ辺までの距離で判定する
    r0 = radius if i % 2 == 0 else radius * inner_ratio
    r1 = radius * inner_ratio if i % 2 == 0 else radius
    a0, a1 = i * sector, (i + 1) * sector
    x0, y0 = r0 * math.sin(a0), r0 * math.cos(a0)
    x1, y1 = r1 * math.sin(a1), r1 * math.cos(a1)
    px, py = r * math.sin(angle), r * math.cos(angle)
    # 原点と辺の同じ側にあれば中
    cross_edge = (x1 - x0) * (py - y0) - (y1 - y0) * (px - x0)
    cross_origin = (x1 - x0) * (0 - y0) - (y1 - y0) * (0 - x0)
    return cross_edge * cross_origin >= 0.0


def coin_icon(u, v):
    """金色のコイン。濃い縁・内側の輪・中央の星・左上の光の筋。"""
    r = math.hypot(u, v)
    if r > 0.96:
        return (0.0, 0.0, 0.0, 0.0)
    if r > 0.82:
        return rgb(170, 105, 20) + (1.0,)    # 縁
    if 0.64 < r < 0.71:
        return rgb(205, 140, 30) + (1.0,)    # 内側の輪
    if star_contains(u, v, 0.42):
        return rgb(255, 236, 150) + (1.0,)   # 星
    # 面。左上ほど明るい
    light = 0.5 - 0.35 * (u + v) / 1.414
    base = (1.0, 0.72 + 0.12 * light, 0.18 + 0.2 * light)
    # 光の筋（左上の斜めの帯）
    if 0.30 < (-u - v) / 1.414 < 0.42 and r < 0.8:
        base = rgb(255, 245, 200)
    return base + (1.0,)


def fp_icon(u, v):
    """フルーツポイント。ピンクの丸に、白い果物（丸い実・葉・茎）。"""
    r = math.hypot(u, v)
    if r > 0.96:
        return (0.0, 0.0, 0.0, 0.0)
    if r > 0.82:
        return rgb(190, 45, 105) + (1.0,)    # 縁
    # 葉（右上に傾けた楕円）
    lu, lv = u - 0.2, v + 0.42
    ca, sa = math.cos(-0.6), math.sin(-0.6)
    eu, ev = lu * ca - lv * sa, lu * sa + lv * ca
    if (eu / 0.22) ** 2 + (ev / 0.1) ** 2 <= 1.0:
        return rgb(120, 205, 95) + (1.0,)
    # 茎
    if abs(u + 0.02) < 0.045 and -0.5 < v < -0.25:
        return rgb(120, 70, 40) + (1.0,)
    # 実（少し下に置いた丸。上側を少しへこませてりんごのように）
    fu, fv = u, v - 0.1
    dent = 0.08 * math.exp(-(fu / 0.12) ** 2) if fv < 0 else 0.0
    if math.hypot(fu, fv) <= 0.45 - dent:
        if math.hypot(fu + 0.15, fv + 0.15) < 0.1:
            return rgb(255, 210, 225) + (1.0,)    # つや
        return rgb(255, 250, 248) + (1.0,)
    # 面。上ほど明るいピンク
    light = 0.5 - 0.4 * v
    return (0.92 + 0.06 * light, 0.38 + 0.12 * light, 0.58 + 0.1 * light, 1.0)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    write_png(os.path.join(OUT_DIR, "Glow.png"), SIZE, glow)
    write_png(os.path.join(OUT_DIR, "Sparkle.png"), SIZE, sparkle)
    write_png(os.path.join(OUT_DIR, "Ring.png"), SIZE, ring)
    write_icon(os.path.join(OUT_DIR, "CoinIcon.png"), coin_icon)
    write_icon(os.path.join(OUT_DIR, "FpIcon.png"), fp_icon)
    print("wrote Glow.png, Sparkle.png, Ring.png, CoinIcon.png, FpIcon.png to", OUT_DIR)


if __name__ == "__main__":
    main()

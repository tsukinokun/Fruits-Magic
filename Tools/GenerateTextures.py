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


# ---------------------------------------------------------------------------
# 屋台の UI の部品（木の板・紙・黒板・カプセル・丸ボタン・リボン・値札・栞・画びょう・吊り紐・印）。
# 板は 9 スライス（SpriteComponent::sliceBorder）で使うので、四辺の帯（SLICE_*）の中だけに角や縁を描き、
# 真ん中は伸ばしても目立たない模様にする。色は実際の色で入れる（tintColor は白のまま。色を変えたいときだけ掛ける）。
# 座標はピクセル（左上原点、y は下向き）
# ---------------------------------------------------------------------------
UI_DIR = os.path.join(OUT_DIR, "UI")
UI_SAMPLES = 3


def write_png_rect(path, width, height, pixel):
    """pixel(x, y) -> (r, g, b, a)（0〜255）で幅×高さの RGBA の PNG を書く。"""
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        for x in range(width):
            raw.extend(pixel(x, y))

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def write_shape(name, width, height, shade):
    """shade(x, y) -> (r, g, b, a)（0〜1。x, y はピクセルの中の点）を、なめらかにして UI/<name>.png に書く。"""
    def pixel(px, py):
        acc = [0.0, 0.0, 0.0, 0.0]
        for sy in range(UI_SAMPLES):
            for sx in range(UI_SAMPLES):
                r, g, b, a = shade(px + (sx + 0.5) / UI_SAMPLES, py + (sy + 0.5) / UI_SAMPLES)
                acc[0] += r * a
                acc[1] += g * a
                acc[2] += b * a
                acc[3] += a
        a = acc[3] / (UI_SAMPLES * UI_SAMPLES)
        if a <= 0.0:
            return (0, 0, 0, 0)
        return tuple(max(0, min(255, int(round(c / acc[3] * 255)))) for c in acc[:3]) + (max(0, min(255, int(round(a * 255)))),)
    write_png_rect(os.path.join(UI_DIR, name + ".png"), width, height, pixel)


CLEAR = (0.0, 0.0, 0.0, 0.0)


def round_rect_distance(x, y, w, h, radius):
    """角丸の四角の縁からの距離（内側が正）。"""
    qx = min(x, w - x)
    qy = min(y, h - y)
    if qx < radius and qy < radius:
        return radius - math.hypot(radius - qx, radius - qy)
    return min(qx, qy)


def mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(len(a)))


def wood_color(x, y, base):
    """木目（横向きの筋。真ん中を伸ばしても目立たない程度にゆるやかにする）。"""
    grain = 0.5 + 0.5 * math.sin(y * 0.55 + 2.2 * math.sin(x * 0.045 + y * 0.11))
    ring = 0.5 + 0.5 * math.sin(y * 1.7 + math.sin(x * 0.02) * 4.0)
    k = 0.9 + 0.07 * grain + 0.03 * ring
    return tuple(min(1.0, c * k) for c in base)


WOOD_LIGHT = rgb(214, 162, 104)
WOOD_DARK = rgb(120, 74, 38)
WOOD_EDGE = rgb(92, 54, 26)
PAPER = rgb(253, 246, 228)
PAPER_EDGE = rgb(214, 196, 160)
BOARD = rgb(46, 74, 60)


def wood_panel(x, y, w=128, h=128, radius=22, frame=10):
    """木の板（濃い縁＋明るい木目の面）。9 スライスの帯は 32px。"""
    d = round_rect_distance(x, y, w, h, radius)
    if d <= 0.0:
        return CLEAR
    if d < 3.0:
        return WOOD_EDGE + (1.0,)
    if d < frame:
        # 縁は面より暗く、上側だけ少し明るい（光が上から当たって見える）
        return wood_color(x, y, mix(WOOD_DARK, WOOD_LIGHT, 0.25 if y < h * 0.5 else 0.0)) + (1.0,)
    return wood_color(x, y, WOOD_LIGHT) + (1.0,)


def paper_panel(x, y, w=128, h=128):
    """紙（角が少し丸く、縁がうっすら焼けている）。9 スライスの帯は 24px。"""
    d = round_rect_distance(x, y, w, h, 6)
    if d <= 0.0:
        return CLEAR
    t = max(0.0, 1.0 - d / 10.0)
    return mix(PAPER, PAPER_EDGE, t * 0.8) + (1.0,)


def board_panel(x, y, w=128, h=128):
    """黒板（木の枠＋深い緑の面。面にはうっすらチョークの跡）。9 スライスの帯は 32px。"""
    d = round_rect_distance(x, y, w, h, 10)
    if d <= 0.0:
        return CLEAR
    if d < 2.5:
        return WOOD_EDGE + (1.0,)
    if d < 16.0:
        return wood_color(x, y, mix(WOOD_DARK, WOOD_LIGHT, 0.45)) + (1.0,)
    if d < 18.0:
        return rgb(30, 46, 38) + (1.0,)
    chalk = 0.03 * (0.5 + 0.5 * math.sin(x * 0.21 + y * 0.07) * math.sin(y * 0.17))
    return tuple(min(1.0, c + chalk) for c in BOARD) + (1.0,)


def capsule(x, y, w=128, h=64):
    """カプセル（両端が丸い木の札）。9 スライスの帯は左右 32px・上下 0px（高さはそのまま）。"""
    r = h * 0.5
    cx = min(max(x, r), w - r)
    d = r - math.hypot(x - cx, y - r)
    if d <= 0.0:
        return CLEAR
    if d < 3.0:
        return WOOD_EDGE + (1.0,)
    if d < 7.0:
        return wood_color(x, y, WOOD_DARK) + (1.0,)
    return wood_color(x, y, WOOD_LIGHT) + (1.0,)


def round_button(x, y, size=128):
    """丸ボタン（白〜灰。tintColor で色を付ける）。濃い縁・明るい面・上の光。"""
    c = size * 0.5
    r = math.hypot(x - c, y - c)
    outer = c - 2.0
    if r > outer:
        return CLEAR
    if r > outer - 6.0:
        return (0.42, 0.42, 0.42, 1.0)
    shine = max(0.0, 1.0 - math.hypot(x - c * 0.75, y - c * 0.6) / (c * 0.55))
    v = 0.86 + 0.12 * shine - 0.1 * max(0.0, (y - c) / c)
    return (v, v, v, 1.0)


def pill(x, y, w=128, h=64):
    """カプセルのボタン（白〜灰。tintColor で色を付ける）。濃い縁・明るい面・上の光。9 スライスの帯は 32px。"""
    r = h * 0.5
    cx = min(max(x, r), w - r)
    d = r - math.hypot(x - cx, y - r)
    if d <= 0.0:
        return CLEAR
    if d < 4.0:
        return (0.42, 0.42, 0.42, 1.0)
    v = 0.92 - 0.12 * (y / h) + (0.06 if y < h * 0.35 else 0.0)
    return (v, v, v, 1.0)


def alert_badge(x, y, size=64):
    """「!」の印（赤い丸に白い「!」）。"""
    c = size * 0.5
    r = math.hypot(x - c, y - c)
    if r > c - 1.0:
        return CLEAR
    if r > c - 5.0:
        return (1.0, 1.0, 1.0, 1.0)
    bar = abs(x - c) < 4.5 and 14.0 < y < 38.0
    dot = math.hypot(x - c, y - 46.0) < 4.8
    if bar or dot:
        return (1.0, 1.0, 1.0, 1.0)
    return rgb(226, 62, 62) + (1.0,)


def pushpin(x, y, size=32):
    """画びょう（赤い頭と光）。"""
    c = size * 0.5
    r = math.hypot(x - c, y - c)
    if r > c - 1.0:
        return CLEAR
    if math.hypot(x - c * 0.8, y - c * 0.75) < c * 0.28:
        return rgb(255, 190, 190) + (1.0,)
    if r > c - 4.0:
        return rgb(150, 30, 40) + (1.0,)
    return rgb(214, 52, 64) + (1.0,)


def rope(x, y, w=16, h=64):
    """吊り紐（ねじった縄。縦に繰り返しても継ぎ目が出ない周期）。"""
    if abs(x - w * 0.5) > 3.5:
        return CLEAR
    twist = 0.5 + 0.5 * math.sin((y + (x - w * 0.5) * 1.6) * (2.0 * math.pi / 8.0))
    return mix(rgb(120, 86, 50), rgb(196, 160, 110), twist) + (1.0,)


def ribbon(x, y, w=256, h=64):
    """リボン（両端に切れ込み。白〜灰で作り tintColor で色を付ける）。9 スライスの帯は左右 48px。"""
    notch = 18.0
    body_top, body_bottom = 8.0, h - 8.0
    if y < body_top or y > body_bottom:
        return CLEAR
    mid = h * 0.5
    # 両端の V 字の切れ込み
    depth = notch * (1.0 - abs(y - mid) / (mid - body_top))
    if x < depth or x > w - depth:
        return CLEAR
    if y < body_top + 3.0 or y > body_bottom - 3.0:
        return (0.62, 0.62, 0.62, 1.0)
    fold = 0.06 if (x < 40.0 or x > w - 40.0) else 0.0    # 両端は折り返しで少し暗い
    v = 0.95 - fold - 0.08 * ((y - body_top) / (body_bottom - body_top))
    return (v, v, v, 1.0)


def price_tag(x, y, w=128, h=64):
    """値札（左が尖り、穴が開いている）。9 スライスの帯は左 40px・右 16px・上下 16px。"""
    point = 26.0
    mid = h * 0.5
    if x < point:
        if abs(y - mid) > (x / point) * mid:
            return CLEAR
    d = round_rect_distance(max(x, point), y, w, h, 8) if x >= point else min(y, h - y) - abs(y - mid) * 0.0
    if d <= 0.0 and x >= point:
        return CLEAR
    if math.hypot(x - 18.0, y - mid) < 5.0:
        return CLEAR    # 紐を通す穴
    if math.hypot(x - 18.0, y - mid) < 7.0:
        return rgb(176, 150, 110) + (1.0,)
    edge = (x >= point and d < 2.5) or (x < point and abs(abs(y - mid) - (x / point) * mid) < 2.5)
    if edge:
        return rgb(176, 150, 110) + (1.0,)
    return rgb(250, 238, 206) + (1.0,)


def bookmark_tab(x, y, w=64, h=96):
    """栞のタブ（上の角が丸く、下はまっすぐ。白〜灰で作り tintColor で色を付ける）。9 スライスの帯は 20px。"""
    d = round_rect_distance(x, y, w, h + 30.0, 14)    # 下側の角は丸めない（高さを足して下にはみ出させる）
    if d <= 0.0 or y > h:
        return CLEAR
    if d < 3.0:
        return (0.6, 0.6, 0.6, 1.0)
    v = 0.95 - 0.08 * (y / h)
    return (v, v, v, 1.0)


def close_button(x, y, size=64):
    """閉じるボタン（丸に「×」。白〜灰で作り tintColor で色を付ける）。"""
    c = size * 0.5
    r = math.hypot(x - c, y - c)
    if r > c - 1.0:
        return CLEAR
    if r > c - 5.0:
        return (0.4, 0.4, 0.4, 1.0)
    dx, dy = x - c, y - c
    if abs(dx) < 15.0 and abs(dy) < 15.0 and (abs(dx - dy) < 4.5 or abs(dx + dy) < 4.5):
        return (1.0, 1.0, 1.0, 1.0)
    return (0.75, 0.75, 0.75, 1.0)


def bubble_tail(x, y, w=48, h=40):
    """吹き出しのしっぽ（下向きの三角。紙の色）。吹き出しの本体は紙の板を使う。"""
    t = y / h
    half = (1.0 - t) * w * 0.5
    cx = w * 0.5 - t * w * 0.25    # 少し左へ曲がる
    if abs(x - cx) > half:
        return CLEAR
    if abs(x - cx) > half - 2.5:
        return PAPER_EDGE + (1.0,)
    return PAPER + (1.0,)


def ellipse_distance(x, y, cx, cy, rx, ry):
    """楕円の縁からのおおよその距離（ピクセル。内側が正）。"""
    k = math.hypot((x - cx) / rx, (y - cy) / ry)
    return (1.0 - k) * min(rx, ry)


def fairy(x, y, size=96):
    """妖精（丸い顔・葉っぱの服・透けた羽）。右パネルのおすすめの吹き出しの横に置く。手前の部品から順に見る。"""
    s = size / 96.0
    outline = rgb(92, 54, 60)

    def layer(d, color, alpha=1.0, edge=2.2):
        """縁からの距離 d の部品を塗る（縁は濃い線）。外なら None。"""
        if d <= 0.0:
            return None
        return (outline + (1.0,)) if d < edge * s else (color + (alpha,))

    # 顔の中（目・ほっぺ・口）
    head_cx, head_cy, head_r = 48.0 * s, 40.0 * s, 19.0 * s
    for ex in (-7.0, 7.0):
        if math.hypot(x - (head_cx + ex * s), y - (head_cy + 2.0 * s)) < 2.8 * s:
            return outline + (1.0,)
    for ex in (-12.0, 12.0):
        if ellipse_distance(x, y, head_cx + ex * s, head_cy + 8.0 * s, 4.0 * s, 2.6 * s) > 0.0:
            return rgb(255, 160, 170) + (1.0,)
    mouth = math.hypot(x - head_cx, y - (head_cy + 6.0 * s))
    if abs(mouth - 4.0 * s) < 1.1 * s and y > head_cy + 7.5 * s:
        return outline + (1.0,)

    # 髪（顔の上半分にかぶせる）と、頭の上の葉っぱ
    hair = ellipse_distance(x, y, head_cx, head_cy - 6.0 * s, head_r + 2.0 * s, head_r * 0.8)
    if y < head_cy - 3.0 * s or (abs(x - head_cx) > head_r * 0.82 and y < head_cy + 8.0 * s):
        part = layer(hair, rgb(255, 176, 120))
        if part:
            return part
    leaf = ellipse_distance(x, y, head_cx + 6.0 * s, head_cy - head_r - 5.0 * s, 8.0 * s, 4.5 * s)
    part = layer(leaf, rgb(118, 196, 96))
    if part:
        return part

    # 顔
    part = layer(head_r - math.hypot(x - head_cx, y - head_cy), rgb(255, 228, 206))
    if part:
        return part

    # 葉っぱの服（下に広がる台形）
    top, bottom = head_cy + head_r - 4.0 * s, 88.0 * s
    if top < y < bottom:
        t = (y - top) / (bottom - top)
        half = (8.0 + 16.0 * t) * s
        d = min(half - abs(x - head_cx), y - top, bottom - y)
        part = layer(d, mix(rgb(132, 210, 110), rgb(84, 160, 80), t))
        if part:
            return part

    # 羽（左右に2枚ずつ。透けて見えるよう少し薄く）
    for side in (-1.0, 1.0):
        for cx, cy, rx, ry in ((30.0, 52.0, 17.0, 11.0), (34.0, 70.0, 11.0, 8.0)):
            d = ellipse_distance(x, y, head_cx + side * cx * s, cy * s, rx * s, ry * s)
            part = layer(d, rgb(196, 236, 255), 0.85, edge=1.8)
            if part:
                return part
    return CLEAR


def generate_ui():
    os.makedirs(UI_DIR, exist_ok=True)
    write_shape("WoodPanel", 128, 128, lambda x, y: wood_panel(x, y))
    write_shape("PaperPanel", 128, 128, lambda x, y: paper_panel(x, y))
    write_shape("BoardPanel", 128, 128, lambda x, y: board_panel(x, y))
    write_shape("Capsule", 128, 64, lambda x, y: capsule(x, y))
    write_shape("RoundButton", 128, 128, lambda x, y: round_button(x, y))
    write_shape("Pill", 128, 64, lambda x, y: pill(x, y))
    write_shape("AlertBadge", 64, 64, lambda x, y: alert_badge(x, y))
    write_shape("Pushpin", 32, 32, lambda x, y: pushpin(x, y))
    write_shape("Rope", 16, 64, lambda x, y: rope(x, y))
    write_shape("Ribbon", 256, 64, lambda x, y: ribbon(x, y))
    write_shape("PriceTag", 128, 64, lambda x, y: price_tag(x, y))
    write_shape("BookmarkTab", 64, 96, lambda x, y: bookmark_tab(x, y))
    write_shape("CloseButton", 64, 64, lambda x, y: close_button(x, y))
    write_shape("BubbleTail", 48, 40, lambda x, y: bubble_tail(x, y))
    write_shape("Fairy", 96, 96, lambda x, y: fairy(x, y))
    print("wrote UI parts to", UI_DIR)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    generate_ui()
    write_png(os.path.join(OUT_DIR, "Glow.png"), SIZE, glow)
    write_png(os.path.join(OUT_DIR, "Sparkle.png"), SIZE, sparkle)
    write_png(os.path.join(OUT_DIR, "Ring.png"), SIZE, ring)
    write_icon(os.path.join(OUT_DIR, "CoinIcon.png"), coin_icon)
    write_icon(os.path.join(OUT_DIR, "FpIcon.png"), fp_icon)
    print("wrote Glow.png, Sparkle.png, Ring.png, CoinIcon.png, FpIcon.png to", OUT_DIR)


if __name__ == "__main__":
    main()

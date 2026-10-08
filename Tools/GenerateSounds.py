"""効果音（Assets/Sounds/*.wav）を合成するスクリプト。

外部の素材やライブラリを使わず、正弦波・三角波・矩形波・ノイズと音量の包絡線を組み合わせて作る。
音を作り直したいときは、リポジトリのルートで次を実行する:

    python Tools/GenerateSounds.py

どの出来事でどの音を鳴らすか・音量は Assets/Data/Sounds.json で決める。
"""
import math
import os
import random
import struct
import wave

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "Assets", "Sounds")
RATE = 44100

random.seed(20261005)  # 作り直しても同じ音になるように


# ---------------------------------------------------------------------------
# 波形と包絡線
# ---------------------------------------------------------------------------
def sine(phase):
    return math.sin(2.0 * math.pi * phase)


def triangle(phase):
    p = phase % 1.0
    return 4.0 * p - 1.0 if p < 0.5 else 3.0 - 4.0 * p


def square(phase):
    return 1.0 if (phase % 1.0) < 0.5 else -1.0


def note(name):
    """"C6" のような音名を周波数にする（A4 = 440Hz）。"""
    names = {"C": -9, "D": -7, "E": -5, "F": -4, "G": -2, "A": 0, "B": 2}
    semitone = names[name[0]]
    rest = name[1:]
    if rest.startswith("#"):
        semitone += 1
        rest = rest[1:]
    octave = int(rest)
    return 440.0 * 2.0 ** ((semitone + (octave - 4) * 12) / 12.0)


def tone(freq_start, duration, wave_fn=sine, freq_end=None, attack=0.005, decay=None, volume=1.0, vibrato=0.0):
    """1つの音。周波数は始め→終わりへ指数的に変わる。decay を渡すと指数的に減衰、無ければ最後まで保ってから短く消える。"""
    freq_end = freq_end if freq_end is not None else freq_start
    n = int(duration * RATE)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        k = i / max(1, n - 1)
        freq = freq_start * (freq_end / freq_start) ** k
        if vibrato:
            freq *= 1.0 + vibrato * math.sin(2.0 * math.pi * 7.0 * t)
        phase += freq / RATE
        env = min(1.0, t / attack) if attack > 0 else 1.0
        if decay:
            env *= math.exp(-t / decay)
        else:
            env *= min(1.0, (duration - t) / 0.02)
        out.append(wave_fn(phase) * env * volume)
    return out


def noise(duration, decay, volume=1.0, lowpass=0.0):
    """ノイズ（カチッ・ポトッの手触り）。lowpass を大きくするとこもった音になる。"""
    n = int(duration * RATE)
    out = []
    prev = 0.0
    for i in range(n):
        t = i / RATE
        v = random.uniform(-1.0, 1.0)
        prev = prev * lowpass + v * (1.0 - lowpass)
        out.append(prev * math.exp(-t / decay) * volume)
    return out


def mix(length, *parts):
    """(開始秒, サンプル列) を重ねる。"""
    total = max([length] + [start + len(samples) / RATE for start, samples in parts])
    buf = [0.0] * int(total * RATE + 1)
    for start, samples in parts:
        offset = int(start * RATE)
        for i, v in enumerate(samples):
            buf[offset + i] += v
    return buf


def bell(freq, duration, volume=1.0, ring=1.0):
    """金属の響き（チャリン）。整数倍でない倍音を重ねる。ring を大きくすると長く響く。"""
    parts = []
    for ratio, amp, dec in ((1.0, 1.0, 0.18), (2.76, 0.5, 0.10), (5.4, 0.25, 0.05)):
        parts.append((0.0, tone(freq * ratio, duration, sine, decay=dec * ring, attack=0.001, volume=amp * volume)))
    return mix(duration, *parts)


def write(name, samples, peak=0.8):
    """最大の振れ幅を peak にそろえて 16bit モノラルの WAV に書く。"""
    top = max(1e-6, max(abs(v) for v in samples))
    scale = peak / top
    path = os.path.join(OUT_DIR, name + ".wav")
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, v * scale)) * 32767)) for v in samples))
    print("wrote", os.path.relpath(path, ROOT))


# ---------------------------------------------------------------------------
# 効果音
# ---------------------------------------------------------------------------
def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # コインが台に落ちる（ちゃりーん）: 当たったときの短いノイズ（ちゃ）、少しずらした2つ目の響き（り）、長い余韻（ーん）。
    # 払い出しの「チャリン」（E7→B7 の短い2連）と聞き分けられるよう、低めの高さで長く響かせる
    # （ノイズの長さを変えると乱数の進みが変わって、後の音まで変わるので 0.03 秒のまま）
    write("coin_launch", mix(0.7,
                             (0.0, noise(0.03, 0.006, 0.5)),
                             (0.0, bell(note("C7"), 0.25, 0.6)),
                             (0.035, bell(note("G7"), 0.65, 0.8, ring=2.2))), peak=0.5)

    # 払い出し（チャリン）: 金属の響きを2回
    write("coin_payout", mix(0.4, (0.0, bell(note("E7"), 0.35)), (0.06, bell(note("B7"), 0.3, 0.7))), peak=0.55)

    # 溝に落ちた（ポトッ）: 低く下がる音とこもったノイズ
    write("gutter", mix(0.2, (0.0, tone(180, 0.15, sine, freq_end=90, decay=0.05)), (0.0, noise(0.08, 0.02, 0.4, lowpass=0.85))), peak=0.5)

    # 果物ゲット（ポン＋キラン）
    write("fruit_get", mix(0.45,
                           (0.0, tone(330, 0.12, sine, freq_end=660, decay=0.06)),
                           (0.08, tone(note("G6"), 0.25, triangle, decay=0.08, volume=0.5)),
                           (0.14, tone(note("C7"), 0.3, triangle, decay=0.1, volume=0.5))), peak=0.7)

    # 図鑑に登録（上がっていくアルペジオとキラキラ）
    arpeggio = [(i * 0.07, tone(note(n), 0.3, triangle, decay=0.12, volume=0.6)) for i, n in enumerate(["C6", "E6", "G6", "C7", "E7"])]
    sparkle = [(0.35 + i * 0.03, tone(random.uniform(3000, 6000), 0.08, sine, decay=0.03, volume=0.15)) for i in range(10)]
    write("zukan_new", mix(0.9, *(arpeggio + sparkle)), peak=0.7)

    # ルーレットの回転（チッ）
    write("roulette_tick", mix(0.03, (0.0, tone(3800, 0.025, square, decay=0.004, volume=0.4))), peak=0.25)

    # ルーレット当たり（ピンポン）
    write("roulette_hit", mix(0.7, (0.0, tone(note("E6"), 0.35, sine, decay=0.2)), (0.18, tone(note("C6"), 0.5, sine, decay=0.25))), peak=0.6)

    # ルーレットはずれ（ぽわん）
    write("roulette_miss", mix(0.4, (0.0, tone(420, 0.35, sine, freq_end=240, decay=0.15, vibrato=0.03))), peak=0.45)

    # ジャックポットチャンス（どんどん上がる）
    rise = [(i * 0.06, tone(note("C5") * 2 ** (i / 12.0 * 2), 0.07, square, decay=0.03, volume=0.35)) for i in range(16)]
    write("jackpot_chance", mix(1.1, *rise), peak=0.55)

    # ジャックポット当たり（ファンファーレ）
    fanfare = [(0.0, tone(note("C5"), 0.12, square, volume=0.3)), (0.13, tone(note("C5"), 0.12, square, volume=0.3)),
               (0.26, tone(note("C5"), 0.12, square, volume=0.3)), (0.39, tone(note("G5"), 0.35, square, volume=0.3))]
    chord = [(0.78, tone(note(n), 0.9, triangle, decay=0.5, volume=0.5)) for n in ("C6", "E6", "G6")]
    shine = [(0.78 + i * 0.04, tone(random.uniform(3000, 7000), 0.1, sine, decay=0.04, volume=0.12)) for i in range(16)]
    write("jackpot_win", mix(1.8, *(fanfare + chord + shine)), peak=0.75)

    # 魔法（シャラーン）: 高い音がたくさん上へすべる
    shimmer = []
    for i in range(18):
        f = random.uniform(1200, 2600)
        shimmer.append((i * 0.025, tone(f, 0.5, sine, freq_end=f * 1.6, decay=0.15, volume=0.25)))
    write("magic_cast", mix(1.0, *(shimmer + [(0.0, noise(0.4, 0.12, 0.15, lowpass=0.6))])), peak=0.6)

    # 強化（レベルアップ）
    levelup = [(i * 0.08, tone(note(n), 0.15, square, decay=0.08, volume=0.3)) for i, n in enumerate(["G5", "C6", "E6", "G6"])]
    levelup += [(0.35, tone(note(n), 0.6, triangle, decay=0.3, volume=0.45)) for n in ("C6", "E6", "G6", "C7")]
    write("upgrade", mix(1.0, *levelup), peak=0.65)

    # ボタン（ポチ）
    write("button", mix(0.06, (0.0, tone(1200, 0.05, sine, freq_end=900, decay=0.015))), peak=0.4)

    # ここから下は乱数を使わない（上の音の乱数の並びを変えないよう、足す音は末尾に置く）

    # スロットのリールが止まる（ガチャッ）: 低い打音と短い金属音
    write("reel_stop", mix(0.12,
                           (0.0, tone(260, 0.08, square, freq_end=140, decay=0.02, volume=0.5)),
                           (0.0, tone(1600, 0.05, triangle, decay=0.012, volume=0.35))), peak=0.5)

    # リーチ（だんだん高くなる2音の繰り返し）
    reach = []
    for i in range(6):
        base = note("A5") * 2 ** (i / 12.0)
        reach.append((i * 0.09, tone(base, 0.08, square, decay=0.04, volume=0.3)))
        reach.append((i * 0.09 + 0.045, tone(base * 1.5, 0.08, square, decay=0.04, volume=0.25)))
    write("reach", mix(0.7, *reach), peak=0.55)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import math
import random
import struct
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Saved" / "GeneratedSFX"
SR = 44100


def clamp(v):
    return max(-1.0, min(1.0, v))


def write_wav(name, duration, fn):
    OUT.mkdir(parents=True, exist_ok=True)
    path = OUT / f"SFX_{name.replace('-', '_')}.wav"
    frames = []
    count = int(duration * SR)
    for i in range(count):
        t = i / SR
        amp = clamp(fn(t, i, count))
        frames.append(struct.pack("<h", int(amp * 32767)))
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(SR)
        wav.writeframes(b"".join(frames))
    return path


def sine(freq, t):
    return math.sin(2.0 * math.pi * freq * t)


def saw(freq, t):
    return 2.0 * ((t * freq) % 1.0) - 1.0


def envelope(t, dur, attack=0.01, release=0.08):
    if t < attack:
        return t / attack
    if t > dur - release:
        return max(0.0, (dur - t) / release)
    return 1.0


def main():
    rng = random.Random(8421)
    specs = {
        "arrow-shot": lambda t, i, n: 0.22 * envelope(t, n / SR, 0.004, 0.07) * (sine(580 + 900 * t, t) + 0.35 * rng.uniform(-1, 1)),
        "hit": lambda t, i, n: 0.55 * envelope(t, n / SR, 0.001, 0.045) * rng.uniform(-1, 1),
        "pickup": lambda t, i, n: 0.32 * envelope(t, n / SR, 0.005, 0.06) * sine(520 + 1000 * t, t),
        "level-up": lambda t, i, n: 0.25 * envelope(t, n / SR, 0.01, 0.12) * sine([523, 659, 784][min(2, int(t / 0.16))], t),
        "upgrade-select": lambda t, i, n: 0.28 * envelope(t, n / SR, 0.005, 0.08) * (sine(440, t) + 0.7 * sine(660, t)),
        "boss-roar": lambda t, i, n: 0.28 * envelope(t, n / SR, 0.03, 0.2) * saw(95 - 35 * t, t),
        "campfire-loop": lambda t, i, n: 0.08 * (0.5 * rng.uniform(-1, 1) + 0.5 * sine(42, t)),
        "ui-tap": lambda t, i, n: 0.24 * envelope(t, n / SR, 0.001, 0.035) * sine(780, t),
        "defense-horn": lambda t, i, n: 0.34 * envelope(t, n / SR, 0.02, 0.18) * (sine(196, t) + 0.5 * sine(294, t)),
        "defeat-sting": lambda t, i, n: 0.28 * envelope(t, n / SR, 0.01, 0.3) * sine(330 - 120 * t, t),
        "victory-fanfare": lambda t, i, n: 0.24 * envelope(t, n / SR, 0.01, 0.18) * sine([523, 659, 784, 1046, 1318][min(4, int(t / 0.18))], t),
    }
    durations = {
        "arrow-shot": 0.18,
        "hit": 0.10,
        "pickup": 0.22,
        "level-up": 0.56,
        "upgrade-select": 0.18,
        "boss-roar": 0.9,
        "campfire-loop": 2.0,
        "ui-tap": 0.08,
        "defense-horn": 0.65,
        "defeat-sting": 0.8,
        "victory-fanfare": 1.0,
    }
    written = [write_wav(name, durations[name], fn) for name, fn in specs.items()]
    print(f"SFX_OK n={len(written)} out={OUT}")


if __name__ == "__main__":
    main()

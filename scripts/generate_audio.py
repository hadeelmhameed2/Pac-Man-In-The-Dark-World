import math
import struct
import wave
import os
import random

def write_wav(filepath, sample_rate, samples):
    # Ensure directory exists
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with wave.open(filepath, 'w') as w:
        w.setnchannels(1) # mono
        w.setsampwidth(2) # 16-bit
        w.setframerate(sample_rate)
        for s in samples:
            # clamp value between -32768 and 32767
            val = int(max(-32768, min(32767, s * 32767.0)))
            w.writeframes(struct.pack('<h', val))
    print(f"Generated: {filepath}")

def main():
    sample_rate = 44100
    base_dir = "assets/audio"
    
    # 1. eating.wav (subtle retro click/pop)
    duration = 0.015
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 600 - 200 * (t / duration)
        val = math.sin(2 * math.pi * freq * t) * math.exp(-8.0 * (t / duration))
        samples.append(val * 0.12)
    write_wav(os.path.join(base_dir, "eating.wav"), sample_rate, samples)

    # 2. ghost_vulnerable.wav (siren oscillating 400Hz to 550Hz)
    duration = 0.5
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 400 + 150 * math.sin(2 * math.pi * 2.0 * t)
        val = math.sin(2 * math.pi * freq * t)
        edge = 0.02
        envelope = 1.0
        if t < edge:
            envelope = t / edge
        elif t > duration - edge:
            envelope = (duration - t) / edge
        samples.append(val * envelope * 0.4)
    write_wav(os.path.join(base_dir, "ghost_vulnerable.wav"), sample_rate, samples)

    # 3. death.wav (descending sweep 800Hz to 60Hz with decay)
    duration = 1.2
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 800 - 740 * (t / duration)**2
        val = math.sin(2 * math.pi * freq * t)
        envelope = math.exp(-2.5 * t)
        val = 0.7 * val + 0.3 * math.copysign(0.5, val)
        samples.append(val * envelope * 0.8)
    write_wav(os.path.join(base_dir, "death.wav"), sample_rate, samples)

    # 4. light_pellet.wav (subtle low-frequency soft thud)
    duration = 0.03
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 120 - 40 * (t / duration)
        val = math.sin(2 * math.pi * freq * t) * math.exp(-6.0 * (t / duration))
        samples.append(val * 0.05)
    write_wav(os.path.join(base_dir, "light_pellet.wav"), sample_rate, samples)

    # 5. heartbeat.wav (double thump-thump)
    duration = 0.4
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        val = 0.0
        if t < 0.15:
            t1 = t
            val += 0.8 * math.sin(2 * math.pi * 55 * t1) * math.exp(-30.0 * t1)
        else:
            t2 = t - 0.15
            val += 0.7 * math.sin(2 * math.pi * 50 * t2) * math.exp(-25.0 * t2)
        samples.append(val * 0.9)
    write_wav(os.path.join(base_dir, "heartbeat.wav"), sample_rate, samples)

    # 6. power_down.wav (descending sweep with crackle)
    duration = 0.8
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 350 - 300 * (t / duration)
        val = math.sin(2 * math.pi * freq * t)
        noise = random.uniform(-0.08, 0.08) if (i % 4 == 0) else 0.0
        envelope = 1.0 - t / duration
        samples.append((val * 0.8 + noise) * envelope * 0.7)
    write_wav(os.path.join(base_dir, "power_down.wav"), sample_rate, samples)

    # 7. turbine_surge.wav (rising turbine surge with noise)
    duration = 1.5
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        freq = 150 + 750 * (t / duration)
        val = math.sin(2 * math.pi * freq * t)
        noise = random.uniform(-0.15, 0.15)
        envelope = math.sin(math.pi * t / duration)
        samples.append((val * 0.6 + noise * 0.4) * envelope * 0.7)
    write_wav(os.path.join(base_dir, "turbine_surge.wav"), sample_rate, samples)

    # 8. ambient_drone.wav (low spooky drone)
    duration = 4.0
    num_samples = int(sample_rate * duration)
    samples = []
    for i in range(num_samples):
        t = i / sample_rate
        val = (
            0.4 * math.sin(2 * math.pi * 60 * t) +
            0.3 * math.sin(2 * math.pi * 90 * t + 0.5) +
            0.2 * math.sin(2 * math.pi * 120 * t + 1.0) +
            0.1 * math.sin(2 * math.pi * 150 * t + 1.5)
        )
        edge = 0.1
        envelope = 1.0
        if t < edge:
            envelope = t / edge
        elif t > duration - edge:
            envelope = (duration - t) / edge
        samples.append(val * envelope * 0.6)
    write_wav(os.path.join(base_dir, "ambient_drone.wav"), sample_rate, samples)

    # 9. normal_bgm.wav (catchy 8-bit retro background melody loop)
    duration = 4.0
    num_samples = int(sample_rate * duration)
    samples = []
    melody = [220.0, 261.6, 293.7, 329.6, 392.0, 329.6, 293.7, 261.6]
    for i in range(num_samples):
        t = i / sample_rate
        note_idx = int(t / 0.5) % len(melody)
        freq = melody[note_idx]
        note_t = t % 0.5
        val = math.sin(2 * math.pi * freq * t)
        square = math.copysign(0.3, val)
        mixed = 0.6 * val + 0.4 * square
        envelope = math.exp(-4.0 * note_t)
        samples.append(mixed * envelope * 0.4)
    write_wav(os.path.join(base_dir, "normal_bgm.wav"), sample_rate, samples)

    # 10. victory.wav (pleasant chip-tune rising fanfare arpeggio)
    duration = 0.6
    num_samples = int(sample_rate * duration)
    samples = []
    notes = [523.25, 659.25, 783.99, 1046.50, 1318.51, 1567.98] # C5, E5, G5, C6, E6, G6
    note_duration = 0.1
    for i in range(num_samples):
        t = i / sample_rate
        note_idx = min(int(t / note_duration), len(notes) - 1)
        freq = notes[note_idx]
        note_t = t % note_duration
        
        # Chip-tune mixed wave (sine + square)
        val = math.sin(2 * math.pi * freq * t)
        square = math.copysign(0.25, val)
        mixed = 0.7 * val + 0.3 * square
        
        # Exponential decay envelope on each note
        envelope = math.exp(-8.0 * note_t)
        samples.append(mixed * envelope * 0.45)
    write_wav(os.path.join(base_dir, "victory.wav"), sample_rate, samples)

if __name__ == "__main__":
    main()

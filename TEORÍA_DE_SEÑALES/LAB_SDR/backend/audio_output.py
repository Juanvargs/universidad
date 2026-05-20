import os

os.environ.setdefault("PULSE_SERVER", "unix:/mnt/wslg/PulseServer")

import numpy as np

try:
    import sounddevice as sd

    SOUNDDEVICE_ERROR = None
except Exception as exc:
    sd = None
    SOUNDDEVICE_ERROR = exc


class AudioOutput:
    def __init__(self, audio_buffer):
        self.audio_buffer = audio_buffer
        self.stream = None

    @property
    def available(self):
        return sd is not None

    def start(self, sample_rate_hz, block_frames=960, latency_s=0.12):
        if sd is None:
            raise RuntimeError(f"No se puede iniciar audio: {SOUNDDEVICE_ERROR}")

        if self.stream is not None:
            return

        def callback(outdata, frames, time, status):
            del time, status
            outdata[:, 0] = self.audio_buffer.read(frames)

        try:
            self.stream = sd.OutputStream(
                samplerate=sample_rate_hz,
                channels=1,
                dtype="float32",
                callback=callback,
                blocksize=block_frames,
                latency=latency_s,
            )
        except Exception:
            self.stream = sd.OutputStream(
                samplerate=sample_rate_hz,
                channels=1,
                dtype="float32",
                callback=callback,
                blocksize=block_frames,
                latency="high",
            )
        self.stream.start()

    def stop(self):
        if self.stream is not None:
            self.stream.stop()
            self.stream.close()
            self.stream = None
        self.audio_buffer.clear()

    def play_test_tone(self, sample_rate_hz):
        if sd is None:
            raise RuntimeError(f"No se puede reproducir audio: {SOUNDDEVICE_ERROR}")

        t = np.arange(sample_rate_hz // 2) / sample_rate_hz
        tone = (0.25 * np.sin(2 * np.pi * 440 * t)).astype(np.float32)
        sd.play(tone, sample_rate_hz)
        sd.wait()

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
        self.callbacks = 0
        self.frames_consumed = 0
        self.last_status = ""

    @property
    def available(self):
        return sd is not None

    def start(self, sample_rate_hz, block_frames=960, latency_s=0.12):
        if sd is None:
            raise RuntimeError(f"No se puede iniciar audio: {SOUNDDEVICE_ERROR}")

        if self.stream is not None:
            return

        def callback(outdata, frames, time, status):
            del time
            self.callbacks += 1
            self.frames_consumed += frames
            if status:
                self.last_status = str(status)
            outdata[:, 0] = self.audio_buffer.read(frames)

        try:
            self.stream = sd.OutputStream(
                samplerate=sample_rate_hz,
                device="pulse",
                channels=1,
                dtype="float32",
                callback=callback,
                blocksize=block_frames,
                latency=latency_s,
            )
        except Exception:
            self.stream = sd.OutputStream(
                samplerate=sample_rate_hz,
                device="pulse",
                channels=1,
                dtype="float32",
                callback=callback,
                blocksize=block_frames,
                latency="high",
            )
        self.stream.start()

    def stop(self, clear_buffer=True):
        if self.stream is not None:
            self.stream.stop()
            self.stream.close()
            self.stream = None
        if clear_buffer:
            self.audio_buffer.clear()
        self.callbacks = 0
        self.frames_consumed = 0
        self.last_status = ""

    def restart(self, sample_rate_hz, block_frames=0, latency_s=0.2):
        self.stop(clear_buffer=False)
        self.start(sample_rate_hz, block_frames=block_frames, latency_s=latency_s)

    def stats(self):
        return {
            "callbacks": self.callbacks,
            "frames_consumed": self.frames_consumed,
            "last_status": self.last_status,
        }

    def play_test_tone(self, sample_rate_hz):
        if sd is None:
            raise RuntimeError(f"No se puede reproducir audio: {SOUNDDEVICE_ERROR}")

        t = np.arange(sample_rate_hz // 2) / sample_rate_hz
        tone = (0.25 * np.sin(2 * np.pi * 440 * t)).astype(np.float32)
        sd.play(tone, sample_rate_hz)
        sd.wait()

import threading
import time


class AcquisitionWorker:
    def __init__(self, sdr, config, iq_buffer, audio_buffer, demodulator, on_error=None, on_audio=None):
        self.sdr = sdr
        self.config = config
        self.iq_buffer = iq_buffer
        self.audio_buffer = audio_buffer
        self.demodulator = demodulator
        self.on_error = on_error
        self.on_audio = on_audio
        self.stop_event = threading.Event()
        self.thread = None
        self.blocks_read = 0
        self.last_block_time = 0.0

    def start(self):
        self.stop_event.clear()
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def stop(self):
        self.stop_event.set()
        self.sdr.cancel_async_read()
        if self.thread is not None and self.thread.is_alive():
            self.thread.join(timeout=1.5)
        self.thread = None

    @property
    def is_alive(self):
        return self.thread is not None and self.thread.is_alive()

    def _run(self):
        callback_error = None

        def handle_samples(samples):
            nonlocal callback_error
            if self.stop_event.is_set():
                self.sdr.cancel_async_read()
                return

            try:
                self.iq_buffer.push(samples, self.config.psd_buffer_samples)

                audio = self.demodulator.demodulate(samples, self.config)
                self.audio_buffer.push(audio)

                if self.on_audio is not None and audio.size:
                    self.on_audio(audio)

                self.blocks_read += 1
                self.last_block_time = time.monotonic()
            except Exception as exc:
                callback_error = exc
                self.stop_event.set()
                self.sdr.cancel_async_read()

        try:
            self.sdr.read_samples_async(handle_samples, self.config.audio_block_samples)
            if callback_error is not None:
                raise callback_error
        except Exception as exc:
            if (callback_error is not None or not self.stop_event.is_set()) and self.on_error is not None:
                self.on_error(exc)

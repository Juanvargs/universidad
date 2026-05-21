import threading
import time
from collections import deque


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
        self.process_thread = None
        self.queue = deque()
        self.queue_lock = threading.Lock()
        self.queue_ready = threading.Condition(self.queue_lock)
        self.max_queue_blocks = 12
        self.dropped_blocks = 0
        self.blocks_read = 0
        self.last_block_time = 0.0

    def start(self):
        self.stop_event.clear()
        self.process_thread = threading.Thread(target=self._process_samples, daemon=True)
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.process_thread.start()
        self.thread.start()

    def stop(self):
        self.stop_event.set()
        self.sdr.cancel_async_read()
        with self.queue_ready:
            self.queue_ready.notify_all()

        current = threading.current_thread()
        if self.thread is not None and self.thread.is_alive() and self.thread is not current:
            self.thread.join(timeout=1.5)
        if self.process_thread is not None and self.process_thread.is_alive() and self.process_thread is not current:
            self.process_thread.join(timeout=1.5)
        self.thread = None
        self.process_thread = None
        with self.queue_ready:
            self.queue.clear()

    @property
    def is_alive(self):
        return (
            self.thread is not None
            and self.thread.is_alive()
            or self.process_thread is not None
            and self.process_thread.is_alive()
        )

    def _run(self):
        callback_error = None

        def handle_samples(samples):
            nonlocal callback_error
            if self.stop_event.is_set():
                self.sdr.cancel_async_read()
                return

            try:
                with self.queue_ready:
                    if len(self.queue) >= self.max_queue_blocks:
                        self.queue.popleft()
                        self.dropped_blocks += 1
                    self.queue.append(samples)
                    self.queue_ready.notify()
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

    def _process_samples(self):
        try:
            while not self.stop_event.is_set():
                with self.queue_ready:
                    while not self.queue and not self.stop_event.is_set():
                        self.queue_ready.wait(timeout=0.2)
                    if self.stop_event.is_set():
                        return
                    samples = self.queue.popleft()

                self.iq_buffer.push(samples, self.config.psd_buffer_samples)

                audio = self.demodulator.demodulate(samples, self.config)
                self.audio_buffer.push(audio)

                if self.on_audio is not None and audio.size:
                    self.on_audio(audio)

                self.blocks_read += 1
                self.last_block_time = time.monotonic()
        except Exception as exc:
            if not self.stop_event.is_set() and self.on_error is not None:
                self.on_error(exc)

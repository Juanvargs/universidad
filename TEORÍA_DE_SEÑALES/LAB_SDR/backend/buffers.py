import collections
import threading

import numpy as np


class IQBuffer:
    def __init__(self):
        self.chunks = collections.deque()
        self.size = 0
        self.lock = threading.Lock()

    def clear(self):
        with self.lock:
            self.chunks.clear()
            self.size = 0

    def push(self, samples, max_samples):
        if samples.size == 0:
            return

        with self.lock:
            self.chunks.append(samples)
            self.size += len(samples)

            while self.size > max_samples and self.chunks:
                extra = self.size - max_samples
                first = self.chunks[0]

                if len(first) <= extra:
                    self.size -= len(first)
                    self.chunks.popleft()
                else:
                    self.chunks[0] = first[extra:]
                    self.size -= extra

    def latest(self, max_samples):
        with self.lock:
            if self.size == 0:
                return np.array([], dtype=np.complex64)

            data = np.concatenate(list(self.chunks))

        if len(data) > max_samples:
            data = data[-max_samples:]
        return data

    def available_samples(self):
        with self.lock:
            return self.size


class AudioBuffer:
    def __init__(self, max_chunks=64, max_frames=24_000):
        self.queue = collections.deque()
        self.max_chunks = max_chunks
        self.max_frames = max_frames
        self.lock = threading.Lock()
        self.frames = 0
        self.underruns = 0
        self.underrun_frames = 0
        self.dropped_frames = 0
        self.peak_frames = 0

    def configure(self, max_frames, max_chunks=None):
        with self.lock:
            self.max_frames = max_frames
            if max_chunks is not None:
                self.max_chunks = max_chunks
            self._trim_locked()

    def clear(self, reset_stats=True):
        with self.lock:
            self.queue.clear()
            self.frames = 0
            if reset_stats:
                self.underruns = 0
                self.underrun_frames = 0
                self.dropped_frames = 0
                self.peak_frames = 0

    def push(self, samples):
        if samples.size == 0:
            return

        with self.lock:
            self.queue.append(samples.astype(np.float32, copy=False))
            self.frames += len(samples)
            self.peak_frames = max(self.peak_frames, self.frames)
            self._trim_locked()

    def read(self, frames):
        output = np.zeros(frames, dtype=np.float32)
        filled = 0

        with self.lock:
            while filled < frames and self.queue:
                chunk = self.queue[0]
                needed = frames - filled

                if len(chunk) <= needed:
                    output[filled : filled + len(chunk)] = chunk
                    filled += len(chunk)
                    self.frames -= len(chunk)
                    self.queue.popleft()
                else:
                    output[filled:] = chunk[:needed]
                    self.queue[0] = chunk[needed:]
                    self.frames -= needed
                    filled = frames

            if filled < frames:
                self.underruns += 1
                self.underrun_frames += frames - filled

        return output

    def available_frames(self):
        with self.lock:
            return self.frames

    def stats(self):
        with self.lock:
            return {
                "frames": self.frames,
                "max_frames": self.max_frames,
                "peak_frames": self.peak_frames,
                "underruns": self.underruns,
                "underrun_frames": self.underrun_frames,
                "dropped_frames": self.dropped_frames,
                "chunks": len(self.queue),
            }

    def _trim_locked(self):
        while len(self.queue) > self.max_chunks:
            removed = self.queue.popleft()
            removed_frames = len(removed)
            self.frames -= removed_frames
            self.dropped_frames += removed_frames

        while self.frames > self.max_frames and self.queue:
            extra = self.frames - self.max_frames
            first = self.queue[0]

            if len(first) <= extra:
                removed_frames = len(first)
                self.queue.popleft()
                self.frames -= removed_frames
                self.dropped_frames += removed_frames
            else:
                self.queue[0] = first[extra:]
                self.frames -= extra
                self.dropped_frames += extra

import threading
import time
from dataclasses import replace

import numpy as np

from backend.acquisition import AcquisitionWorker
from backend.audio_output import AudioOutput
from backend.buffers import AudioBuffer, IQBuffer
from backend.config import SDRConfig
from backend.dsp import FMDemodulator, compute_psd
from backend.sdr_device import SDRDevice


class DashboardController:
    def __init__(self):
        self.lock = threading.RLock()
        self.control_lock = threading.Lock()
        self.config = SDRConfig()
        self.sdr = SDRDevice()
        self.iq_buffer = IQBuffer()
        self.audio_buffer = AudioBuffer()
        self.audio_output = AudioOutput(self.audio_buffer)
        self.demodulator = FMDemodulator()
        self.worker = None
        self.monitor_thread = None
        self.stop_monitor = threading.Event()
        self.retry_thread = None
        self.retry_stop = threading.Event()
        self.desired_running = False
        self.waiting_device = False
        self.last_error = None
        self.running = False
        self.audio_started = False
        self.status = "Listo para iniciar."
        self.start_time = None
        self.last_audio = np.zeros(1200, dtype=np.float32)
        self.last_audio_rms = 0.0
        self.last_audio_peak = 0.0
        self.psd_db_avg = None
        self.last_psd_at = 0.0
        self.last_psd_payload = {"freq_mhz": [], "db": []}
        self.last_audio_consumed_frames = 0
        self.last_audio_consumed_at = time.monotonic()
        self.audio_buffer.configure(self.config.audio_buffer_frames, max_chunks=256)
        self.demodulator.reset(self.config)

    def start(self, payload):
        with self.control_lock:
            with self.lock:
                config = self._config_from_payload(payload)
                config.validate()
                self.desired_running = True

            self._stop_runtime("Aplicando nueva configuracion...", keep_desired=True)
            self.retry_stop.clear()
            return self._start_runtime_or_wait(config)

    def stop(self):
        with self.control_lock:
            with self.lock:
                self.desired_running = False
            self._stop_runtime("Sistema detenido.")
            return self.status_payload()

    def status_payload(self):
        with self.lock:
            stats = self.audio_buffer.stats()
            output_stats = self.audio_output.stats()
            uptime = 0.0 if self.start_time is None else max(0.0, time.monotonic() - self.start_time)
            return {
                "running": self.running,
                "audio_started": self.audio_started,
                "desired_running": self.desired_running,
                "waiting_device": self.waiting_device,
                "status": self.status,
                "last_error": self.last_error,
                "uptime_s": uptime,
                "config": self._config_payload_locked(),
                "buffer_ms": stats["frames"] / self.config.audio_rate_hz * 1000.0,
                "buffer_max_ms": stats["max_frames"] / self.config.audio_rate_hz * 1000.0,
                "underruns": stats["underruns"],
                "dropped_ms": stats["dropped_frames"] / self.config.audio_rate_hz * 1000.0,
                "blocks_read": 0 if self.worker is None else self.worker.blocks_read,
                "audio_rms": self.last_audio_rms,
                "audio_peak": self.last_audio_peak,
                "audio_callbacks": output_stats["callbacks"],
                "audio_frames_consumed": output_stats["frames_consumed"],
                "audio_last_status": output_stats["last_status"],
            }

    def snapshot_payload(self):
        psd_update = None
        with self.lock:
            config = self.config
            previous_psd = self.psd_db_avg
            should_update_psd = (
                self.iq_buffer.available_samples() >= config.nperseg
                and time.monotonic() - self.last_psd_at >= 0.5
            )
            if should_update_psd:
                iq_samples = self.iq_buffer.latest(config.psd_buffer_samples)
            else:
                iq_samples = None
            audio = self.last_audio.copy()

        if iq_samples is not None:
            freqs_mhz, psd_db_avg = compute_psd(iq_samples, config, previous_psd)
            psd_update = (psd_db_avg, self._decimate_xy(freqs_mhz, psd_db_avg, 900))

        with self.lock:
            if psd_update is not None:
                self.psd_db_avg, self.last_psd_payload = psd_update
                self.last_psd_at = time.monotonic()

            audio_t_ms = np.arange(len(audio)) / self.config.audio_rate_hz * 1000.0
            audio_payload = self._decimate_xy(audio_t_ms, audio, 700)
            status = self.status_payload()
            return {
                "status": status,
                "psd": self.last_psd_payload,
                "audio": {"time_ms": audio_payload["freq_mhz"], "amplitude": audio_payload["db"]},
            }

    def shutdown(self):
        self._stop_runtime("Sistema detenido.")

    def _start_runtime_or_wait(self, config):
        try:
            self._start_runtime(config)
        except Exception as exc:
            self._enter_waiting_device(config, exc)
        return self.status_payload()

    def _start_runtime(self, config):
        with self.lock:
            self.config = config
            self._reset_processing_locked()
            self.running = False
            self.audio_started = False
            self.waiting_device = False
            self.last_error = None
            self.start_time = None
            self.status = "Abriendo SDR..."

        self.sdr.open(config)
        config = self._auto_tune_config(config)
        with self.lock:
            self.config = config

        worker = AcquisitionWorker(
            self.sdr,
            config,
            self.iq_buffer,
            self.audio_buffer,
            self.demodulator,
            on_error=self._handle_worker_error,
            on_audio=self._store_latest_audio,
        )
        self.stop_monitor.clear()
        monitor_thread = threading.Thread(target=self._monitor_audio, daemon=True)

        with self.lock:
            self.worker = worker
            self.monitor_thread = monitor_thread
            self.running = True
            self.audio_started = False
            self.waiting_device = False
            self.last_error = None
            self.start_time = time.monotonic()
            self.status = self._running_status_locked()

        worker.start()
        monitor_thread.start()

    def _auto_tune_config(self, config):
        if not self._should_auto_scan(config):
            return config

        try:
            best_freq = self._scan_fm_band(config)
            if best_freq is None:
                return config

            if abs(best_freq - config.fc_mhz) < 0.005:
                return config

            tuned_config = replace(config, fc_mhz=round(best_freq, 6))
            self.sdr.configure(tuned_config)
            return tuned_config
        except Exception:
            return config

    def _should_auto_scan(self, config):
        if not (87.5 <= config.fc_mhz <= 108.0):
            return False
        return abs(config.fc_mhz - 100.0) < 1e-6

    def _scan_fm_band(self, config):
        scan_span = min(3.2, max(config.span_mhz, 2.88))
        step = scan_span * 0.8
        centers = []
        current = 87.5 + scan_span / 2
        while current <= 108.0 - scan_span / 2 + 1e-9:
            centers.append(current)
            current += step
        if not centers or centers[-1] + scan_span / 2 < 108.0 - 1e-6:
            centers.append(108.0 - scan_span / 2)

        best_freq = None
        best_power = -1e18
        for center in centers:
            scan_config = replace(config, fc_mhz=center, span_mhz=scan_span)
            self.sdr.configure(scan_config)
            samples = self.sdr.read_samples(scan_config.nperseg * 4)
            freq_mhz, peak_db = self._find_peak_frequency(scan_config, samples)
            if peak_db is not None and peak_db > best_power:
                best_power = peak_db
                best_freq = freq_mhz

        return best_freq

    def _find_peak_frequency(self, config, samples):
        if samples.size == 0:
            return None, None

        freqs_mhz, psd_db = compute_psd(samples, config)
        peak_index = int(np.argmax(psd_db))
        if peak_index < 0 or peak_index >= len(freqs_mhz):
            return None, None
        return float(freqs_mhz[peak_index]), float(psd_db[peak_index])

    def _enter_waiting_device(self, config, exc):
        self.sdr.close()
        self.retry_stop.clear()
        with self.lock:
            self.config = config
            self.running = False
            self.audio_started = False
            self.waiting_device = True
            self.last_error = str(exc)
            self.start_time = None
            self.status = f"Esperando SDR. Reconecta la antena o adjuntala a WSL. Ultimo error: {exc}"
        self._ensure_retry_thread()

    def _ensure_retry_thread(self):
        with self.lock:
            if self.retry_thread is not None and self.retry_thread.is_alive():
                return
            self.retry_thread = threading.Thread(target=self._retry_open_loop, daemon=True)
            self.retry_thread.start()

    def _retry_open_loop(self):
        while not self.retry_stop.wait(2.0):
            with self.lock:
                if not self.desired_running:
                    return
                if self.running:
                    self.waiting_device = False
                    return
                config = self.config
                self.status = "Buscando SDR..."

            with self.control_lock:
                with self.lock:
                    if not self.desired_running or self.running:
                        return
                try:
                    self._start_runtime(config)
                    return
                except Exception as exc:
                    self.sdr.close()
                    with self.lock:
                        if not self.desired_running:
                            return
                        self.waiting_device = True
                        self.last_error = str(exc)
                        self.status = f"Esperando SDR. Reintentando en 2 s. Ultimo error: {exc}"

    def _config_from_payload(self, payload):
        payload = payload or {}
        nfft = payload.get("nfft", payload.get("nperseg", self.config.nperseg))
        return SDRConfig(
            fc_mhz=self._float(payload, "fc_mhz", self.config.fc_mhz),
            span_mhz=self._float(payload, "span_mhz", self.config.span_mhz),
            nperseg=self._int_value(nfft),
            noverlap=self._int(payload, "noverlap", self.config.noverlap),
            gain_db=self._float(payload, "gain_db", self.config.gain_db),
            freq_correction_ppm=self._int(payload, "freq_correction_ppm", self.config.freq_correction_ppm),
            fine_tune_khz=self._float(payload, "fine_tune_khz", self.config.fine_tune_khz),
            deemphasis_us=self._float(payload, "deemphasis_us", self.config.deemphasis_us),
        )

    def _reset_processing_locked(self):
        self.psd_db_avg = None
        self.last_psd_payload = {"freq_mhz": [], "db": []}
        self.iq_buffer.clear()
        self.audio_buffer.configure(self.config.audio_buffer_frames, max_chunks=256)
        self.audio_buffer.clear()
        self.last_audio = np.zeros(1200, dtype=np.float32)
        self.last_audio_rms = 0.0
        self.last_audio_peak = 0.0
        self.last_audio_consumed_frames = 0
        self.last_audio_consumed_at = time.monotonic()
        self.demodulator.reset(self.config)

    def _stop_runtime(self, final_status, keep_desired=False, stop_retry=True):
        with self.lock:
            worker = self.worker
            retry_thread = self.retry_thread if stop_retry else None
            self.worker = None
            if stop_retry:
                self.retry_thread = None
                self.retry_stop.set()
            if not keep_desired:
                self.desired_running = False
            self.running = False
            self.audio_started = False
            self.waiting_device = False
            self.start_time = None
            self.stop_monitor.set()
            self.status = "Deteniendo..."

        if worker is not None:
            worker.stop()

        self.audio_output.stop()
        self.sdr.close()

        if retry_thread is not None and retry_thread.is_alive() and retry_thread is not threading.current_thread():
            retry_thread.join(timeout=1.0)

        with self.lock:
            self.status = final_status

    def _monitor_audio(self):
        while not self.stop_monitor.is_set():
            try:
                start_audio = False
                audio_args = None
                restart_audio = False
                stalled_error = None
                with self.lock:
                    if not self.running:
                        return
                    if self.worker is not None and self.start_time is not None:
                        now = time.monotonic()
                        last_block_time = self.worker.last_block_time or self.start_time
                        if now - self.start_time > 4.0 and now - last_block_time > 4.0:
                            stalled_error = RuntimeError("SDR sin muestras por mas de 4 s.")
                    if not self.audio_started:
                        available = self.audio_buffer.available_frames()
                        needed = self.config.audio_prebuffer_frames
                        if available >= needed:
                            start_audio = True
                            audio_args = (
                                self.config.audio_rate_hz,
                                self.config.audio_output_block_frames,
                                self.config.audio_output_latency_s,
                            )
                        else:
                            self.status = f"Llenando buffer de audio: {available}/{needed} muestras."
                    else:
                        self.status = self._running_status_locked()
                        output_stats = self.audio_output.stats()
                        consumed = output_stats["frames_consumed"]
                        now = time.monotonic()
                        if consumed != self.last_audio_consumed_frames:
                            self.last_audio_consumed_frames = consumed
                            self.last_audio_consumed_at = now
                        elif now - self.last_audio_consumed_at > 2.0:
                            restart_audio = True
                            audio_args = (
                                self.config.audio_rate_hz,
                                self.config.audio_output_block_frames,
                                self.config.audio_output_latency_s,
                            )
                            self.last_audio_consumed_at = now

                if stalled_error is not None:
                    self._handle_worker_error(stalled_error)
                    return

                if restart_audio and audio_args is not None:
                    self.audio_output.restart(audio_args[0], block_frames=audio_args[1], latency_s=audio_args[2])
                    with self.lock:
                        self.last_audio_consumed_frames = 0
                        self.last_audio_consumed_at = time.monotonic()
                        self.status = "Salida de audio reiniciada por watchdog."
                    time.sleep(0.08)
                    continue

                if start_audio and audio_args is not None:
                    self.audio_output.start(audio_args[0], block_frames=audio_args[1], latency_s=audio_args[2])
                    with self.lock:
                        if self.running:
                            self.audio_started = True
                            self.last_audio_consumed_frames = 0
                            self.last_audio_consumed_at = time.monotonic()
                            self.status = self._running_status_locked()
                time.sleep(0.08)
            except Exception as exc:
                self._handle_worker_error(exc)
                return

    def _handle_worker_error(self, exc):
        with self.lock:
            should_retry = self.desired_running
            config = self.config
        self._stop_runtime("SDR desconectado.", keep_desired=should_retry, stop_retry=False)
        if should_retry:
            self._enter_waiting_device(config, exc)

    def _store_latest_audio(self, audio):
        if audio.size == 0:
            return
        with self.lock:
            max_plot_samples = self.config.audio_rate_hz // 20
            self.last_audio = audio[-min(len(audio), max_plot_samples) :].astype(np.float32, copy=True)
            self.last_audio_rms = float(np.sqrt(np.mean(audio * audio)))
            self.last_audio_peak = float(np.max(np.abs(audio)))

    def _running_status_locked(self):
        stats = self.audio_buffer.stats()
        buffer_ms = stats["frames"] / self.config.audio_rate_hz * 1000.0
        return (
            f"Adquiriendo FC={self.config.fc_mhz:.3f} MHz, Span={self.config.span_mhz:.3f} MHz, "
            f"fine={self.config.fine_tune_khz:+.1f} kHz. "
            f"Buffer={buffer_ms:.0f} ms, underruns={stats['underruns']}."
        )

    def _config_payload_locked(self):
        return {
            "fc_mhz": self.config.fc_mhz,
            "span_mhz": self.config.span_mhz,
            "nfft": self.config.nperseg,
            "nperseg": self.config.nperseg,
            "noverlap": self.config.noverlap,
            "gain_db": self.config.gain_db,
            "freq_correction_ppm": self.config.freq_correction_ppm,
            "fine_tune_khz": self.config.fine_tune_khz,
            "deemphasis_us": self.config.deemphasis_us,
            "channel_rate_hz": self.config.channel_rate_hz,
            "channel_filter_cutoff_hz": self.config.channel_filter_cutoff_hz,
            "audio_rate_hz": self.config.audio_rate_hz,
            "sample_rate_hz": self.config.sample_rate_hz,
            "center_freq_hz": self.config.center_freq_hz,
        }

    @staticmethod
    def _decimate_xy(x_values, y_values, max_points):
        if len(x_values) == 0 or len(y_values) == 0:
            return {"freq_mhz": [], "db": []}
        step = max(1, int(np.ceil(len(x_values) / max_points)))
        return {
            "freq_mhz": np.asarray(x_values[::step], dtype=float).round(6).tolist(),
            "db": np.asarray(y_values[::step], dtype=float).round(4).tolist(),
        }

    @staticmethod
    def _float(payload, key, default):
        value = payload.get(key, default)
        if isinstance(value, str):
            value = value.replace(",", ".")
        return float(value)

    @staticmethod
    def _int(payload, key, default):
        return DashboardController._int_value(payload.get(key, default))

    @staticmethod
    def _int_value(value):
        if isinstance(value, str):
            value = value.replace(",", ".")
        return int(float(value))

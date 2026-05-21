import threading
import time
import tkinter as tk
from tkinter import messagebox, ttk

import matplotlib

matplotlib.use("TkAgg")

import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

from backend.acquisition import AcquisitionWorker
from backend.audio_output import AudioOutput
from backend.buffers import AudioBuffer, IQBuffer
from backend.config import SDRConfig
from backend.dsp import FMDemodulator, compute_psd
from backend.sdr_device import SDRDevice


class SDRApp:
    def __init__(self, root):
        self.root = root
        self.root.title("RTL-SDR")
        self.root.geometry("1280x760")
        self.root.minsize(1120, 680)
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

        self.config = SDRConfig()
        self.vars = {}

        self.sdr = SDRDevice()
        self.iq_buffer = IQBuffer()
        self.audio_buffer = AudioBuffer()
        self.audio_output = AudioOutput(self.audio_buffer)
        self.demodulator = FMDemodulator()
        self.acquisition_worker = None

        self.running = False
        self.after_id = None
        self.psd_db_avg = None
        self.last_audio = np.zeros(1200, dtype=np.float32)
        self.last_audio_lock = threading.Lock()
        self.last_psd_update = 0.0
        self.last_audio_plot_update = 0.0
        self.last_status_update = 0.0
        self.audio_started = False
        self.start_time = 0.0

        self._build_style()
        self._build_layout()
        self._create_plots()
        self.apply_parameters(update_device=False)

    def _build_style(self):
        self.root.configure(bg="#f4f6f8")
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("TFrame", background="#f4f6f8")
        style.configure("Panel.TFrame", background="#ffffff", borderwidth=1, relief="solid")
        style.configure("TLabel", background="#ffffff", foreground="#1f2937", font=("Segoe UI", 10))
        style.configure("Title.TLabel", background="#ffffff", foreground="#111827", font=("Segoe UI", 13, "bold"))
        style.configure("Status.TLabel", background="#ffffff", foreground="#374151", font=("Segoe UI", 9))
        style.configure("TButton", font=("Segoe UI", 10), padding=6)
        style.configure("Start.TButton", font=("Segoe UI", 10, "bold"))

    def _build_layout(self):
        self.root.columnconfigure(0, weight=0, minsize=300)
        self.root.columnconfigure(1, weight=1)
        self.root.rowconfigure(0, weight=1)

        self.param_frame = ttk.Frame(self.root, style="Panel.TFrame", padding=16)
        self.param_frame.grid(row=0, column=0, sticky="nsew", padx=(14, 8), pady=14)

        self.plot_frame = ttk.Frame(self.root, style="TFrame")
        self.plot_frame.grid(row=0, column=1, sticky="nsew", padx=(8, 14), pady=14)
        self.plot_frame.columnconfigure(0, weight=1)
        self.plot_frame.rowconfigure(0, weight=3)
        self.plot_frame.rowconfigure(1, weight=2)

        ttk.Label(self.param_frame, text="Parámetros", style="Title.TLabel").grid(
            row=0, column=0, columnspan=2, sticky="w", pady=(0, 12)
        )

        fields = [
            ("FC [MHz]", "fc_mhz", self.config.fc_mhz),
            ("Span [MHz]", "span_mhz", self.config.span_mhz),
            ("NFFT / Nperseg", "nperseg", self.config.nperseg),
            ("noverlap", "noverlap", self.config.noverlap),
        ]

        for row, (label, key, value) in enumerate(fields, start=1):
            ttk.Label(self.param_frame, text=label).grid(row=row, column=0, sticky="w", pady=6)
            var = tk.StringVar(value=str(value))
            self.vars[key] = var
            ttk.Entry(self.param_frame, textvariable=var, width=12).grid(
                row=row,
                column=1,
                sticky="ew",
                pady=6,
                padx=(10, 0),
            )

        self.param_frame.columnconfigure(1, weight=1)

        button_frame = ttk.Frame(self.param_frame, style="Panel.TFrame")
        button_frame.grid(row=6, column=0, columnspan=2, sticky="ew", pady=(18, 8))
        button_frame.columnconfigure((0, 1), weight=1)

        ttk.Button(button_frame, text="Iniciar", style="Start.TButton", command=self.start).grid(
            row=0, column=0, sticky="ew", padx=(0, 5), pady=4
        )
        ttk.Button(button_frame, text="Detener", command=self.stop).grid(row=0, column=1, sticky="ew", padx=(5, 0), pady=4)
        ttk.Button(button_frame, text="Aplicar", command=lambda: self.apply_parameters(update_device=True)).grid(
            row=1, column=0, sticky="ew", padx=(0, 5), pady=4
        )
        ttk.Button(button_frame, text="Salir", command=self.on_close).grid(row=1, column=1, sticky="ew", padx=(5, 0), pady=4)
        ttk.Button(button_frame, text="Verificar SDR", command=self.verify_sdr).grid(
            row=2, column=0, sticky="ew", padx=(0, 5), pady=4
        )
        ttk.Button(button_frame, text="Probar audio", command=self.test_audio).grid(
            row=2, column=1, sticky="ew", padx=(5, 0), pady=4
        )

        self.status_var = tk.StringVar(value="Listo. Configura FC, Span, NFFT/Nperseg y noverlap.")
        ttk.Label(
            self.param_frame,
            textvariable=self.status_var,
            style="Status.TLabel",
            wraplength=250,
            justify="left",
        ).grid(row=7, column=0, columnspan=2, sticky="ew", pady=(12, 0))

    def _create_plots(self):
        self.psd_panel = ttk.Frame(self.plot_frame, style="Panel.TFrame", padding=8)
        self.psd_panel.grid(row=0, column=0, sticky="nsew", pady=(0, 8))

        self.audio_panel = ttk.Frame(self.plot_frame, style="Panel.TFrame", padding=8)
        self.audio_panel.grid(row=1, column=0, sticky="nsew", pady=(8, 0))

        self.psd_panel.columnconfigure(0, weight=1)
        self.psd_panel.rowconfigure(0, weight=1)
        self.audio_panel.columnconfigure(0, weight=1)
        self.audio_panel.rowconfigure(0, weight=1)

        self.psd_fig = Figure(figsize=(8, 4.2), dpi=100)
        self.psd_ax = self.psd_fig.add_subplot(111)
        self.psd_line, = self.psd_ax.plot([], [], color="#2563eb", linewidth=1.2)
        self.psd_ax.set_title("PSD")
        self.psd_ax.set_xlabel("Frecuencia [MHz]")
        self.psd_ax.set_ylabel("PSD [dB/Hz]")
        self.psd_ax.grid(True, alpha=0.35)
        self.psd_fig.tight_layout()

        self.psd_canvas = FigureCanvasTkAgg(self.psd_fig, master=self.psd_panel)
        self.psd_canvas.get_tk_widget().grid(row=0, column=0, sticky="nsew")

        self.audio_fig = Figure(figsize=(8, 2.6), dpi=100)
        self.audio_ax = self.audio_fig.add_subplot(111)
        self.audio_line, = self.audio_ax.plot([], [], color="#16a34a", linewidth=1.0)
        self.audio_ax.set_title("Audio demodulado")
        self.audio_ax.set_xlabel("Tiempo [ms]")
        self.audio_ax.set_ylabel("Amplitud")
        self.audio_ax.grid(True, alpha=0.35)
        self.audio_ax.set_ylim(-1.1, 1.1)
        self.audio_fig.tight_layout()

        self.audio_canvas = FigureCanvasTkAgg(self.audio_fig, master=self.audio_panel)
        self.audio_canvas.get_tk_widget().grid(row=0, column=0, sticky="nsew")

    def apply_parameters(self, update_device):
        try:
            if update_device and self.running:
                self.stop()

            config = self.read_config_from_ui()
            config.validate()
            self.config = config
            self.reset_processing()

            if update_device and self.sdr.is_open:
                self.sdr.configure(self.config)

            self.update_axes()
            self.set_status("Parámetros aplicados.")
            return True
        except Exception as exc:
            self.show_error(exc)
            return False

    def read_config_from_ui(self):
        return SDRConfig(
            fc_mhz=self.read_float("fc_mhz"),
            span_mhz=self.read_float("span_mhz"),
            nperseg=self.read_int("nperseg"),
            noverlap=self.read_int("noverlap"),
        )

    def read_float(self, key):
        try:
            return float(self.vars[key].get().replace(",", "."))
        except ValueError as exc:
            raise ValueError(f"Valor inválido en {key}.") from exc

    def read_int(self, key):
        try:
            return int(float(self.vars[key].get().replace(",", ".")))
        except ValueError as exc:
            raise ValueError(f"Valor inválido en {key}.") from exc

    def reset_processing(self):
        self.psd_db_avg = None
        self.iq_buffer.clear()
        self.audio_buffer.configure(self.config.audio_buffer_frames)
        self.audio_buffer.clear()
        with self.last_audio_lock:
            self.last_audio = np.zeros(1200, dtype=np.float32)
        self.demodulator.reset(self.config)

    def update_axes(self):
        freq_axis_mhz = np.linspace(
            self.config.fc_mhz - self.config.span_mhz / 2,
            self.config.fc_mhz + self.config.span_mhz / 2,
            self.config.nperseg,
            endpoint=False,
        )

        self.psd_line.set_data(freq_axis_mhz, np.zeros(self.config.nperseg))
        self.psd_ax.set_xlim(freq_axis_mhz[0], freq_axis_mhz[-1])
        self.psd_ax.set_ylim(self.config.psd_y_min, self.config.psd_y_max)
        self.psd_canvas.draw_idle()

        with self.last_audio_lock:
            audio_plot = self.last_audio.copy()

        audio_t_ms = np.arange(len(audio_plot)) / self.config.audio_rate_hz * 1000.0
        self.audio_line.set_data(audio_t_ms, audio_plot)
        self.audio_ax.set_xlim(0, max(1.0, audio_t_ms[-1] if len(audio_t_ms) else 1.0))
        self.audio_canvas.draw_idle()

    def start(self):
        if self.running:
            return

        try:
            if not self.apply_parameters(update_device=False):
                return

            self.sdr.open(self.config)
            self.running = True
            self.audio_started = False
            self.start_time = time.monotonic()
            self.acquisition_worker = AcquisitionWorker(
                self.sdr,
                self.config,
                self.iq_buffer,
                self.audio_buffer,
                self.demodulator,
                on_error=self.handle_worker_error,
                on_audio=self.store_latest_audio,
            )
            self.acquisition_worker.start()
            self.set_status(f"Adquiriendo: FC={self.config.fc_mhz:.3f} MHz, Span={self.config.span_mhz:.3f} MHz.")
            self.schedule_ui_update(delay_ms=20)
        except Exception as exc:
            self.running = False
            self.stop_worker()
            self.sdr.close()
            self.show_error(exc)

    def stop(self):
        self.running = False
        if self.after_id is not None:
            self.root.after_cancel(self.after_id)
            self.after_id = None
        self.stop_worker()
        self.audio_output.stop()
        self.audio_started = False
        self.sdr.close()
        self.set_status("Adquisición detenida.")

    def stop_worker(self):
        if self.acquisition_worker is not None:
            self.acquisition_worker.stop()
            self.acquisition_worker = None

    def schedule_ui_update(self, delay_ms=20):
        self.after_id = self.root.after(delay_ms, self.update_ui_once)

    def update_ui_once(self):
        if not self.running:
            return

        try:
            self.start_audio_when_ready()

            now = time.monotonic()
            if self.iq_buffer.available_samples() >= self.config.nperseg and now - self.last_psd_update >= 0.15:
                freqs_mhz, self.psd_db_avg = compute_psd(
                    self.iq_buffer.latest(self.config.psd_buffer_samples),
                    self.config,
                    self.psd_db_avg,
                )
                self.psd_line.set_data(freqs_mhz, self.psd_db_avg)
                self.psd_canvas.draw_idle()
                self.last_psd_update = now

            if now - self.last_audio_plot_update >= 0.04:
                self.draw_latest_audio()
                self.audio_canvas.draw_idle()
                self.last_audio_plot_update = now

            if self.audio_started and now - self.last_status_update >= 0.5:
                self.update_stream_status()
                self.last_status_update = now

            self.schedule_ui_update(delay_ms=20)
        except Exception as exc:
            self.running = False
            self.stop_worker()
            self.audio_output.stop()
            self.sdr.close()
            self.show_error(exc)

    def start_audio_when_ready(self):
        if self.audio_started:
            return

        prebuffer_frames = self.config.audio_prebuffer_frames
        if self.audio_buffer.available_frames() < prebuffer_frames:
            self.set_status(
                f"Llenando buffer de audio: {self.audio_buffer.available_frames()}/{prebuffer_frames} muestras."
            )
            return

        self.audio_output.start(
            self.config.audio_rate_hz,
            block_frames=self.config.audio_output_block_frames,
            latency_s=self.config.audio_output_latency_s,
        )
        self.audio_started = True
        self.update_stream_status()

    def update_stream_status(self):
        stats = self.audio_buffer.stats()
        buffer_ms = stats["frames"] / self.config.audio_rate_hz * 1000.0
        dropped_ms = stats["dropped_frames"] / self.config.audio_rate_hz * 1000.0
        self.set_status(
            f"Adquiriendo: FC={self.config.fc_mhz:.3f} MHz, Span={self.config.span_mhz:.3f} MHz. "
            f"Buffer={buffer_ms:.0f} ms, underruns={stats['underruns']}, descartes={dropped_ms:.0f} ms."
        )

    def store_latest_audio(self, audio):
        if audio.size == 0:
            return

        max_plot_samples = self.config.audio_rate_hz // 20
        with self.last_audio_lock:
            self.last_audio = audio[-min(len(audio), max_plot_samples) :]

    def draw_latest_audio(self):
        with self.last_audio_lock:
            audio = self.last_audio.copy()

        audio_t_ms = np.arange(len(audio)) / self.config.audio_rate_hz * 1000.0
        self.audio_line.set_data(audio_t_ms, audio)
        self.audio_ax.set_xlim(0, max(1.0, audio_t_ms[-1] if len(audio_t_ms) else 1.0))

    def handle_worker_error(self, exc):
        self.root.after(0, lambda error=exc: self.stop_with_error(error))

    def stop_with_error(self, exc):
        self.running = False
        self.stop_worker()
        self.audio_output.stop()
        self.sdr.close()
        self.show_error(exc)

    def verify_sdr(self):
        if self.running:
            self.set_status("Detén la adquisición antes de verificar el SDR.")
            return
        if not self.apply_parameters(update_device=False):
            return

        def read_test():
            sdr = SDRDevice()
            try:
                self.root.after(0, lambda: self.set_status("Verificando lectura SDR..."))
                sdr.open(self.config)
                samples = sdr.read_samples(self.config.audio_block_samples)
                message = f"SDR OK: {len(samples)} muestras leídas."
                self.root.after(0, lambda: self.set_status(message))
            except Exception as exc:
                self.root.after(0, lambda error=exc: self.show_error(error))
            finally:
                sdr.close()

        threading.Thread(target=read_test, daemon=True).start()

    def test_audio(self):
        def play_tone():
            try:
                self.audio_output.play_test_tone(self.config.audio_rate_hz)
                self.root.after(0, lambda: self.set_status("Prueba de audio terminada."))
            except Exception as exc:
                self.root.after(0, lambda error=exc: self.show_error(error))

        self.set_status("Reproduciendo tono de prueba...")
        threading.Thread(target=play_tone, daemon=True).start()

    def set_status(self, message):
        self.status_var.set(message)

    def show_error(self, exc):
        message = str(exc)
        self.set_status(f"Error: {message}")
        messagebox.showerror("RTL-SDR", message)

    def on_close(self):
        self.stop()
        self.root.destroy()


def run_app():
    root = tk.Tk()
    app = SDRApp(root)
    root.app = app
    root.mainloop()

from dataclasses import dataclass


@dataclass
class SDRConfig:
    fc_mhz: float = 100.0
    span_mhz: float = 2.88
    nperseg: int = 4096
    noverlap: int = 2048

    gain_db: float = 22.9
    channel_rate_hz: int = 288_000
    audio_rate_hz: int = 48_000
    audio_block_duration_s: float = 0.0256
    psd_buffer_samples: int = 256 * 1024
    psd_y_min: float = -150.0
    psd_y_max: float = -20.0
    psd_alpha: float = 0.35
    audio_volume: float = 0.8
    deemphasis_us: float = 50.0
    audio_buffer_duration_s: float = 1.0
    audio_prebuffer_duration_s: float = 0.25
    audio_output_block_duration_s: float = 0.02
    audio_output_latency_s: float = 0.12

    @property
    def center_freq_hz(self):
        return self.fc_mhz * 1e6

    @property
    def sample_rate_hz(self):
        return self.span_mhz * 1e6

    @property
    def audio_block_samples(self):
        raw_samples = int(round(self.sample_rate_hz * self.audio_block_duration_s))
        multiple = 512
        return max(multiple, int(round(raw_samples / multiple)) * multiple)

    @property
    def audio_buffer_frames(self):
        return int(round(self.audio_rate_hz * self.audio_buffer_duration_s))

    @property
    def audio_prebuffer_frames(self):
        return int(round(self.audio_rate_hz * self.audio_prebuffer_duration_s))

    @property
    def audio_output_block_frames(self):
        raw_frames = int(round(self.audio_rate_hz * self.audio_output_block_duration_s))
        return max(128, raw_frames)

    def validate(self):
        if not 24.0 <= self.fc_mhz <= 1766.0:
            raise ValueError("FC debe estar entre 24 y 1766 MHz.")
        if not 0.25 <= self.span_mhz <= 3.2:
            raise ValueError("Span debe estar entre 0.25 y 3.2 MHz para RTL-SDR.")
        if self.nperseg < 256:
            raise ValueError("NFFT/Nperseg debe ser mayor o igual a 256.")
        if self.nperseg > self.psd_buffer_samples:
            raise ValueError("NFFT/Nperseg no puede ser mayor que el buffer PSD interno.")
        if self.noverlap < 0:
            raise ValueError("noverlap no puede ser negativo.")
        if self.noverlap >= self.nperseg:
            raise ValueError("noverlap debe ser menor que NFFT/Nperseg.")
        if self.channel_rate_hz >= self.sample_rate_hz:
            raise ValueError("La tasa de canal debe ser menor que el Span/sample rate.")
        if self.audio_block_samples % 512 != 0:
            raise ValueError("audio_block_samples debe ser múltiplo de 512.")
        if self.psd_buffer_samples % 256 != 0:
            raise ValueError("psd_buffer_samples debe ser múltiplo de 256.")
        if self.audio_buffer_frames < self.audio_prebuffer_frames:
            raise ValueError("El buffer de audio debe ser mayor o igual que el prebuffer.")
        if self.audio_prebuffer_frames < self.audio_output_block_frames * 2:
            raise ValueError("El prebuffer de audio debe cubrir al menos dos bloques de salida.")

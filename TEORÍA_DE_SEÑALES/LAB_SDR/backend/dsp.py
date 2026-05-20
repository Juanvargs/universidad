import math

import numpy as np
from scipy.signal import butter, resample_poly, sosfilt, sosfilt_zi, welch


def compute_psd(samples, config, previous_psd=None):
    freqs, psd = welch(
        samples,
        fs=config.sample_rate_hz,
        window="hann",
        nperseg=config.nperseg,
        noverlap=config.noverlap,
        return_onesided=False,
        scaling="density",
    )

    freqs = np.fft.fftshift(freqs)
    psd = np.fft.fftshift(psd)

    rf_freq_mhz = (config.center_freq_hz + freqs) / 1e6
    psd_db = 10 * np.log10(np.maximum(psd, 1e-20))

    if previous_psd is None:
        smoothed = psd_db
    else:
        smoothed = config.psd_alpha * psd_db + (1.0 - config.psd_alpha) * previous_psd

    return rf_freq_mhz, smoothed


class FMDemodulator:
    def __init__(self):
        self.deemphasis_state = 0.0
        self.audio_filter_sos = None
        self.audio_filter_state = None
        self.audio_level = 0.15
        self.audio_rate_hz = None
        self.channel_rate_hz = None
        self.last_channel_sample = None
        self.channel_filter_sos = None
        self.channel_filter_state = None
        self.channel_decim = None
        self.channel_sample_index = 0
        self.audio_decim_filter_sos = None
        self.audio_decim_filter_state = None
        self.audio_decim = None
        self.deemphasis_tau = 50e-6

    def reset(self, config):
        self.audio_rate_hz = config.audio_rate_hz
        self.channel_rate_hz = config.channel_rate_hz
        self.deemphasis_state = 0.0
        self.audio_level = 0.15
        self.deemphasis_tau = config.deemphasis_us * 1e-6
        self.last_channel_sample = None
        self.channel_sample_index = 0
        self._configure_channelizer(config)
        self._configure_audio_decimator(config)
        self.audio_filter_sos = butter(
            4,
            [40.0, 16_000.0],
            btype="bandpass",
            fs=config.audio_rate_hz,
            output="sos",
        )
        self.audio_filter_state = sosfilt_zi(self.audio_filter_sos) * 0.0

    def demodulate(self, iq_samples, config):
        if len(iq_samples) < 2:
            return np.array([], dtype=np.float32)

        if self.audio_rate_hz != config.audio_rate_hz or self.channel_rate_hz != config.channel_rate_hz:
            self.reset(config)

        channel_iq = self._channelize(iq_samples, config)
        if len(channel_iq) < 2:
            return np.array([], dtype=np.float32)

        if self.last_channel_sample is None:
            phase_input = channel_iq
        else:
            phase_input = np.concatenate(([self.last_channel_sample], channel_iq))

        self.last_channel_sample = channel_iq[-1]
        phase_diff = np.angle(phase_input[1:] * np.conj(phase_input[:-1]))
        phase_diff = phase_diff - np.mean(phase_diff)

        audio = self._resample_audio(phase_diff, config)
        if audio.size == 0:
            return np.array([], dtype=np.float32)

        audio = audio - np.mean(audio)
        audio = self._filter_audio(audio)
        audio = self._deemphasis(audio, config.audio_rate_hz)
        audio = self._agc(audio, config.audio_volume)
        return np.clip(audio, -1.0, 1.0).astype(np.float32)

    def _channelize(self, iq_samples, config):
        sample_rate_hz = int(round(config.sample_rate_hz))
        if self.channel_decim is None:
            gcd = math.gcd(sample_rate_hz, config.channel_rate_hz)
            return resample_poly(iq_samples, config.channel_rate_hz // gcd, sample_rate_hz // gcd)

        filtered, self.channel_filter_state = sosfilt(
            self.channel_filter_sos,
            iq_samples,
            zi=self.channel_filter_state,
        )
        start = (-self.channel_sample_index) % self.channel_decim
        self.channel_sample_index += len(iq_samples)
        return filtered[start:: self.channel_decim]

    def _resample_audio(self, fm_baseband, config):
        if self.audio_decim is None:
            gcd = math.gcd(config.channel_rate_hz, config.audio_rate_hz)
            return resample_poly(fm_baseband, config.audio_rate_hz // gcd, config.channel_rate_hz // gcd)

        filtered, self.audio_decim_filter_state = sosfilt(
            self.audio_decim_filter_sos,
            fm_baseband,
            zi=self.audio_decim_filter_state,
        )
        return filtered[:: self.audio_decim]

    def _configure_channelizer(self, config):
        sample_rate_hz = int(round(config.sample_rate_hz))
        if sample_rate_hz % config.channel_rate_hz != 0:
            self.channel_decim = None
            self.channel_filter_sos = None
            self.channel_filter_state = None
            return

        self.channel_decim = sample_rate_hz // config.channel_rate_hz
        cutoff_hz = min(120_000.0, 0.88 * (config.channel_rate_hz / 2))
        self.channel_filter_sos = butter(
            6,
            cutoff_hz,
            btype="lowpass",
            fs=sample_rate_hz,
            output="sos",
        )
        self.channel_filter_state = np.zeros((self.channel_filter_sos.shape[0], 2), dtype=np.complex128)

    def _configure_audio_decimator(self, config):
        if config.channel_rate_hz % config.audio_rate_hz != 0:
            self.audio_decim = None
            self.audio_decim_filter_sos = None
            self.audio_decim_filter_state = None
            return

        self.audio_decim = config.channel_rate_hz // config.audio_rate_hz
        self.audio_decim_filter_sos = butter(
            6,
            17_000.0,
            btype="lowpass",
            fs=config.channel_rate_hz,
            output="sos",
        )
        self.audio_decim_filter_state = sosfilt_zi(self.audio_decim_filter_sos) * 0.0

    def _filter_audio(self, audio):
        filtered, self.audio_filter_state = sosfilt(
            self.audio_filter_sos,
            audio,
            zi=self.audio_filter_state,
        )
        return filtered

    def _deemphasis(self, audio, audio_rate_hz):
        tau = self.deemphasis_tau
        dt = 1.0 / audio_rate_hz
        alpha = dt / (tau + dt)
        output = np.empty_like(audio, dtype=np.float64)
        y = self.deemphasis_state

        for index, sample in enumerate(audio):
            y = y + alpha * (sample - y)
            output[index] = y

        self.deemphasis_state = float(y)
        return output

    def _agc(self, audio, volume):
        rms = float(np.sqrt(np.mean(audio * audio))) if audio.size else 0.0
        if rms > 1e-6:
            if rms > self.audio_level:
                self.audio_level = 0.85 * self.audio_level + 0.15 * rms
            else:
                self.audio_level = 0.995 * self.audio_level + 0.005 * rms

        gain = volume * 0.08 / max(self.audio_level, 1e-4)
        gain = min(gain, 5.0)
        return audio * gain

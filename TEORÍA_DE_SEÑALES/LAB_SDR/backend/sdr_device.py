from rtlsdr import RtlSdr


class SDRDevice:
    def __init__(self):
        self.sdr = None

    @property
    def is_open(self):
        return self.sdr is not None

    def open(self, config):
        self.close()
        self.sdr = RtlSdr()
        self.configure(config)

    def configure(self, config):
        if self.sdr is None:
            return

        self.sdr.sample_rate = config.sample_rate_hz
        self.sdr.center_freq = config.center_freq_hz
        self.sdr.gain = config.gain_db

    def read_samples(self, sample_count):
        if self.sdr is None:
            raise RuntimeError("La RTL-SDR no está abierta.")

        samples = self.sdr.read_samples(sample_count)
        return samples - samples.mean()

    def read_samples_async(self, callback, sample_count):
        if self.sdr is None:
            raise RuntimeError("La RTL-SDR no está abierta.")

        def wrapped_callback(samples, context):
            del context
            callback(samples - samples.mean())

        self.sdr.read_samples_async(wrapped_callback, sample_count)

    def cancel_async_read(self):
        if self.sdr is not None:
            try:
                self.sdr.cancel_read_async()
            except Exception:
                pass

    def close(self):
        if self.sdr is not None:
            self.sdr.close()
            self.sdr = None

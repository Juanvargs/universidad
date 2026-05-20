# Auditoria de buffers y demodulacion en tiempo real

## Contexto

El software usa un RTL-SDR para adquirir muestras IQ, calcula PSD para
visualizacion y demodula FM en tiempo real para reproducir audio. El audio
entrecortado suele aparecer cuando la salida de audio consume muestras mas
rapido de lo que el hilo de adquisicion/DSP las produce, o cuando hay jitter por
bloques, latencia del sistema operativo, WSL, USB o carga de CPU.

## Hallazgos principales

1. El buffer de audio no registraba underruns.
   - Antes, si el callback de audio pedia mas muestras de las disponibles,
     `AudioBuffer.read()` completaba con ceros.
   - Eso evita una excepcion, pero se escucha como corte y no deja diagnostico.

2. El prebuffer inicial era bajo.
   - `4096` frames a 48 kHz son aproximadamente 85 ms.
   - Para RTL-SDR + Python + `sounddevice`, especialmente en WSL, conviene dar
     mas margen. Se subio a 250 ms por configuracion.

3. El callback de audio era muy frecuente.
   - `blocksize=480` equivale a 10 ms a 48 kHz.
   - Eso reduce latencia, pero aumenta la probabilidad de underrun si el sistema
     tiene jitter. Se paso a 20 ms por defecto.

4. El tamano maximo del buffer estaba fijo.
   - `24_000` frames equivale a 0.5 s a 48 kHz.
   - Ahora se deriva de `SDRConfig.audio_buffer_duration_s`, por defecto 1 s.

5. La decimacion puede producir bloques de audio ligeramente variables.
   - Con `span_mhz=2.88`, `channel_rate_hz=288000` y `audio_rate_hz=48000`, los
     factores nominales son adecuados: 10:1 y 6:1.
   - Sin embargo, `audio_block_samples` se redondea a multiplos de 512 para el
     RTL-SDR. Eso no siempre coincide perfectamente con los factores de
     decimacion, asi que el numero de frames de audio por iteracion puede variar
     levemente. El buffer debe absorber esa variacion.

6. No habia telemetria visible.
   - La UI no mostraba cantidad de buffer, underruns ni descartes.
   - Sin esos datos era dificil saber si el problema era DSP, USB, audio del
     sistema o configuracion.

## Cambios implementados

- `backend/config.py`
  - Agrega parametros:
    - `audio_buffer_duration_s = 1.0`
    - `audio_prebuffer_duration_s = 0.25`
    - `audio_output_block_duration_s = 0.02`
    - `audio_output_latency_s = 0.12`
  - Agrega propiedades derivadas en frames.
  - Valida que el prebuffer sea coherente con el bloque de salida.

- `backend/buffers.py`
  - `AudioBuffer` ahora cuenta:
    - underruns
    - frames faltantes por underrun
    - frames descartados por overflow
    - pico de ocupacion del buffer
  - Agrega `configure()` y `stats()`.

- `backend/audio_output.py`
  - `AudioOutput.start()` acepta `block_frames` y `latency_s`.
  - El stream usa bloques de 20 ms y latencia objetivo de 120 ms por defecto.
  - Si PortAudio no acepta la latencia numerica, reintenta con `latency="high"`.

- `frontend/interface.py`
  - Configura el tamano del buffer desde `SDRConfig`.
  - Usa prebuffer de 250 ms antes de iniciar audio.
  - Espera a que el prebuffer se llene; ya no inicia por timeout con el buffer
    bajo.
  - Muestra en estado:
    - buffer actual en ms
    - numero de underruns
    - audio descartado por overflow en ms

## Como interpretar la nueva telemetria

- `underruns = 0`: el buffer no se esta vaciando. Si aun hay cortes, revisar
  driver/audio del sistema.
- `underruns` sube: el callback se queda sin audio. Conviene aumentar
  `audio_prebuffer_duration_s`, `audio_output_latency_s` o reducir carga de CPU.
- `descartes` sube mucho: el productor va mas rapido que la salida o el buffer
  es demasiado pequeno. El audio no deberia cortarse por falta de muestras, pero
  puede aumentar la latencia o perder continuidad si se descarta audio viejo.
- `Buffer` muy cerca de 0 ms: riesgo alto de cortes.
- `Buffer` estable entre 150 y 600 ms: operacion normalmente sana para streaming.

## Recomendaciones adicionales para RTL-SDR

- Mantener `span_mhz=2.88` si el equipo lo soporta de forma estable; si aparecen
  perdidas USB, probar `2.4` o `2.048`.
- Evitar graficas PSD con `nperseg` excesivamente alto si el CPU esta justo.
- Usar puertos USB directos, no hubs saturados.
- Si se usa WSL, probar tambien ejecucion nativa en Linux cuando el audio o USB
  muestren jitter alto.

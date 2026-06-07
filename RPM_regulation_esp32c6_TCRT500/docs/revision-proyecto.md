# Revision tecnica del proyecto RPM Motor Control

Este documento resume el estado actual del firmware, los riesgos principales y la conectividad HTTP agregada sin romper la ruta de control PID que ya existe.

## Resumen del sistema

El proyecto implementa un control de velocidad para motor DC sobre ESP32-C6. La ruta principal es:

1. `rpm.c` cuenta pulsos del sensor por interrupcion y calcula RPM filtradas.
2. `control.c` lee la medicion, compara contra la referencia y ejecuta el PID.
3. `pid.c` calcula una salida limitada al rango PWM de 8 bits.
4. `motor.c` aplica direccion y duty al driver del motor.
5. `oled.c` muestra RPM medida, objetivo y barra de duty.
6. `uart_cmd.c` permite modificar la referencia por UART.

La separacion por modulos es buena para un proyecto embebido: cada archivo tiene una responsabilidad clara y la referencia de RPM ya esta encapsulada con `control_set_setpoint()` y `control_get_setpoint()`. La interfaz HTTP reutiliza esa frontera y lee un snapshot seguro con `control_get_status()`.

## Fortalezas

- Arquitectura modular y facil de seguir.
- Control loop periodico con `SAMPLE_TIME_MS` centralizado en `config.h`.
- Setpoint y telemetria protegidos con seccion critica, utiles para la tarea HTTP concurrente.
- Comandos UART ya validan rango minimo y maximo antes de aplicar cambios.
- OLED no bloquea la inicializacion completa si falla; el firmware continua operando.
- El README documenta hardware, pinout, comandos, calibracion y flujo de trabajo.

## Riesgos y mejoras recomendadas

### Control PID

- `pid_compute()` satura la salida entre `0` y `255`, por lo que la rama de direccion negativa en `control_task()` nunca se usa realmente.
- No hay anti-windup: si la referencia queda lejos de la medicion durante mucho tiempo, el termino integral puede acumularse aunque la salida ya este saturada.
- `dt` se pasa fijo como `0.1`, pero el periodo real depende de `SAMPLE_TIME_MS`. Conviene calcularlo desde la constante o medir tiempo real.
- Los comentarios tienen caracteres mal codificados, por ejemplo `SaturaciÃ³n`. Conviene guardar los archivos como UTF-8 o usar ASCII en comentarios.

### Medicion de RPM

- `pulse_count` se modifica en ISR y se lee/reinicia en tarea sin una seccion critica. Puede perder pulsos o leer valores inconsistentes.
- `gpio_install_isr_service(0)` puede fallar si otro modulo ya instalo el servicio. Conviene manejar `ESP_ERR_INVALID_STATE`.
- El rechazo fijo de pulsos menores a 2 ms limita la velocidad maxima medible. Con `PULSES_PER_REV = 1`, 2 ms equivale a un maximo teorico de 30000 RPM, pero con encoders de mas pulsos por vuelta el limite baja rapidamente.
- No se configuran pull-up/pull-down para `PIN_SENSOR`; depende totalmente del hardware externo.

### Motor y seguridad

- `motor_set_direction()` no contempla `dir == 0`. Para una parada segura podria ponerse ambos pines en bajo o usar una funcion explicita `motor_stop()`.
- No hay rampa de aceleracion ni limite de cambio de duty. Algunos drivers y fuentes pueden sufrir con escalones bruscos.
- No hay watchdog de referencia: si una interfaz remota queda escribiendo valores invalidos o muy cambiantes, solo el rango protege el sistema.

### Configuracion del proyecto

- `main/Kconfig.projbuild` todavia contiene opciones de ejemplo `BLINK_*` que no se usan en el firmware actual.
- `main/idf_component.yml` declara `espressif/led_strip`, pero el codigo actual no usa tiras LED.
- `main/CMakeLists.txt` ya declara los componentes de red necesarios para WiFi, HTTP, NVS, eventos y JSON.

## Apartado de conectividad HTTP

La implementacion agrega un servidor HTTP embebido en el ESP32-C6. La interfaz HTTP reutiliza las funciones existentes de control, especialmente `control_set_setpoint()`, `control_get_setpoint()` y `control_get_status()`. El endpoint no llama `rpm_get()` directamente para no alterar la cadencia del loop PID.

### Objetivo

Permitir monitoreo y control basico desde una red local:

- Consultar RPM medida.
- Consultar referencia configurada.
- Consultar duty aplicado, si se expone desde `control.c`.
- Cambiar la referencia de RPM con validacion de rango.
- Verificar salud del firmware con un endpoint simple.

### Endpoints implementados

| Metodo | Ruta | Funcion |
| --- | --- | --- |
| `GET` | `/api/status` | Devuelve estado del sistema: RPM medida, referencia, limites y estado de control |
| `GET` | `/api/rpm` | Devuelve solo la medicion actual de RPM |
| `GET` | `/api/setpoint` | Devuelve la referencia actual |
| `POST` | `/api/setpoint` | Cambia la referencia de RPM |
| `GET` | `/health` | Responde si el servidor esta vivo |

Ejemplo de respuesta para `GET /api/status`:

```json
{
  "rpm": 842.15,
  "setpoint": 1000.0,
  "min_setpoint": 0.0,
  "max_setpoint": 9000.0
}
```

Ejemplo de cuerpo para `POST /api/setpoint`:

```json
{
  "rpm": 1500
}
```

### Cambios tecnicos implementados

1. Se agrego `wifi_connect.c` / `wifi_connect.h` para inicializar NVS, `esp_netif`, event loop y modo station.
2. Se agrego `http_server.c` / `http_server.h` basado en `esp_http_server`.
3. Se agregaron credenciales por Kconfig y `sdkconfig.defaults`, evitando hardcodearlas en `config.h`.
4. Se agregaron dependencias a `main/CMakeLists.txt`:

```cmake
REQUIRES
    driver
    esp_timer
    esp_wifi
    esp_netif
    esp_event
    esp_http_server
    nvs_flash
    json
```

5. Se inicializa red y servidor HTTP en `app_main()` si el SSID esta configurado:

```c
if (wifi_connect_sta() == ESP_OK) {
    ESP_ERROR_CHECK(rpm_http_server_start());
}
```

6. UART se mantiene como canal local de respaldo. HTTP no reemplaza la interfaz serial durante pruebas.

### Seguridad minima recomendada

- No exponer el ESP32 directamente a Internet.
- Mantenerlo solo en red local o VLAN de laboratorio.
- Validar rango de `rpm` usando los mismos limites que UART.
- Responder `400 Bad Request` ante JSON invalido o valores fuera de rango.
- Considerar un token simple por header, por ejemplo `X-Api-Key`, si la red es compartida.
- Registrar cambios de setpoint por HTTP con `ESP_LOGI`.

### Concurrencia

`control_set_setpoint()` protege la referencia con `portMUX_TYPE`, asi que puede ser llamada desde una tarea HTTP. Para exponer datos del loop se agrego `control_get_status()`, que entrega RPM, setpoint, salida PID, duty y limites en una sola lectura segura.

### Orden de implementacion sugerido

1. Probar `idf.py build` con el entorno ESP-IDF cargado.
2. Configurar SSID/password con `idf.py menuconfig`.
3. Flashear y leer la IP desde el monitor serial.
4. Probar la pagina web desde el navegador.
5. Probar los endpoints con `curl` desde la misma red.

## Pruebas sugeridas

- Compilar con `idf.py build` despues de cada cambio de dependencias.
- Validar UART: `rpm 1500`, `set 0`, `ref=9000`, valores fuera de rango.
- Validar RPM con generador de pulsos antes de conectar el motor.
- Validar HTTP con:

```bash
curl http://ESP_IP/health
curl http://ESP_IP/api/status
curl -X POST http://ESP_IP/api/setpoint -H "Content-Type: application/json" -d "{\"rpm\":1500}"
```

- Confirmar que el loop PID sigue estable mientras se consultan endpoints repetidamente.

## Conclusion

El proyecto queda encaminado para control local por UART, visualizacion OLED y monitoreo/control HTTP en red local. La integracion HTTP usa endpoints pequenos, validacion estricta y funciones de control existentes para no duplicar reglas de negocio.

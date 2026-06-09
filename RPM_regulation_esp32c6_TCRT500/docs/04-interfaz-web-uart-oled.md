# Interfaz web, UART y OLED

El sistema se puede controlar y monitorear por tres vias: monitor serial, pagina web y pantalla OLED.

## UART

Archivo:

```text
main/uart_cmd.c
```

Configuracion:

```c
#define UART_CMD_PORT UART_NUM_0
#define UART_CMD_BAUD 115200
```

Comandos aceptados:

```text
rpm 500
set 1000
ref=3000
rpm 0
```

La funcion `parse_rpm_command()` acepta los prefijos `rpm`, `set` y `ref`. Luego valida que el valor este dentro del rango:

```c
if (end == line || *end != '\0' || value < MIN_RPM_SETPOINT || value > MAX_RPM_SETPOINT) {
    return false;
}
```

Si el comando es valido, se aplica:

```c
control_set_setpoint(rpm);
```

## Pagina web

Archivo:

```text
main/http_server.c
```

La web esta embebida como cadenas HTML, CSS y JavaScript dentro del firmware. Permite ver:

- RPM actual.
- Referencia.
- `PIDOUT`.
- `CMD`.
- `DUTY`.
- Error.
- Modo `STARTING` o `RUNNING`.

Tambien permite aplicar una referencia nueva con un formulario.

## API HTTP

Rutas registradas:

```c
const httpd_uri_t routes[] = {
    {.uri = "/", .method = HTTP_GET, .handler = index_get_handler},
    {.uri = "/health", .method = HTTP_GET, .handler = health_get_handler},
    {.uri = "/api/status", .method = HTTP_GET, .handler = status_get_handler},
    {.uri = "/api/setpoint", .method = HTTP_GET, .handler = setpoint_get_handler},
    {.uri = "/api/setpoint", .method = HTTP_POST, .handler = setpoint_post_handler},
};
```

### GET `/api/status`

Devuelve:

```json
{
  "rpm": 500.00,
  "setpoint": 500.00,
  "output": 0.00,
  "command": 190.00,
  "duty": 190.00,
  "min_setpoint": 0.00,
  "max_setpoint": 6000.00,
  "mode": "RUNNING"
}
```

Los datos salen de:

```c
control_get_status(&status);
```

Esto evita que la web lea directamente el sensor o interfiera con el control.

### POST `/api/setpoint`

Recibe:

```json
{
  "rpm": 1000
}
```

Valida el rango:

```c
if (rpm < MIN_RPM_SETPOINT || rpm > MAX_RPM_SETPOINT) {
    return bad_request(req, "rpm fuera de rango");
}
```

Aplica:

```c
control_set_setpoint(rpm);
```

## WiFi

Archivo:

```text
main/wifi_connect.c
```

Las credenciales se configuran con:

```powershell
idf.py menuconfig
```

Opciones:

```text
RPM Motor Control
  WiFi SSID
  WiFi password
  WiFi max retry
```

Si el SSID esta vacio, el firmware conserva UART, OLED y control PID, pero no inicia HTTP:

```c
if (strlen(CONFIG_RPM_WIFI_SSID) == 0) {
    ESP_LOGW(TAG, "CONFIG_RPM_WIFI_SSID esta vacio; HTTP queda deshabilitado");
    return ESP_ERR_INVALID_ARG;
}
```

## OLED

Archivo:

```text
main/oled.c
```

La pantalla se actualiza en una tarea separada:

```c
xTaskCreate(oled_task, "oled_task", 4096, NULL, 3, NULL);
```

La tarea consulta el estado del control:

```c
control_status_t status;
control_get_status(&status);
oled_show_tachometer(status.rpm, status.setpoint, status.duty);
```

Esto separa la visualizacion del control principal.

## Frecuencia de actualizacion

La OLED tiene:

```c
#define OLED_REFRESH_INTERVAL_US 100000
```

Eso equivale a:

```text
100000 us = 100 ms
```

La web consulta el estado cada:

```js
setInterval(poll,250);
```

Es decir:

```text
250 ms
```

La web no es el lazo de control; solo muestra datos y envia referencias. El control del motor sigue ejecutandose en `control_task()`.

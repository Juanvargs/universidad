# Documentacion tecnica del proyecto

Esta carpeta contiene la explicacion detallada del sistema de control de RPM. El README principal resume el proyecto; estos documentos desarrollan cada tema con calculos, parametros y referencias al codigo.

## Documentos

| Documento | Contenido |
| --- | --- |
| [01-hardware-conexiones.md](01-hardware-conexiones.md) | Componentes, pinout, L293D, sensor y OLED |
| [02-medicion-rpm-sensor.md](02-medicion-rpm-sensor.md) | Calculo de RPM, interrupciones, filtros y rechazo de ruido |
| [03-control-pid-pwm.md](03-control-pid-pwm.md) | PID, feedforward, PWM, duty, arranque y rampa progresiva |
| [04-interfaz-web-uart-oled.md](04-interfaz-web-uart-oled.md) | Uso de UART, pagina web, API HTTP y OLED |
| [05-pruebas-calibracion.md](05-pruebas-calibracion.md) | Procedimiento de pruebas, datos a registrar y criterios de ajuste |

## Flujo general

```text
Sensor TCRT5000 -> rpm.c -> control.c -> pid.c -> control.c -> motor.c -> L293D -> Motor
                                      -> oled.c
                                      -> http_server.c
                                      -> uart_cmd.c
```

El sistema mide, filtra, compara, calcula una correccion y aplica PWM al motor. La OLED y la web muestran el estado, pero no deben alterar la cadencia del lazo principal de control.

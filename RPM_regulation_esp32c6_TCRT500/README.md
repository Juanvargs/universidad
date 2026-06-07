# RPM Motor Control

ESP-IDF firmware for closed-loop DC motor speed control on an ESP32-C6. The project measures motor speed from a pulse sensor, filters the RPM signal, computes a PID correction, and drives a motor driver through PWM and two direction pins.

The application is intentionally split into small modules so the control path is easy to inspect and tune:

- `main.c` initializes the motor driver, RPM sensor, and FreeRTOS control task.
- `motor.c` configures LEDC PWM output and GPIO direction control.
- `rpm.c` counts sensor pulses with a GPIO interrupt and converts them to RPM.
- `pid.c` implements a simple proportional-integral-derivative controller.
- `control.c` runs the periodic feedback loop and applies the PID output to the motor.
- `oled.c` drives a 128x64 SSD1306 OLED over I2C and renders the tachometer view.
- `uart_cmd.c` reads serial commands to update the desired RPM reference.
- `config.h` centralizes pin assignments and control constants.

## Features

- Closed-loop RPM regulation using a PID controller.
- PWM motor speed control through the ESP-IDF LEDC driver.
- Direction control using two digital GPIO outputs.
- Interrupt-based pulse counting for RPM measurement.
- Basic pulse debounce/noise rejection in the interrupt handler.
- Exponential smoothing filter for RPM readings.
- Periodic logging of measured RPM and controller output.
- SSD1306 OLED tachometer interface with measured and target RPM.
- UART command input for changing the RPM setpoint while the firmware is running.
- WiFi station mode with an embedded HTTP server for browser-based monitoring and setpoint control.
- Modern embedded web UI with RPM history, live gauge, JSON API, CSS, JavaScript, and SVG icon.

## Technical Review and HTTP Connectivity

A deeper Spanish-language review is available in [`docs/revision-proyecto.md`](docs/revision-proyecto.md). It covers the firmware architecture, PID and RPM measurement risks, configuration cleanup items, and the HTTP connectivity design.

HTTP support is implemented with a lightweight ESP-IDF HTTP server on the ESP32-C6. UART remains available as the local fallback interface. The web app is served directly by the firmware:

| Route | Content |
| --- | --- |
| `/` | Main HTML dashboard |
| `/styles.css` | Embedded modern CSS |
| `/app.js` | Browser-side polling, setpoint form, and RPM history chart |
| `/icon.svg` | SVG icon and favicon |

The JSON API exposes:

| Method | Route | Purpose |
| --- | --- | --- |
| `GET` | `/health` | Check that the firmware HTTP server is alive |
| `GET` | `/api/status` | Read measured RPM, current setpoint, and control limits |
| `GET` | `/api/setpoint` | Read the current RPM reference |
| `POST` | `/api/setpoint` | Update the RPM reference after range validation |

The RPM history is maintained in the browser, so the ESP32 only serves the latest control snapshot. Configure WiFi credentials with `idf.py menuconfig` under `RPM Motor Control`; if the SSID is empty, the firmware keeps running without HTTP.

## Hardware

The firmware is configured for an ESP32-C6 target and expects:

- ESP32-C6 development board.
- DC motor.
- H-bridge or motor driver with one PWM input and two direction inputs.
- RPM sensor that generates one or more digital pulses per motor revolution.
- External motor power supply sized for the motor and driver.
- Common ground between the ESP32-C6, motor driver, sensor, and motor supply.

## Pinout

Default pin assignments are defined in `main/config.h`.

| Signal | GPIO | Description |
| --- | ---: | --- |
| `PIN_PWM` | 4 | PWM output to the motor driver speed input |
| `PIN_IN1` | 5 | Motor driver direction input 1 |
| `PIN_IN2` | 6 | Motor driver direction input 2 |
| `PIN_SENSOR` | 1 | RPM sensor pulse input |
| `OLED_I2C_SDA` | 8 | SSD1306 I2C data |
| `OLED_I2C_SCL` | 9 | SSD1306 I2C clock |

The RPM input is configured to trigger on the negative edge. If your sensor produces active-high pulses or needs pull-up/pull-down configuration, adjust `rpm_init()` in `main/rpm.c`.

The OLED defaults to I2C address `0x3C` at 400 kHz. If your display uses address `0x3D`, update `OLED_I2C_ADDR` in `main/config.h`.

## Control Parameters

The main control constants live in `main/config.h`.

| Constant | Current value | Purpose |
| --- | ---: | --- |
| `PWM_FREQ` | `20000` | PWM frequency in Hz |
| `PWM_RES` | `LEDC_TIMER_8_BIT` | PWM resolution, giving a duty range from 0 to 255 |
| `SAMPLE_TIME_MS` | `100` | Control loop period |
| `PULSES_PER_REV` | `1` | Sensor pulses generated per full motor revolution |
| `DEFAULT_RPM_SETPOINT` | `1000.0f` | Initial desired RPM reference |
| `MAX_RPM_SETPOINT` | `9000.0f` | Maximum accepted desired RPM for the 9 V DC motor |

The PID gains are currently set in `main/control.c`:

```c
pid_init(&pid, 0.5, 0.1, 0.01);
```

Tune `Kp`, `Ki`, `Kd`, and the RPM reference according to your motor, load, sensor resolution, and power stage. The controller output is limited to the 8-bit PWM range used by the LEDC channel.

## UART RPM Command

Open the ESP-IDF serial monitor and send one of these commands followed by Enter:

```text
rpm 1500
set 1500
ref=1500
```

The value is interpreted as the desired RPM reference. Valid values are from `0` to `9000` RPM.

## HTTP Web Dashboard

After configuring WiFi and flashing the firmware, open the IP printed in the serial monitor. The dashboard provides:

- Live RPM gauge.
- Current setpoint, PID output, and PWM duty.
- RPM history chart refreshed every second.
- Numeric input for the desired RPM reference.
- Embedded SVG icon section and favicon.

You can also use the API directly:

```bash
curl http://ESP_IP/health
curl http://ESP_IP/api/status
curl -X POST http://ESP_IP/api/setpoint -H "Content-Type: application/json" -d "{\"rpm\":1500}"
```

## How It Works

1. `app_main()` initializes the motor module and sets the default direction.
2. `rpm_init()` configures the sensor GPIO interrupt and starts the measurement timer.
3. `control_task()` runs every `SAMPLE_TIME_MS`.
4. The task reads the filtered RPM value from `rpm_get()`.
5. `pid_compute()` compares the measured RPM against the setpoint and calculates a PWM duty command.
6. The motor direction and speed are applied with `motor_set_direction()` and `motor_set_speed()`.
7. The OLED displays a tachometer needle, measured RPM, target RPM, and a small duty bar.
8. The control module stores a thread-safe status snapshot for HTTP reads.
9. If WiFi is configured, the firmware starts the HTTP server and serves the dashboard/API.
10. The firmware logs the current RPM, target RPM, raw PID output, and duty value through the ESP-IDF logging system.

RPM is calculated from the number of pulses accumulated over the elapsed time:

```text
RPM = (pulse_count / elapsed_seconds) * (60 / PULSES_PER_REV)
```

An exponential filter is applied to reduce measurement jitter:

```text
filtered = alpha * current_rpm + (1 - alpha) * previous_filtered
```

The current filter coefficient is `0.2` in `main/rpm.c`.

## Build and Flash

Install and export ESP-IDF before building. This project was last generated with ESP-IDF `5.5.3` and target `esp32c6`, according to `dependencies.lock`.

Set the target:

```bash
idf.py set-target esp32c6
```

Build the firmware:

```bash
idf.py build
```

Flash and monitor the board:

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for your board, for example `COM5` on Windows or `/dev/ttyUSB0` on Linux. To leave the monitor, press `Ctrl+]`.

## Expected Serial Output

During operation, the control task prints periodic feedback similar to:

```text
I (1234) PID: RPM: 842.15 | REF: 1000.00 | OUT: 78.42 | DUTY: 78
```

The values depend on the motor, sensor, load, PID tuning, and setpoint.

## Project Layout

```text
.
|-- CMakeLists.txt
|-- dependencies.lock
|-- main
|   |-- CMakeLists.txt
|   |-- config.h
|   |-- control.c / control.h
|   |-- main.c
|   |-- motor.c / motor.h
|   |-- oled.c / oled.h
|   |-- pid.c / pid.h
|   |-- uart_cmd.c / uart_cmd.h
|   |-- wifi_connect.c / wifi_connect.h
|   |-- http_server.c / http_server.h
|   `-- rpm.c / rpm.h
`-- sdkconfig.defaults*
```

## Calibration Notes

- Set `PULSES_PER_REV` to match the physical sensor or encoder.
- Confirm the interrupt edge in `rpm_init()` matches the sensor output.
- Adjust or remove the 2 ms pulse rejection window in `isr_handler()` if the motor can produce valid pulses faster than that.
- Start PID tuning with a low `Kp`, then introduce `Ki` and `Kd` gradually.
- Make sure the motor driver accepts 3.3 V ESP32 GPIO logic, or use level shifting if required.
- Verify that the motor power supply can handle startup current without resetting the ESP32-C6.

## Troubleshooting

- No RPM reading: check the sensor wiring, signal level, interrupt edge, and `PULSES_PER_REV`.
- Motor does not move: check the driver enable pins, motor supply, PWM pin, and direction pins.
- Unstable speed: reduce PID gains, increase filtering, or improve sensor signal quality.
- RPM is scaled incorrectly: update `PULSES_PER_REV` to the actual number of pulses per revolution.
- ESP32 resets when the motor starts: separate logic and motor power appropriately and keep grounds common.

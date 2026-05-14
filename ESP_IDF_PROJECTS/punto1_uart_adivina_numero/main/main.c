#include <stdio.h>                 // Permite usar printf(), sscanf() y snprintf() para texto por consola serial.
#include <stdbool.h>               // Permite usar el tipo bool con los valores true y false.
#include <stdint.h>                // Permite usar tipos enteros exactos como uint32_t.
#include "freertos/FreeRTOS.h"     // Permite usar constantes de FreeRTOS como portTICK_PERIOD_MS.
#include "freertos/task.h"         // Permite usar vTaskDelay() para evitar bloquear el sistema.
#include "driver/gpio.h"           // Permite configurar y leer pines GPIO como el boton BOOT.
#include "driver/usb_serial_jtag.h" // Permite leer bytes directamente desde el USB Serial/JTAG de la ESP32-C6.
#include "driver/usb_serial_jtag_vfs.h" // Permite conectar printf() con el driver USB Serial/JTAG.
#include "esp_random.h"            // Permite usar esp_random() para generar numeros aleatorios en la ESP32-C6.

#define LINE_BUFFER_SIZE 64        // Define el tamano maximo de cada linea recibida desde el monitor serial.
#define BOOT_BUTTON_GPIO GPIO_NUM_9 // Define GPIO9 como pin del boton BOOT en placas ESP32-C6 DevKit.

typedef enum {                     // Crea un tipo enumerado para indicar que evento ocurrio al esperar entrada.
    INPUT_LINE_RECEIVED,           // Indica que se recibio una linea completa desde el Serial Monitor.
    INPUT_BOOT_PRESSED             // Indica que se presiono el boton BOOT mientras se esperaba entrada.
} input_result_t;                  // Define el nombre input_result_t para usar este tipo en las funciones.

static void print_welcome(void)    // Declara una funcion que muestra el mensaje inicial del juego.
{                                  // Inicia el bloque de instrucciones de la funcion print_welcome.
    printf("\n");                  // Imprime una linea en blanco para separar el arranque del texto del juego.
    printf("=== Juego de Adivinar el Numero por UART ===\n"); // Muestra el titulo principal del proyecto.
    printf("ESP32-C6 con ESP-IDF\n"); // Indica la placa y el entorno usados en la implementacion.
    printf("Ingrese dos numeros separados por espacio para definir el rango.\n"); // Explica como iniciar una partida.
    printf("Ejemplo: 10 50\n");   // Muestra un ejemplo valido para el rango minimo y maximo.
    printf("Luego ingrese el numero maximo de intentos para esa ronda.\n"); // Explica que el limite de intentos sera definido por el usuario.
    printf("Ejemplo intentos: 4\n"); // Muestra un ejemplo valido para el numero maximo de intentos.
}                                  // Finaliza el bloque de instrucciones de la funcion print_welcome.

static void configure_usb_serial(void) // Declara una funcion que inicializa el driver USB Serial/JTAG.
{                                  // Inicia el bloque de instrucciones de la funcion configure_usb_serial.
    if (!usb_serial_jtag_is_driver_installed()) { // Verifica si el driver USB Serial/JTAG aun no esta instalado.
        usb_serial_jtag_driver_config_t usb_config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT(); // Crea la configuracion por defecto del driver.
        usb_serial_jtag_driver_install(&usb_config); // Instala el driver para poder leer y escribir por USB Serial/JTAG.
    }                              // Cierra la validacion de driver no instalado.

    usb_serial_jtag_vfs_use_driver(); // Hace que printf() use el driver USB Serial/JTAG instalado.
}                                  // Finaliza el bloque de instrucciones de la funcion configure_usb_serial.

static void configure_boot_button(void) // Declara una funcion que prepara el boton BOOT para lectura digital.
{                                  // Inicia el bloque de instrucciones de la funcion configure_boot_button.
    gpio_reset_pin(BOOT_BUTTON_GPIO); // Limpia configuraciones previas del GPIO9 antes de usarlo como entrada.
    gpio_set_direction(BOOT_BUTTON_GPIO, GPIO_MODE_INPUT); // Configura GPIO9 como entrada digital.
    gpio_set_pull_mode(BOOT_BUTTON_GPIO, GPIO_PULLUP_ONLY); // Activa pull-up interno porque BOOT normalmente conecta el pin a GND.
}                                  // Finaliza el bloque de instrucciones de la funcion configure_boot_button.

static bool boot_button_is_pressed(void) // Declara una funcion que detecta si el boton BOOT esta presionado.
{                                  // Inicia el bloque de instrucciones de la funcion boot_button_is_pressed.
    return gpio_get_level(BOOT_BUTTON_GPIO) == 0; // Retorna true cuando GPIO9 esta en bajo por accion del boton BOOT.
}                                  // Finaliza el bloque de instrucciones de la funcion boot_button_is_pressed.

static void wait_for_boot_release(void) // Declara una funcion que espera a que el usuario suelte el boton BOOT.
{                                  // Inicia el bloque de instrucciones de la funcion wait_for_boot_release.
    while (boot_button_is_pressed()) { // Repite mientras el boton BOOT siga presionado.
        vTaskDelay(pdMS_TO_TICKS(10)); // Espera 10 ms para no ocupar el procesador innecesariamente.
    }                              // Finaliza el ciclo de espera de liberacion.
}                                  // Finaliza el bloque de instrucciones de la funcion wait_for_boot_release.

static bool boot_press_confirmed(void) // Declara una funcion que detecta una pulsacion estable de BOOT.
{                                  // Inicia el bloque de instrucciones de la funcion boot_press_confirmed.
    if (!boot_button_is_pressed()) { // Verifica si BOOT no esta presionado en este instante.
        return false;              // Retorna false porque no hay pulsacion.
    }                              // Cierra la validacion de boton suelto.

    vTaskDelay(pdMS_TO_TICKS(40)); // Espera 40 ms para filtrar rebotes mecanicos.

    if (!boot_button_is_pressed()) { // Verifica si la pulsacion desaparecio despues del antirrebote.
        return false;              // Retorna false porque era ruido o rebote.
    }                              // Cierra la validacion posterior al antirrebote.

    wait_for_boot_release();       // Espera a que el usuario suelte BOOT para aceptar futuras pulsaciones.
    return true;                   // Retorna true porque se detecto una pulsacion valida.
}                                  // Finaliza el bloque de instrucciones de la funcion boot_press_confirmed.

static bool read_start_line(char *buffer, size_t buffer_size) // Declara una funcion que espera BOOT o una linea serial inicial.
{                                  // Inicia el bloque de instrucciones de la funcion read_start_line.
    size_t index = 0;              // Crea el indice donde se guardaran caracteres recibidos antes del mensaje inicial.

    buffer[0] = '\0';              // Inicializa el buffer como cadena vacia.

    while (true) {                 // Inicia un ciclo que espera una accion de inicio.
        if (boot_press_confirmed()) { // Verifica si el usuario presiono BOOT de forma estable.
            return false;          // Retorna false porque no hay una linea serial precargada.
        }                          // Cierra la validacion del boton BOOT.

        uint8_t character = 0;     // Crea una variable para recibir un byte desde USB Serial/JTAG.
        int bytes_read = usb_serial_jtag_read_bytes(&character, 1, pdMS_TO_TICKS(10)); // Intenta leer un byte sin bloquear mucho tiempo.

        if (bytes_read <= 0) {     // Verifica si no llego ningun byte desde el Serial Monitor.
            continue;              // Vuelve a revisar BOOT y el puerto serial.
        }                          // Cierra la validacion de ausencia de bytes.

        if (character == '\r' || character == '\n') { // Verifica si el usuario envio Enter.
            if (index == 0) {      // Verifica si Enter llego sin texto previo.
                return false;      // Retorna false para iniciar mostrando el mensaje sin rango precargado.
            }                      // Cierra la validacion de linea vacia.

            buffer[index] = '\0';  // Termina la cadena recibida desde serial.
            return true;           // Retorna true porque se recibio una linea que puede servir como rango.
        }                          // Cierra la validacion de Enter.

        if (index < buffer_size - 1) { // Verifica que el buffer tenga espacio para guardar otro caracter.
            buffer[index] = (char)character; // Guarda el caracter recibido en el buffer.
            index++;              // Avanza el indice hacia la siguiente posicion libre.
        }                          // Cierra la validacion de espacio del buffer.
    }                              // Finaliza el ciclo de espera de inicio.
}                                  // Finaliza el bloque de instrucciones de la funcion read_start_line.

static input_result_t read_line(char *buffer, size_t buffer_size) // Declara una funcion que lee una linea o detecta BOOT.
{                                  // Inicia el bloque de instrucciones de la funcion read_line.
    size_t index = 0;              // Crea el indice donde se guardara el siguiente caracter recibido.

    while (true) {                 // Inicia un ciclo que espera caracteres hasta recibir Enter.
        if (boot_press_confirmed()) { // Verifica si el usuario presiono BOOT de forma estable.
            buffer[0] = '\0';      // Limpia el buffer porque no se recibio una linea serial.
            return INPUT_BOOT_PRESSED; // Informa a la funcion llamadora que BOOT fue presionado.
        }                          // Cierra la revision del boton BOOT.

        uint8_t character = 0;     // Crea una variable para recibir un byte desde USB Serial/JTAG.
        int bytes_read = usb_serial_jtag_read_bytes(&character, 1, pdMS_TO_TICKS(10)); // Intenta leer un byte sin bloquear mucho tiempo.

        if (bytes_read <= 0) {     // Verifica si aun no llego ningun byte desde el Serial Monitor.
            continue;              // Vuelve a intentar leer otro caracter.
        }                          // Cierra la validacion de ausencia temporal de bytes.

        if (character == '\r' || character == '\n') { // Verifica si el usuario presiono Enter.
            if (index == 0) {      // Verifica si el salto de linea llego antes de escribir datos.
                continue;          // Ignora saltos de linea vacios o el segundo caracter de CRLF.
            }                      // Cierra la validacion de linea vacia.

            buffer[index] = '\0';  // Termina la cadena de texto recibida.
            return INPUT_LINE_RECEIVED; // Retorna que ya se recibio una linea completa.
        }                          // Cierra la validacion de Enter.

        if (index < buffer_size - 1) { // Verifica que aun exista espacio para guardar el caracter.
            buffer[index] = (char)character; // Guarda el caracter recibido dentro del buffer.
            index++;              // Avanza el indice para el siguiente caracter.
        }                          // Cierra la validacion de espacio disponible.
    }                              // Finaliza el ciclo de lectura de caracteres.
}                                  // Finaliza el bloque de instrucciones de la funcion read_line.

static bool parse_range(const char *line, int *range_min, int *range_max) // Declara una funcion que extrae el rango desde texto.
{                                  // Inicia el bloque de instrucciones de la funcion parse_range.
    char extra = '\0';             // Crea una variable para detectar texto adicional no permitido.
    int values_read = sscanf(line, " %d %d %c", range_min, range_max, &extra); // Intenta leer minimo, maximo y posible basura extra.

    if (values_read != 2) {        // Verifica que se hayan recibido exactamente dos numeros enteros.
        return false;              // Retorna false si el formato no coincide con dos numeros.
    }                              // Cierra la validacion de cantidad de valores leidos.

    if (*range_min >= *range_max) { // Verifica que el valor minimo sea menor que el valor maximo.
        return false;              // Retorna false si el rango esta invertido o no tiene amplitud.
    }                              // Cierra la validacion de orden del rango.

    return true;                   // Retorna true si el rango tiene formato y valores correctos.
}                                  // Finaliza el bloque de instrucciones de la funcion parse_range.

static bool parse_guess(const char *line, int *guess) // Declara una funcion que extrae un intento desde texto.
{                                  // Inicia el bloque de instrucciones de la funcion parse_guess.
    char extra = '\0';             // Crea una variable para detectar caracteres adicionales despues del intento.
    int values_read = sscanf(line, " %d %c", guess, &extra); // Intenta leer un numero entero y posible texto extra.

    if (values_read != 1) {        // Verifica que se haya recibido exactamente un numero entero.
        return false;              // Retorna false si el intento no tiene el formato esperado.
    }                              // Cierra la validacion de cantidad de valores leidos.

    return true;                   // Retorna true si el intento fue convertido correctamente.
}                                  // Finaliza el bloque de instrucciones de la funcion parse_guess.

static bool parse_attempt_limit(const char *line, int *attempt_limit) // Declara una funcion que extrae el limite de intentos desde texto.
{                                  // Inicia el bloque de instrucciones de la funcion parse_attempt_limit.
    char extra = '\0';             // Crea una variable para detectar caracteres adicionales despues del numero.
    int values_read = sscanf(line, " %d %c", attempt_limit, &extra); // Intenta leer un entero y posible texto extra.

    if (values_read != 1) {        // Verifica que se haya recibido exactamente un numero entero.
        return false;              // Retorna false si el formato no contiene un unico numero.
    }                              // Cierra la validacion de cantidad de valores leidos.

    if (*attempt_limit <= 0) {     // Verifica que el limite de intentos sea mayor que cero.
        return false;              // Retorna false si el numero de intentos no es positivo.
    }                              // Cierra la validacion de intentos positivos.

    return true;                   // Retorna true si el limite de intentos es valido.
}                                  // Finaliza el bloque de instrucciones de la funcion parse_attempt_limit.

static int generate_secret_number(int range_min, int range_max) // Declara una funcion que genera el numero secreto.
{                                  // Inicia el bloque de instrucciones de la funcion generate_secret_number.
    uint32_t range_size = (uint32_t)(range_max - range_min + 1); // Calcula cuantos numeros posibles existen en el rango.
    uint32_t random_offset = esp_random() % range_size; // Obtiene un desplazamiento aleatorio dentro del rango permitido.
    return range_min + (int)random_offset; // Devuelve el numero secreto ajustado al minimo del rango.
}                                  // Finaliza el bloque de instrucciones de la funcion generate_secret_number.

static void request_valid_range(int *range_min, int *range_max, const char *initial_line) // Declara una funcion que pide el rango hasta que sea valido.
{                                  // Inicia el bloque de instrucciones de la funcion request_valid_range.
    char line[LINE_BUFFER_SIZE];   // Crea un arreglo para guardar la linea escrita por el usuario.
    bool use_initial_line = initial_line != NULL && initial_line[0] != '\0'; // Indica si ya existe una linea recibida antes del mensaje.

    while (true) {                 // Inicia un ciclo que se repite hasta obtener un rango valido.
        if (use_initial_line) {    // Verifica si debe procesar primero la linea recibida al iniciar.
            snprintf(line, sizeof(line), "%s", initial_line); // Copia la linea inicial al buffer local.
            use_initial_line = false; // Marca la linea inicial como consumida para no repetirla.
        } else {                   // Ejecuta esta rama cuando no hay linea inicial pendiente.
            printf("\nEscriba el rango minimo y maximo, luego presione Enter.\n"); // Indica al usuario que debe enviar una linea completa.
            printf("Rango minimo y maximo: "); // Solicita al usuario escribir el rango de la partida.

            input_result_t input_result = read_line(line, sizeof(line)); // Espera una linea serial o una pulsacion de BOOT.

            if (input_result == INPUT_BOOT_PRESSED) { // Verifica si el usuario presiono BOOT mientras esperaba el rango.
                print_welcome();   // Vuelve a mostrar el mensaje inicial para que aparezca en el Serial Monitor reabierto.
                continue;          // Vuelve al inicio del ciclo para pedir el rango otra vez.
            }                      // Cierra la validacion de lectura del rango.
        }                          // Cierra la validacion de lectura del rango.

        if (!parse_range(line, range_min, range_max)) { // Convierte la linea a dos numeros y verifica si el rango es valido.
            printf("Rango invalido. Use dos enteros con minimo menor que maximo. Ejemplo: 10 50\n"); // Explica el formato correcto.
            continue;              // Vuelve al inicio del ciclo para pedir el rango otra vez.
        }                          // Cierra la validacion del formato y contenido del rango.

        return;                    // Sale de la funcion cuando ya existe un rango valido.
    }                              // Finaliza el ciclo de solicitud de rango.
}                                  // Finaliza el bloque de instrucciones de la funcion request_valid_range.

static void request_valid_attempt_limit(int *attempt_limit) // Declara una funcion que pide el numero de intentos hasta que sea valido.
{                                  // Inicia el bloque de instrucciones de la funcion request_valid_attempt_limit.
    char line[LINE_BUFFER_SIZE];   // Crea un arreglo para guardar la linea escrita por el usuario.

    while (true) {                 // Inicia un ciclo que se repite hasta obtener un limite valido.
        printf("\nEscriba el numero maximo de intentos, luego presione Enter.\n"); // Indica al usuario que debe enviar el limite.
        printf("Numero de intentos: "); // Solicita al usuario escribir cuantos intentos tendra la ronda.

        input_result_t input_result = read_line(line, sizeof(line)); // Espera una linea serial o una pulsacion de BOOT.

        if (input_result == INPUT_BOOT_PRESSED) { // Verifica si el usuario presiono BOOT mientras esperaba el limite.
            print_welcome();       // Vuelve a mostrar el mensaje inicial para que aparezca en el Serial Monitor reabierto.
            continue;              // Vuelve al inicio del ciclo para pedir el numero de intentos otra vez.
        }                          // Cierra la validacion del boton BOOT.

        if (!parse_attempt_limit(line, attempt_limit)) { // Convierte la linea a entero y verifica si el limite es valido.
            printf("Numero de intentos invalido. Use un entero mayor que cero. Ejemplo: 4\n"); // Explica el formato correcto.
            continue;              // Vuelve al inicio del ciclo para pedir el numero de intentos otra vez.
        }                          // Cierra la validacion del limite de intentos.

        return;                    // Sale de la funcion cuando ya existe un numero de intentos valido.
    }                              // Finaliza el ciclo de solicitud de intentos.
}                                  // Finaliza el bloque de instrucciones de la funcion request_valid_attempt_limit.

static void play_round(int range_min, int range_max, int max_attempts) // Declara una funcion que ejecuta una partida completa.
{                                  // Inicia el bloque de instrucciones de la funcion play_round.
    char line[LINE_BUFFER_SIZE];   // Crea un arreglo para guardar cada intento escrito por el usuario.
    int attempts = 0;              // Inicializa el contador de intentos realizados.
    int secret_number = generate_secret_number(range_min, range_max); // Genera el numero que el jugador debe adivinar.

    printf("\nJuego iniciado.\n"); // Informa que la partida ya comenzo.
    printf("Adivine el numero entre %d y %d.\n", range_min, range_max); // Muestra el rango valido de intentos.
    printf("Tiene %d intentos para acertar.\n", max_attempts); // Informa cuantos intentos validos permite la ronda.

    while (attempts < max_attempts) { // Inicia el ciclo principal mientras queden intentos disponibles.
        int guess = 0;             // Crea una variable para guardar el intento actual del usuario.

        printf("Intento: ");       // Solicita al usuario escribir un intento.

        input_result_t input_result = read_line(line, sizeof(line)); // Espera una linea serial o una pulsacion de BOOT.

        if (input_result == INPUT_BOOT_PRESSED) { // Verifica si el usuario presiono BOOT mientras esperaba un intento.
            print_welcome();       // Vuelve a mostrar el encabezado para que aparezca en el Serial Monitor reabierto.
            printf("\nJuego iniciado.\n"); // Recuerda que la partida actual sigue activa.
            printf("Adivine el numero entre %d y %d.\n", range_min, range_max); // Vuelve a mostrar el rango actual.
            printf("Intentos usados: %d de %d.\n", attempts, max_attempts); // Muestra cuantos intentos validos ya se usaron.
            continue;              // Repite el ciclo para pedir otro intento.
        }                          // Cierra la validacion de lectura del intento.

        if (!parse_guess(line, &guess)) { // Convierte el texto recibido a un numero entero.
            printf("Intento invalido. Escriba un solo numero entero.\n"); // Indica que el formato del intento no es correcto.
            continue;              // Repite el ciclo para pedir otro intento.
        }                          // Cierra la validacion de formato del intento.

        if (guess < range_min || guess > range_max) { // Verifica si el intento esta fuera del rango definido.
            printf("Intento fuera de rango. Debe estar entre %d y %d.\n", range_min, range_max); // Recuerda los limites validos.
            continue;              // Repite el ciclo sin contar el intento fuera de rango.
        }                          // Cierra la validacion de rango del intento.

        attempts++;                // Aumenta el contador porque el intento fue valido.

        if (guess == secret_number) { // Compara el intento con el numero secreto para detectar acierto.
            printf("Correcto. Adivinaste el numero en %d intentos.\n", attempts); // Felicita al usuario y muestra los intentos.
            return;                // Termina la partida actual para permitir iniciar otra.
        }                          // Cierra la validacion de acierto.

        if (guess < secret_number) { // Verifica si el intento fue menor que el numero secreto.
            printf("El numero secreto es mayor.\n"); // Da una pista indicando que debe probar un numero mas alto.
        } else {                   // Ejecuta esta rama si el intento fue mayor que el numero secreto.
            printf("El numero secreto es menor.\n"); // Da una pista indicando que debe probar un numero mas bajo.
        }                          // Cierra la seleccion de pista mayor o menor.
    }                              // Finaliza el ciclo principal de intentos.

    printf("Se acabaron los %d intentos.\n", max_attempts); // Informa que el jugador uso todos los intentos permitidos.
    printf("El numero secreto era %d.\n", secret_number); // Muestra el numero secreto antes de volver a pedir rango.
}                                  // Finaliza el bloque de instrucciones de la funcion play_round.

void app_main(void)                // Define la funcion principal que ESP-IDF ejecuta al iniciar el programa.
{                                  // Inicia el bloque de instrucciones de la funcion app_main.
    int range_min = 0;             // Crea la variable donde se guardara el limite inferior del rango.
    int range_max = 0;             // Crea la variable donde se guardara el limite superior del rango.
    int max_attempts = 0;          // Crea la variable donde se guardara el numero maximo de intentos por ronda.
    char start_line[LINE_BUFFER_SIZE]; // Crea un buffer para guardar una linea enviada antes del mensaje inicial.
    bool has_start_line = false;   // Indica si el usuario envio una linea inicial por serial.

    setvbuf(stdin, NULL, _IONBF, 0); // Desactiva el buffer de entrada para leer datos seriales de forma directa.
    setvbuf(stdout, NULL, _IONBF, 0); // Desactiva el buffer de salida para mostrar mensajes inmediatamente.

    configure_usb_serial();        // Configura el USB Serial/JTAG como canal principal de comunicacion.
    configure_boot_button();       // Configura el boton BOOT para iniciar el mensaje desde GPIO9.
    has_start_line = read_start_line(start_line, sizeof(start_line)); // Espera BOOT, Enter o una linea enviada por Serial Monitor.
    print_welcome();               // Muestra el mensaje inicial del juego por el monitor serial.

    while (true) {                 // Mantiene el programa ejecutandose para permitir partidas infinitas.
        request_valid_range(&range_min, &range_max, has_start_line ? start_line : NULL); // Pide un rango valido antes de iniciar.
        has_start_line = false;    // Desactiva la linea inicial para que las siguientes rondas pidan rango normalmente.
        request_valid_attempt_limit(&max_attempts); // Pide el numero maximo de intentos para la ronda actual.
        play_round(range_min, range_max, max_attempts); // Ejecuta la partida usando el rango y los intentos seleccionados.
        printf("\nIngrese un nuevo rango y un nuevo numero de intentos para jugar otra vez.\n"); // Indica como comenzar una nueva ronda.
    }                              // Finaliza el ciclo infinito del programa.
}                                  // Finaliza el bloque de instrucciones de la funcion app_main.

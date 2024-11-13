/*
Guia2 - Ejercicio 3 "Reloj despertador"

Este código implementa un reloj despertador utilizando el mismo circuito
implementado en ejercicios anteriores, sólo con el agregado de un buzzer.

Se conecta un display 4x7-segmentos de la siguiente forma:
Los segmentos estan conectados así:
PB0 - A
PB1 - B
PB2 - C
PB3 - D
PB4 - E
PB5 - F
PB6 - G

Los digitos se conectan de la siguiente forma:
PB7 - DIG4 (indica las unidades de mil)
PB8 - DIG3 (indica las centenas)
PB9 - DIG2 (indica las decenas)
PB10 - DIG1 (indica las unidades)


El pulsador esta conectado al PB16 y es utilizado para lanzar una interrupcion
que conmuta el estado de activación del alarma. Se utiliza el LED de la placa
para indicar si el alarma esta activa.

El buzzer esta conectado al PB17 y (al menos este buzzer) se activa por
nivel bajo.

*/


#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "pico/time.h"
#include "hardware/rtc.h"


// Definición del pulsador (PB16) y buzzer (PB17)
#define PIN_PULSADOR 16
#define LED_ALARM 25
#define PIN_BUZZER 17

// Estado del reloj y alarma
typedef struct {
    datetime_t tiempo_actual;
    datetime_t alarma;
    bool alarma_activa;
} reloj_t;

reloj_t reloj = {
    .tiempo_actual = { .hour = 18, .min = 30, .sec = 0 },
    .alarma = { .hour = 18, .min = 31, .sec = 0 },
    .alarma_activa = true
};

int digitos[4];
// Codificación de los dígitos para 7 segmentos (común ánodo)
const uint8_t DIGITO_7SEG[] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};



// Prototipos de funciones
void configurar_pulsador();
void manejar_pulsador(uint gpio, uint32_t eventos);
bool actualizar_display(struct repeating_timer *t);
void verificar_alarma();

// Función principal
int main() {

    rtc_init();  // Inicializa el RTC
    configurar_pulsador();

    // Configuro los pines asociados al display 4x 7 segmentos
    gpio_init_mask(0b11111111111);
    gpio_set_dir_out_masked(0b11111111111);

    // Configuro el led que indicará si el alarma esta armada
    gpio_init(LED_ALARM);
    gpio_set_dir(LED_ALARM,true);

    // Configura la hora inicial (00:00:00)
    datetime_t tiempo_inicial = { 
        .year = 2024, .month = 10, .day = 29, 
        .hour = 18, .min = 30, .sec = 0,
        .dotw  = 3, // 0 is Sunday, so 5 is Friday
    };
    rtc_set_datetime(&tiempo_inicial);
    sleep_ms(10);

    // Configura el buzzer (opcional)
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);

    // Temporizador para actualizar el display cada 100 ms
    struct repeating_timer timer;
    add_repeating_timer_ms(-100, actualizar_display, NULL, &timer);

    while (1) {
    // Unidad de minutos
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,false);
        gpio_put(10,true);
        gpio_put_masked(0b1111111,DIGITO_7SEG[digitos[3]]);

    sleep_ms(4);

    // Decena de minutos
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,true);
        gpio_put(10,false);
        gpio_put_masked(0b1111111,DIGITO_7SEG[digitos[2]]);
        
    sleep_ms(4);

    // Unidad de Horas
        gpio_put(7,false);
        gpio_put(8,true);
        gpio_put(9,false);
        gpio_put(10,false);
        gpio_put_masked(0b1111111,DIGITO_7SEG[digitos[1]]);
        
    sleep_ms(4);

    // Decena de Horas
        gpio_put(7,true);
        gpio_put(8,false);
        gpio_put(9,false);
        gpio_put(10,false);
        gpio_put_masked(0b1111111,DIGITO_7SEG[digitos[0]]);
        
    sleep_ms(4);

    }
}

// Configuración del pulsador con interrupción
void configurar_pulsador() {
    gpio_init(PIN_PULSADOR);
    gpio_set_dir(PIN_PULSADOR, GPIO_IN);
    gpio_pull_down(PIN_PULSADOR);
    gpio_set_irq_enabled_with_callback(PIN_PULSADOR, GPIO_IRQ_EDGE_FALL, true, &manejar_pulsador);
}

// Manejador de la interrupción del pulsador
void manejar_pulsador(uint gpio, uint32_t eventos) {
    
    reloj.alarma_activa ^= 0x1;

}

// Actualiza el display con la hora actual
bool actualizar_display(struct repeating_timer *t) {
    rtc_get_datetime(&reloj.tiempo_actual);  // Obtiene la hora actual

    // Extrae los dígitos para cada posición
    digitos[0] = (int)(reloj.tiempo_actual.hour / 10) % 10;        // Decena de horas
    digitos[1] = reloj.tiempo_actual.hour % 10;        // Unidad de horas
    digitos[2] = (int)(reloj.tiempo_actual.min / 10) % 10;      // Decena de minutos
    digitos[3] = reloj.tiempo_actual.min % 10;      // Unidad de minutos

    // Verifica si la alarma esta armada y si debe sonar
    verificar_alarma();

    return true;
}


// Verifica si la hora actual coincide con la alarma
void verificar_alarma() {
    if (reloj.tiempo_actual.hour == reloj.alarma.hour &&
        reloj.tiempo_actual.min == reloj.alarma.min && reloj.alarma_activa) {
        gpio_put(PIN_BUZZER, 0);  // Activa el buzzer

    } else {
        gpio_put(PIN_BUZZER, 1);  // Apaga el buzzer
    }

    if(reloj.alarma_activa)
        gpio_put(LED_ALARM,true);
    else
        gpio_put(LED_ALARM,false);
}

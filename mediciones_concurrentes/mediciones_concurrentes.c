/*
Guia 3 - Ejercicio 2 "Mediciones Concurrentes"

El circuito implementado junto con el código desarrollado para este ejercicio
realiza la medicion de canales analógicos en modo free-running (y cambiando de 
canal mediante el mecanismo Round Robin). Las conversiones de los valores 
medidos son transformados a valores porcentuales (de 00 a 99).

Para mostrar los valores medidos se conecta un display 4x7-segmentos de la iguiente forma:
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

Se conectaron a la Raspberry Pico dos potenciometros (pertenecientes a una palanca
de joystick) en los pines PB26 y PB27, que corresponden con los canales 0 y 1 del 
ADC del RP2040.

*/


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

// ------------------------------------------------------------------------------
// Variables globales
// ------------------------------------------------------------------------------
float pote1, pote2;
bool medida_ok = false;
enum digito {UNIDAD = 10, DECENA=9, CENTENA=8, UNIDAD_MIL=7};

// ------------------------------------------------------------------------------
// Prototipo de funciones
// ------------------------------------------------------------------------------
void callback_adc();
void escribo(uint8_t);

// ------------------------------------------------------------------------------
// Funcion main
// ------------------------------------------------------------------------------

int main()
{

    int med1=0, med2=0;
// Variables correspondientes a los digitos del numero a mostrar
    int dig_m1u = 0, dig_m1d = 0, dig_m2u = 0, dig_m2d = 0;

// Configuro los pines conectados al display como salida        
    gpio_init_mask(0b11111111111);
    gpio_set_dir_out_masked(0b11111111111);

// Configuro el ADC, con los canales 0 y 1 como canales para coversión
// en modo free-running
    adc_gpio_init(26);
    adc_gpio_init(27);

    adc_init();
    adc_set_round_robin(0b00011);

    adc_fifo_setup(true,true,2,false,true);
    adc_set_clkdiv(2000);

    irq_set_exclusive_handler(ADC_IRQ_FIFO, callback_adc);
    irq_set_enabled(ADC_IRQ_FIFO, true);
    adc_irq_set_enabled(true);

    adc_run(true);

    
    while (true) {

        if(medida_ok)
        
        {
            
            // Se limita el valor sólo a un número entre 0 y 99
            med1 = (int)pote1 % 100;
            med2 = (int)pote2 % 100;

            // Se baja la bandera de dato disponible
            medida_ok = false;

        }

    // Extraigo los digitos del numero: unidad, decena, centena y unidad de mil
        dig_m1u = med1 %10;
        dig_m1d = (int)(med1/10) % 10;

        dig_m2u = med2 % 10;
        dig_m2d = (int)(med2/10) % 10;
        

    // Muestro unidad de Canal 0
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,true);
        escribo(dig_m1u);
    sleep_ms(4);

    // Muestro decena de Canal 0
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,true);
        gpio_put(UNIDAD,false);
        escribo(dig_m1d);
    sleep_ms(4);

    // Muestro unidad de Canal 1
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,true);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_m2u);
    sleep_ms(4);

    // Muestro decena de Canal 1
        gpio_put(UNIDAD_MIL,true);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_m2d);
    sleep_ms(4);
        


    }
}

// ------------------------------------------------------------------------------
// Cuerpo de las funciones
// ------------------------------------------------------------------------------

// Servicio de interrupcion cuando la FIFO del ADC recibe 2 conversiones
void callback_adc()
{
    const float conversion = 100.f / 256;

    pote1 = (float)adc_fifo_get() * conversion;
    pote2 = (float)adc_fifo_get() * conversion;

    // Se activa una bandera para indicar que ya se realizo la lectura
    medida_ok = true;

    // Limpiar la interrupción y la FIFO
    adc_fifo_drain();
    irq_clear(ADC_IRQ_FIFO);
}

// Funcion de escritura en display, utilizo "Lookup Table" para encontrar
// la codifiacion correspondiente de los segmentos. Tener en cuenta que
// trato con un display ánodo-común, por lo tanto la logica enciente con 0
void escribo(uint8_t digito)
{
    uint8_t lut[10] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};

    gpio_put_masked(0b1111111,lut[digito]);
}
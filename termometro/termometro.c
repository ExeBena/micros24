/*
Guia3 - Ejercicio 1 "Termómetro"

Este código tiene como objetivo medir la tempertura (utilizando el sensor interno)
y mostrarla en un display de 4x7segmentos.

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

*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"


enum digito {UNIDAD = 10, DECENA=9, CENTENA=8, UNIDAD_MIL=7};
float temperatura;

// Funcion de escritura en display, utilizo "Lookup Table" para encontrar
// la codifiacion correspondiente de los segmentos. Tener en cuenta que
// trato con un display ánodo-común, por lo tanto la logica enciente con 0
void escribo(uint8_t digito)
{
    uint8_t lut[10] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};

    gpio_put_masked(0b1111111,lut[digito]);
}

// Funcion callback de ejecución repetitiva. Dentro de esta función se hace 
// la lectura del ADC en modo one-shot y realiza el cálculo de la temperatura 
// según fórmula de la hoja de datos.
bool medicion(void)
{
    const float factor_conversion = 3.3f / (1 << 12);

    float medicion = (float)adc_read() * factor_conversion;

// se calcula la temperatura, al resultado se lo multiplica por 10 para 
// mostrar en el display hasta el primer digito despues de la coma.
    temperatura = (27.0f - (medicion - 0.706f) / 0.001721f)*10; 

    return true;
}



int main()
{
// Variables correspondientes a los digitos del numero a mostrar y la temperatura
    int dig_u = 0, dig_d = 0, dig_c = 0, dig_um = 0;

// Configuro los pines conectados al display como salida        
    gpio_init_mask(0b11111111111);
    gpio_set_dir_out_masked(0b11111111111);
// Inicializo el display
    gpio_put(7,false);
    gpio_put(8,false);
    gpio_put(9,false);
    gpio_put(10,false);

// Configuro canal y conversor ADC
    adc_init();

    adc_select_input(4); // Se selecciona el sensor de temperatura

    adc_set_temp_sensor_enabled(true);

// Configuro una interrupcion repetitiva de timer
    struct repeating_timer timer;
    add_repeating_timer_ms(3000, medicion, NULL, &timer);

    while (true) {

    // Se extraen los digitos del valor de la temperatura.
    // el primer digito de la derecha va a indicar las
    // decimas de grado. El segundo dígito la unidad de grado
    // y el tercer digito (contando desde la derecha) la decena.
        dig_u = (int)temperatura % 10;
        dig_d = (int)(temperatura / 10) % 10;
        dig_c = (int)(temperatura / 100) % 10;
        dig_um = (int)(temperatura / 1000) % 10;

    // Muestro decimas de grado
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,true);
        escribo(dig_u);
    sleep_ms(4);

    // Muestro unidad de grado
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,true);
        gpio_put(UNIDAD,false);
        escribo(dig_d);
    sleep_ms(4);

    // Muestro decena de grado
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,true);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_c);
    sleep_ms(4);

    }
}

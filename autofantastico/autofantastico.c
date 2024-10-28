/*
Guia 1 - Ejercicio 2 "Auto fantástico"
Este código realiza la secuencia de encendido/apagado de un grupo de 6 leds,
esta secuecia se corresponde con la típica secuencia del conocido 
"auto fantástico".

Los leds estan conectados a los siguientes pines:
PB0, PB1, PB2, PB3, PB4, PB5

La característica particular de este código es que se utilizan funciones del
tipo "masked", el SDK de Raspberry define funciones que permiten, iniciar, setear,
limpiar o conmutar pines mediante el uso de mascaras binarias.


*/


#include <stdio.h>
// #include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"


int main()
{

    gpio_init_mask(0b111111);
    gpio_set_dir_out_masked(0b111111);
    
    bool izquierda = true;
    uint32_t val = 1;

    while (true) {
        
        if(val == 0b100000)
        {
            izquierda = false;
        }
        else if (val == 0b1)
        {
            izquierda = true;
        }
        
        val = (izquierda == true) ? val << 1 : val>> 1;
        
        gpio_put_masked(0b111111, val);
        sleep_ms(100);



    }
}

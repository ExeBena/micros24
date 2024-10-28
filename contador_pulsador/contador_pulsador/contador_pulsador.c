/*
Guia1 - Ejercicio 3 "Contador"

Este código resuelve el problema del ejercicio 3.
Se conecta un display 7-segmentos de la siguiente forma:

PB0 - A
PB1 - B
PB3 - C
PB4 - D
PB5 - E
PB6 - F
PB7 - G

El pulsador esta conectado al PB16.

Las cuentas se realizan mediante interrupción de entrada por el pin PB16

 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"


// Funcion de escritura en display, utilizo "Lookup Table" para encontrar
// la codifiacion correspondiente de los segmentos. Tener en cuenta que
// trato con un display ánodo-común, por lo tanto la logica enciente con 0
void escribo(uint8_t digito)
{
    static uint8_t lut[10] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};

    gpio_put_masked(0b1111111,lut[digito]);
}


// Rutina de interrupcion, llevo una cuenta y muestro el valor
void pulsacion()
{
    static uint8_t contador = 0;

    contador ++;
    contador = contador % 10;

    escribo(contador);
}


int main()
{

// Configuro los pines conectados al display como salida        
    gpio_init_mask(0b1111111);
    gpio_set_dir_out_masked(0b1111111);
    
// Configuro el pin de entrada que será manejado por el botón
    gpio_init(16);
    gpio_set_dir(16,false);
    gpio_pull_down(16);

// A continuación habilito interrupciones y configuro su callback 
    gpio_set_irq_enabled(16,GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_callback(pulsacion);

    irq_set_enabled(IO_IRQ_BANK0,true);

// Inicializo el display
    escribo(0);

    while (true) {

// Espero por interrupcion
        __asm__("wfi");
        
    }
}

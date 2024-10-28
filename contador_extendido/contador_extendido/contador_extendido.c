/*
Guia1 - Ejercicio 4 "Contador extendido"

Este código resuelve el problema del ejercicio 4.
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


El pulsador esta conectado al PB16.

Las cuentas se realizan mediante interrupción de entrada por el pin PB16

 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"


enum digito {UNIDAD = 10, DECENA=9, CENTENA=8, UNIDAD_MIL=7};

// Contador global
int contador = 0;

// Funcion de escritura en display, utilizo "Lookup Table" para encontrar
// la codifiacion correspondiente de los segmentos. Tener en cuenta que
// trato con un display ánodo-común, por lo tanto la logica enciente con 0
void escribo(uint8_t digito)
{
    uint8_t lut[10] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};

    gpio_put_masked(0b1111111,lut[digito]);
}


// Rutina de interrupcion, con esta funcion se lleva la cuenta.
void pulsacion()
{

    contador ++;
    
}


int main()
{
// Variables correspondientes a los digitos del numero a mostrar
    int dig_u = 0, dig_d = 0, dig_c = 0, dig_um = 0;

// Configuro los pines conectados al display como salida        
    gpio_init_mask(0b11111111111);
    gpio_set_dir_out_masked(0b11111111111);
    
// Configuro el pin de entrada que será manejado por el botón
    gpio_init(16);
    gpio_set_dir(16,false);
    gpio_pull_down(16);

// A continuación habilito interrupciones y configuro su callback 
    gpio_set_irq_enabled(16,GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_callback(pulsacion);

    irq_set_enabled(IO_IRQ_BANK0,true);

// Inicializo el display
    gpio_put(7,false);
    gpio_put(8,false);
    gpio_put(9,false);
    gpio_put(10,false);


    while (true) {


/* En el loop se extraen cada uno de los dígitos y luego se muestran. 
Para extraer cada dígito del número del contador se van realizando divisiones 
y calculos de módulo para extraer la unidad basta con calcular el módulo 10
para extraer la decena hay que dividir el valor del contador por 10, para la 
centena por 100 y para al unidad de mil por 1000

Para mostrar los números en el display se van mostrando de a uno. Se muesta la
unidad se espera un tiempo de delay, luego se muestra la decena y así 
sucesivamente. La frecuencia de switcheo debe ser de aproximadamente 30Hz para
que el ojo humano no detecte el encendido y apagado de los distintos display.*/
    
    // Extraigo los digitos del numero: unidad, decena, centena y unidad de mil
        dig_u = contador % 10;
        dig_d = (int)(contador / 10) % 10;
        dig_c = (int)(contador / 100) % 10;
        dig_um = (int)(contador / 1000) % 10;


    // Muestro unidad
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,true);
        escribo(dig_u);
    sleep_ms(33/4);

    // Muestro decena
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,true);
        gpio_put(UNIDAD,false);
        escribo(dig_d);
    sleep_ms(33/4);

    // Muestro centena
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,true);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_c);
    sleep_ms(33/4);

    // Muestro unidad de mil
        gpio_put(UNIDAD_MIL,true);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_um);
    sleep_ms(33/4);
        
    }
}
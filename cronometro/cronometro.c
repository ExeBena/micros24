/*
Guia2 - Ejercicio 2 "Cronómetro"

Este código implementa un cronómetro utilizando el mismo circuito
implementado en ejercicios anteriores.

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
que cambia el estado del cronómetro, inicia, detiene o reestablece.


*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "pico/time.h"


struct repeating_timer timer;

struct tiempo{
    uint8_t minuto;
    uint8_t segundos;
    uint32_t decima;
};

struct tiempo crono= {0,0,0} ;

repeating_timer_callback_t alarm_callback(alarm_id_t id, void *user_data) {

    crono.decima ++;

}



enum digito {UNIDAD = 10, DECENA=9, CENTENA=8, UNIDAD_MIL=7};


// Funcion de escritura en display, utilizo "Lookup Table" para encontrar
// la codifiacion correspondiente de los segmentos. Tener en cuenta que
// trato con un display ánodo-común, por lo tanto la logica enciente con 0
void escribo(uint8_t digito)
{
    uint8_t lut[10] = {0b1000000,0b1111001, 0b0100100, 0b0110000, 0b0011001,
                    0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0011000};

    gpio_put_masked(0b1111111,lut[digito]);
}


// Rutina de interrupcion del pulsador, con esta funcion se inicia el
// cronometro, se detiene y se vuelve a cero.

void pulsacion()
{
static uint8_t estado;
    estado ++;
    estado %=3;


    switch (estado){
        case 0:
            crono.decima = 0;
            crono.segundos = 0;
            crono.minuto = 0;
            break;
        case 1:

            add_repeating_timer_ms(-100, alarm_callback, NULL, &timer);
            break;
        case 2:
            cancel_repeating_timer(&timer);

            break;
    }
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


// Seteamos la interrupcion del Timer
    // add_alarm_in_ms(100, alarm_callback, NULL, false);


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
    
    // Completo los segundo y los minutos, todos refereidos a las decimas de segundos
    // ya que las decimas son el contador libre.
        crono.segundos = (crono.decima/10)%60;
        crono.minuto = (crono.decima/600)%10;


    // Extraigo los digitos del numero: unidad, decena, centena y unidad de mil
        dig_u = crono.decima %10;

        dig_d = (int)(crono.segundos) % 10;
        dig_c = (int)(crono.segundos / 10) % 10;

        dig_um = crono.minuto%10;
        

    // Muestro decimas
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,true);
        escribo(dig_u);
    sleep_ms(33/4);

    // Muestro unidad de segundos
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,true);
        gpio_put(UNIDAD,false);
        escribo(dig_d);
    sleep_ms(33/4);

    // Muestro decena de segundos
        gpio_put(UNIDAD_MIL,false);
        gpio_put(CENTENA,true);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_c);
    sleep_ms(33/4);

    // Muestro minutos
        gpio_put(UNIDAD_MIL,true);
        gpio_put(CENTENA,false);
        gpio_put(DECENA,false);
        gpio_put(UNIDAD,false);
        escribo(dig_um);
    sleep_ms(33/4);
    }
}

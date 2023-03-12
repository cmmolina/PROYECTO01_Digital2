/*
 * File:   DHT11.c
 */

int timeout;
#include "DHT11.h"
#include <stdint.h>

void DHT11_Start(void){
    TRISDbits.TRISD0 = 0;                         // RD0 como OUTPUT
    PORTDbits.RD0 = 0;                            // RD0 en LOW
    __delay_ms(25);
    PORTDbits.RD0 = 1;                            // RD0 en HIGH
    __delay_us(25); 
    TRISDbits.TRISD0 = 1;                         // RD0 como INPUT
}

int DHT11_Response(void){
    timeout=0;
    TMR1 = 0; 
    T1CONbits.TMR1ON = 1;
    while(!PORTDbits.RD0 && TMR1 < 100);           // Esperamos hasta que RD0 esté en HIGH 
        if(TMR1 > 99){
            return 0;
        }                                          // Regresamos un 0 en caso haya un problema en cuanto a la respuesta del sensor
        else{
            TMR1 = 0;                              // Configuramos el valor del TMR1 a 0
            while(PORTDbits.RD0 && TMR1 < 100);    // Esperamos hasta que RD0 esté en LOW
            if(TMR1 > 99){                         // Si el tiempo de respuesta es mayor a 99us, entonces hay un error en la respuesta del sensor
                 return 0;
            }                                     
            else{
                return 1;
            }
        }
}

unsigned int DHT11_Read(void){
    uint8_t i;
    unsigned int _data = 0;
    
    if(timeout){
        ;
    }
    for(i = 0; i < 8; i++){
        TMR1 = 0;                                  // Configuramos el valor del TMR1 a 0
        while(!PORTDbits.RD0);                     // Esperamos hasta que RD0 esté en HIGH 
        if(TMR1 > 100){                            // Si el tiempo en que RD0 está en LOW es mayor a 100us, entonces hay un error de tiempo fuera
            timeout = 1;
            break;
        }
        TMR1 = 0;                                  // Configuramos el valor del TMR1 a 0 
        while(PORTDbits.RD0);                      // Esperamos hasta que RD0 esté en LOW
        if(TMR1 > 100){                            // Si el tiempo en que RD0 está en HIGH es mayor a 100us, entonces hay un error de tiempo fuera
            timeout = 1;
            break;
        }
        if(TMR1 > 50){
             //bit_set(_data, (7 - i));  
            _data|= (1 << (7 - i));
        }                           
  }
  return _data;
 }

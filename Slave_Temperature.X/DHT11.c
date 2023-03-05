/*
 * File:   DHT11.c
 * Oscar Fernando Donis Martínez (#21611) y Carlos Molina (#21253)
 * Created on 3 de marzo de 2023, 07:07 PM
 */

int timeout;
#include "DHT11.h"
#include <stdint.h>

void DHT11_Start(void){
    TRISDbits.TRISD0 = 0;
    PORTDbits.RD0 = 0;
    __delay_ms(25);
    PORTDbits.RD0 = 1; 
    __delay_us(25);
    TRISDbits.TRISD0 = 1;
}

int DHT11_Response(void){
    timeout=0;
    TMR1 = 0; 
    T1CONbits.TMR1ON = 1;
    while(!PORTDbits.RD0 && TMR1 < 100);           // Wait until DHT11_PIN becomes high (cheking of 80µs low time response)
        if(TMR1 > 99){
            return 0;
        }                                          // Return 0 (Device has a problem with response)
        else{
            TMR1 = 0;                              // Set Timer1 value to 0
            while(PORTDbits.RD0 && TMR1 < 100);    // Wait until DHT11_PIN becomes low (cheking of 80µs high time response)
            if(TMR1 > 99){
                 return 0;
            }                                      // If response time > 99µS  ==> Response error
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
        TMR1 = 0;                                        // Set Timer1 value to 0
        while(!PORTDbits.RD0);                          // Wait until DHT11_PIN becomes high
        if(TMR1 > 100){                         // If low time > 100  ==>  Time out error (Normally it takes 50µs)
            timeout = 1;
            break;
        }
        TMR1 = 0;                                    // Set Timer1 value to 0
        while(PORTDbits.RD0);                           // Wait until DHT11_PIN becomes low
        if(TMR1 > 100){                         // If high time > 100  ==>  Time out error (Normally it takes 26-28µs for 0 and 70µs for 1)
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

unsigned DHT11_Join_Data(unsigned h, unsigned l)
{
    unsigned pow = 10;
    while(l >= pow)
        pow *= 10;
    return h * pow + l;        
}
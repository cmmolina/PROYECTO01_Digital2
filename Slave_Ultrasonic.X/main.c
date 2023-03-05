/*
 * Universidad del Valle de Guatemala
 * Electrónica Digital 2
 * Oscar Fernando Donis Martínez (#21611) y Carlos Molina (#21253)
 * Proyecto 01 - PIC Esclavo
 * Created on 25 de febrero de 2023, 01:43 AM
 */

//*****************************************************************************
// Palabra de configuración
//*****************************************************************************

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <proc/pic16f887.h>
#include "IIC.h"

#define _XTAL_FREQ 1000000
//*****************************************************************************
// Definición de variables
//*****************************************************************************

float distance_cm = 0; 
float duration; 
uint8_t z; 

unsigned int proximity;
//******************************************************************************
// Prototipos de Funciones
//******************************************************************************

void setup(void);
void readUpdate(void);

//******************************************************************************
// Interrupción
//******************************************************************************
void __interrupt() isr (void){    
    
    //Interrupción que indica que una Transmisión/Recepción I2C ha tomado lugar
    if (PIR1bits.SSPIF){
        
        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            z = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            z = SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupción recepción/transmisión SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepción se complete
            PORTD = SSPBUF;             // Guardar en el PORTD el valor del buffer de recepción
            __delay_us(250);
            
        }
        
        else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            z = SSPBUF;
            SSPSTATbits.BF = 0;
            readUpdate();
            SSPBUF = proximity;
            SSPCONbits.CKP = 1;
            __delay_us(250);
            while(SSPSTATbits.BF);
        }  
    
        PIR1bits.SSPIF = 0;
        
    }
}

//******************************************************************************
// Código Principal 
//******************************************************************************
void main(void) {
    setup();
    while(1){
       ;
    }
    return;
}

//******************************************************************************
//Funciones
//******************************************************************************

void setup(void){
    
    //Configuración de I/O 
    ANSEL = 0b00000000;             // 
    ANSELH = 0; 

            //76543210
    TRISA = 0b00000000;             //
    TRISB = 0b00000000;             // 
    //TRISC = 0b00000000;             //
    TRISD = 0b00000010;             // RD1 como input
    TRISE = 0b00000000;             // 
    
    PORTA = 0b00000000; 
    PORTB = 0b00000000; 
    //PORTC = 0b00000000;             //
    PORTD = 0b00000000; 
    PORTE = 0b00000000;
    
    //Configuración de las Interrupciones
    INTCONbits.GIE = 1;             // Se habilitan las interrupciones globales
    INTCONbits.PEIE = 1;            // Se habilitan interrupciones de perifericos
    PIE1bits.SSPIE = 0;             // Se habilita la interrupción del SPI
    PIE1bits.ADIE = 0;              // Se ihabilita la interrupción del ADC
    //PIE1bits.CCP1IE = 1;            // Se habilitan las interrupciones de captura
    //PIE1bits.TMR1IE = 1;
    INTCONbits.TMR0IE = 0;          // Se inhabilitan las interrupciones del TMR0    

    PIR1bits.SSPIF = 0;             // Flag de SPI en 0
    PIR1bits.ADIF = 0;              // Flag de ADC en 0
    INTCONbits.RBIF = 0;            // Flag de Interrupciones del Puerto B en 0
    INTCONbits.T0IF = 0;            // Flag de TMR0 en 0
    //PIR1bits.TMR1IF = 0;            // A TMR1 register capture ocurred
         
    //Configuración del Oscilador
    OSCCONbits.IRCF = 0b100;        // 1MHz
    OSCCONbits.SCS = 1;             // Oscilador Interno
    
    //Configuración del TMR1
    T1CONbits.TMR1CS = 0;           // Oscilador Interno
    T1CONbits.T1CKPS = 0b00;        // Clear Prescaler Bits
    T1CONbits.TMR1ON = 0;           // Inicializamos el TMR1
    
    //Configuración del Módulo de Captura
    //CCP1CONbits.CCP1M = 0b0101;     // Capturar flancos positivos
    
    I2C_Slave_Init(0x50);           // Configuramos la dirección del esclavo
}

void readUpdate(void){
    PORTDbits.RD0 = 1; 
    __delay_us(10);
    PORTDbits.RD0 = 0;
    
    TMR1 = 0;
        
    while (!PORTDbits.RD1){
        ;
    }
    
    T1CONbits.TMR1ON = 1;           // Inicializamos el TMR1
    
    while (PORTDbits.RD1){
        ;
    }
    
    T1CONbits.TMR1ON = 0;
    
    duration = TMR1;
    distance_cm = duration/58.8;
    
    if (distance_cm < 5){
        proximity = 1;
    }
    else{
        proximity = 0;
    }
    
    __delay_ms(100);
}
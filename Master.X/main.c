/*
 * Universidad del Valle de Guatemala
 * Electrónica Digital 2
 * Oscar Fernando Donis Martínez (#21611) y Carlos Molina (#21253)
 * Proyecto 01 - PIC Maestro
 * Created on 24 de febrero de 2023, 12:28 AM
 */

//******************************************************************************
// Palabra de Configuración
//******************************************************************************

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
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
#include "LCD.h"
#include "IIC.h"
//#include "RTC.h"

#define _XTAL_FREQ 4000000
#define tmr0_value 179

//Bits de Control
#define RS PORTDbits.RD2
#define EN PORTDbits.RD3
#define D4 PORTDbits.RD4
#define D5 PORTDbits.RD5
#define D6 PORTDbits.RD6
#define D7 PORTDbits.RD7

//******************************************************************************
// Variables 
//******************************************************************************
unsigned int proximidad = 1;
unsigned int espacios;

float VOLTAJE1; 
float voltaje1;

unsigned int cont = 0; 
unsigned int horas = 0;
unsigned int temporal = 0;


char tens;
char ones;

char ADC1[4];
char HORA[] = "000000";
char FECHA[] = "230923";

uint8_t sec, min, hora; 
uint8_t day, month, year;

//******************************************************************************
// Prototipos de Funciones
//******************************************************************************
void setup(void);
unsigned int map(uint8_t value, int inputmin, int inputmax, int outmin, int outmax);


//******************************************************************************
// Interrupción
//******************************************************************************
void __interrupt() isr (void){    
    
    //Interrupción que indica que una Transmisión/Recepción SPI ha tomado lugar
    if (PIR1bits.SSPIF){
        PIR1bits.SSPIF = 0;
    }
    
    //Interrupción de Envío
    if (PIR1bits.TXIF){
        PIR1bits.TXIF = 0;
    }
    
    //Interrupción de Recepción
    if (PIR1bits.RCIF){
        PIR1bits.RCIF = 0;
    }
    
    //Interrupción del ADC cuando la lectura termina
    if (PIR1bits.ADIF){
        PIR1bits.ADIF = 0; 
    }
    
    //Interrupción del TMR0
    if (INTCONbits.T0IF){
        TMR0 = tmr0_value;          // Cargamos Xms de nuevo al TMR0
        INTCONbits.T0IF = 0;
    }
    
    //Interrupción del Puerto B 
    if (INTCONbits.RBIF){ 
        ;
    }
}

//******************************************************************************
// Código Principal 
//******************************************************************************
void main(void) {
    setup();
    Lcd_Init();
    Lcd_Clear();
    
    while(1){  
        
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        proximidad = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(200);
        
        
        I2C_Master_Start();
        I2C_Master_Write(0x101);
        espacios = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(200);
        
        
        if (proximidad){
            PORTAbits.RA5 = 0;
            PORTAbits.RA4 = 1;
        }
        
        else if (!proximidad){
            PORTAbits.RA4 = 0;
            PORTAbits.RA5 = 1;
        }
        
        if (espacios == 2){
            PORTAbits.RA3 = 1; 
        }
        
        else if (espacios != 2){
            PORTAbits.RA3 = 0;
        }
         
       __delay_ms(1);
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
    //TRISC = 0b00000000;           // 
    TRISD = 0b00000000;             //
    TRISE = 0b00000000;             // 
    
    PORTA = 0b00000000; 
    PORTB = 0b00000000; 
    //PORTC = 0b00000000; 
    PORTD = 0b00000000; 
    PORTE = 0b00000000;
    
    //Configuración del Puerto B 
           //76543210
    IOCB = 0b00011111;              // Pines de Puerto B con Interrupción
    OPTION_REGbits.nRBPU = 0;       // Pull-Up/Pull-Down
    INTCONbits.RBIE = 1;            // Se habilitan las interrupciones del Puerto B
    
    //Configuración de las Interrupciones
    INTCONbits.GIE = 1;             // Se inhabilitan las interrupciones globales
    INTCONbits.PEIE = 0;            // Se inhabilitan interrupciones de perifericos
    PIE1bits.SSPIE = 0;             // Se inhabilita la interrupción del SPI
    PIE1bits.ADIE = 0;              // Se inhabilita la interrupción del ADC
    INTCONbits.TMR0IE = 0;          // Se inhabilitan las interrupciones del TMR0    

    PIR1bits.SSPIF = 0;             // Flag de SPI en 0
    PIR1bits.ADIF = 0;              // Flag de ADC en 0
    INTCONbits.RBIF = 0;            // Flag de Interrupciones del Puerto B en 0
    INTCONbits.T0IF = 0;            // Flag de TMR0 en 0
    
    //Configuración del Oscilador
    OSCCONbits.IRCF = 0b110;        // 4MHz
    OSCCONbits.SCS = 1;             // Oscilador Interno
    I2C_Master_Init(100000);        // Inicializamso Comuncación I2C
}
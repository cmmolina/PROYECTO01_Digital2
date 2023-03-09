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

#define _XTAL_FREQ 8000000
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
unsigned int proximidad;
unsigned int espacios;
unsigned int espacios_comp;
unsigned int temperatura;
unsigned int cont;
unsigned int flag = 0;

float VOLTAJE1; 
float voltaje1;

unsigned int cont = 0; 
unsigned int i;
unsigned int temporal = 0;

char tens;
char ones;

char TEMP1[4];

//******************************************************************************
// Prototipos de Funciones
//******************************************************************************
void setup(void);
void initUART(void);
unsigned int map(uint8_t value, int inputmin, int inputmax, int outmin, int outmax);


//******************************************************************************
// Interrupción
//******************************************************************************
void __interrupt() isr (void){    
    
    //Interrupción que indica que la data se ha leído.
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
        INTCONbits.RBIF = 0;
    }
}

//******************************************************************************
// Código Principal 
//******************************************************************************
void main(void) {
    __delay_ms(1500);
    
    setup();
    initUART();
    i = 1;
    Lcd_Init();
    Lcd_Clear();

    while(1){  
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        proximidad = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(250);
          
        I2C_Master_Start();
        I2C_Master_Write(0x101);
        espacios = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(250);
        
        
        I2C_Master_Start();
        I2C_Master_Write(0x111);
        temperatura = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(250);
        
        // Desplegamos Titulos
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("Welcome!");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("Temp: ");
        Lcd_Set_Cursor(2,13);
        Lcd_Write_String("P->");
        
        // Desplegamos Indicador de Vehiculo Entrante 
        Lcd_Set_Cursor(1,15);
        Lcd_Write_Char(proximidad+32);
        
        // Desplegamos Cantidad de Espacios de Parque Disponibles 
        Lcd_Set_Cursor(2,16);
        Lcd_Write_Char(espacios+48);
        
        // Desplegamos Valor de Temperatura
        tens = (temperatura/10)%10;
        ones = temperatura%10; 
        
        Lcd_Set_Cursor(2,6);
        Lcd_Write_Char(tens+48);
        Lcd_Write_Char(ones+48);
        Lcd_Write_Char(0xDF);
        Lcd_Write_String("C");
        
        //Apertura de la talanquera 
        if (proximidad == 1 && (espacios>0) && (flag != 1) ){
            PORTBbits.RB7 = 1;
            PORTBbits.RB6 = 0;
            __delay_ms(100);
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 0;
            espacios_comp = espacios; 
            flag = 1;
        }
        
        if (proximidad == 0 && (espacios != espacios_comp)){
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 1;
            __delay_ms(100);
            PORTBbits.RB7 = 0;
            PORTBbits.RB6 = 0;
            flag = 0;
        }
        
        //Envío de Información al ESP32
        
        if (i == 1){
            TXREG = espacios;             // Enviamos la información
            PIR1bits.TXIF = 0;              // Apagamos la bandera 
            i = 2;
        }
        
        else if (i == 2){
            TXREG = temperatura;             // Enviamos la información
            PIR1bits.TXIF = 0;              // Apagamos la bandera 
            i = 1;
        }
        
        
        
        //TXREG = temperatura;
        /*while(!TXIF){
            ;
        }
        TXIF = 0;
        */
        
        __delay_ms(5250);
    //End of Loop
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
    
    TRISCbits.TRISC6 = 0;           // RC6/TX como output
    
    //Configuración del Puerto B 
           //76543210
    /*
    IOCB = 0b00011111;              // Pines de Puerto B con Interrupción
    OPTION_REGbits.nRBPU = 0;       // Pull-Up/Pull-Down
    INTCONbits.RBIE = 1;            // Se habilitan las interrupciones del Puerto B
    */
     
    //Configuración de las Interrupciones
    INTCONbits.GIE = 1;             // Se inhabilitan las interrupciones globales
    INTCONbits.PEIE = 1;            // Se inhabilitan interrupciones de perifericos
    PIE1bits.SSPIE = 0;             // Se inhabilita la interrupción del SPI
    PIE1bits.ADIE = 0;              // Se inhabilita la interrupción del ADC
    INTCONbits.TMR0IE = 0;          // Se inhabilitan las interrupciones del TMR0    

    PIR1bits.SSPIF = 0;             // Flag de SPI en 0
    PIR1bits.ADIF = 0;              // Flag de ADC en 0
    INTCONbits.RBIF = 0;            // Flag de Interrupciones del Puerto B en 0
    INTCONbits.T0IF = 0;            // Flag de TMR0 en 0
    
    //Configuración del Oscilador
    OSCCONbits.IRCF = 0b111;        // 4MHz
    //OSCCONbits.IRCF = 0b011;        // 500kHz
    OSCCONbits.SCS = 1;             // Oscilador Interno
    
    
    
    I2C_Master_Init(100000);        // Inicializamso Comuncación I2C
}

void initUART(void){
    
    SPBRG = 12;                     // Baud rate (8MHz/9600)
    //SPBRG = 35; 
    TXSTAbits.SYNC = 0;             // Asíncrono 
    RCSTAbits.SPEN = 1;             // Se habilita el módulo UART
    TXSTAbits.TXEN = 1;             /* Transmisión habilitada; TXIF se enciende
                                     automaticamente.*/
    
    PIR1bits.TXIF = 0;              // Apagamos la bandera de transmisión
    
    RCSTAbits.CREN = 1;             // Habilitamos la recepción
    //BAUDCTLbits.BRG16 = 1;
}
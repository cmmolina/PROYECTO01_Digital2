/* 
 * File: IIC.h  
 * Se utiliz� y se adaptaron las librer�as de Ligo George 
 * de la p�gina www.electrosome.com
 * Enlace: https//electrosome.com/lcd-pic-mplab-xc8/
 * Revision history: 
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __IIC_H
#define	__IIC_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <pic16f887.h>
#include <stdint.h>

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 2000000
#endif

//*****************************************************************************
// Funci�n para inicializar I2C Maestro
//*****************************************************************************
void I2C_Master_Init(const unsigned long c);
//*****************************************************************************
// Funcin de espera: mientras se est iniciada una comunicacin,
// est habilitado una recepcin, est habilitado una parada
// est habilitado un reinicio de la comunicacin, est iniciada
// una comunicacin o se este transmitiendo, el IC2 PIC se esperar�
// antes de realizar algn trabajo
//*****************************************************************************
void I2C_Master_Wait(void);
//*****************************************************************************
// Funci�n de inicio de la comunicaci�n I2C PIC
//*****************************************************************************
void I2C_Master_Start(void);
//*****************************************************************************
// Funci�n de reinicio de la comunicaci�n I2C PIC
//*****************************************************************************
void I2C_Master_RepeatedStart(void);
//*****************************************************************************
// Funci�n de parada de la comunicaci�n I2C PIC
//*****************************************************************************
void I2C_Master_Stop(void);
//*****************************************************************************
//Funci�n de transmisin de datos del maestro al esclavo
//esta funcin devolver un 0 si el esclavo a recibido
//el dato
//*****************************************************************************
void I2C_Master_Write(unsigned d);
//*****************************************************************************
//Funci�n de recepcin de datos enviados por el esclavo al maestro
//esta funci�n es para leer los datos que estn en el esclavo
//*****************************************************************************
int I2C_Master_Read(unsigned short a);
//*****************************************************************************
// Funcin para inicializar I2C Esclavo
//*****************************************************************************
void I2C_Slave_Init(uint8_t address);
//*****************************************************************************
#endif	/* __I2C_H */


/* 
 * File: DHT11.h  
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DHT11_H
#define	DHT11_H

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 8000000
#endif


#include <xc.h> // include processor files - each processor file is guarded.  

void DHT11_Start(void);
int DHT11_Response(void);
unsigned int DHT11_Read(void);
unsigned DHT11_Join_Data(unsigned h, unsigned l);

#endif	/* DHT11_H */


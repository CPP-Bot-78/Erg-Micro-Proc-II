/*
 * PM_ask1.c
 *
 * Created: 22-Mar-23 10:04:49 PM
 * Author : kzach
 */ 
/*
#include <avr/io.h>


int main(void)
{
	
	
    // Replace with your application code
    while (1) 
    {
    }
}
*/
#include <avr/io.h>
#include <util/delay.h>
/*#include "io.h"
#include "delay.h"*/
#define del 5
int main(void){	//PIN is output	PORTD.DIR |= 0b00000010; //PIN1_bm
	//LED is off	PORTD.OUT |= 0b00000010; //PIN1_bm
	while (1) {
		//on		PORTD.OUTCLR= 0b00000010; //PIN1_bm		_delay_ms(del); //wait for 10ms		//off		PORTD.OUT |= 0b00000010; //PIN1_bm		_delay_ms(del); //wait for 10ms
	}}
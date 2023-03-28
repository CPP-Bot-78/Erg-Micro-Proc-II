#include <avr/io.h>#include <util/delay.h>#include <avr/interrupt.h>//#include <avr/iom4808.h>//#include "iom4808.h"
#define ped 40
#define del 10

int x=0;
int car=0;
int tram=0;
int pezoi=0;

//_delay_ms(del);
int main(void) 
{
			PORTD.DIR |= 0b00000111; //PINs are output			PORTD.OUT |= 0b00000111; //LEDs are off			PORTD.OUTCLR= 0b00000100; //Car LED is on			PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;			//timerstuff:			//Tram:			TCA0.SPLIT.CTRLD = 1; //TCA_SPLIT_SPLITM_bm  0x01  /* Split Mode Enable bit mask. */			TCA0.SPLIT.LCNT = 0; //clear counter			TCA0.SPLIT.LCMP0= ped; //T1			TCA0.SPLIT.CTRLB=TCA_SPLIT_LCMP0EN_bm;			//Pezoi:			TCA0.SPLIT.HCNT = 0; //clear counter			TCA0.SPLIT.HCMP0= del; //T2,T3			TCA0.SPLIT.CTRLB=TCA_SPLIT_HCMP0EN_bm;			//Enable			TCA0.SPLIT.CTRLA = 0x7<<1; //TCA_SPLIT_CLKSEL_DIV1024_gc = (0x07<<1)			TCA0.SPLIT.CTRLA |=1;//Enable			TCA0.SPLIT.INTCTRL = TCA_SPLIT_LCMP0_bm; //TCA_SPLIT_LCMP0_bm  0x10 /* Low Compare 0 Interrupt Enable bit mask. */													 //TCA_SPLIT_LCMP0EN_bm  0x01  /* Low Compare 0 Enable bit mask. */													 //TCA_SPLIT_HCMP0EN_bm  0x10  /* High Compare 0 Enable bit mask. *///timerstuff://					-------- Tram -------- 	//while(1){tram=0}					sei(); //begin accepting interrupt signals			while (tram==0) 
			{
				//					-------- Pezoi --------				while (pezoi==0)				{					PORTD.OUTCLR= 0b00000100; //Car LED is on				}				PORTD.OUT |= 0b00000101; //off
				PORTD.OUTCLR= 0b00000100; //Car LED is on
			}			
			cli();
}
		
ISR(PORTF_PORT_vect)
{
	asm("break");
	cli();
	int y = PORTF.INTFLAGS;	PORTF.INTFLAGS=y;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000001; //Pezoi LED is on
	while(TCA0.SPLIT.HCMP0)
	{		
	}
	pezoi=1;	sei();}
ISR(TCA0_LCMP0_vect) //{//	TCA0.SPLIT.CTRLA = 0; //Disable	//clear flag	asm("break");	cli();	int intflags = TCA0.SPLIT.INTFLAGS;	TCA0.SPLIT.INTFLAGS=intflags;	PORTD.OUT |= 0b00000111; //LEDs are off	PORTD.OUTCLR= 0b00000011; //Tram & Pezoi LEDs are on			while(TCA0.SPLIT.HCMP0)	{			}		tram=1;
	sei();
}

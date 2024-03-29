#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ped 2
#define del 1

int time_3=0;
int tram=0;
int pezoi=0;

int main(void)
{
	//LEDs:
	PORTD.DIR |= 0b00000111; //PINs are output
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000100; //Car LED is on
	
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	sei(); //begin accepting interrupt signals
	
	//Timers:
	TCA0.SPLIT.CTRLD = 1; //TCA_SPLIT_SPLITM_bm  0x01  /* Split Mode Enable bit mask. */
	TCA0.SPLIT.CTRLB = 0;
	//Tram:
	TCA0.SPLIT.LCNT = ped; //T1
	TCA0.SPLIT.LCMP0 = 0;
	//Pezoi:
	//TCA0.SPLIT.HCNT = del; //T2,T3
	//TCA0.SPLIT.HCMP0= 0;
	//Enable
	TCA0.SPLIT.CTRLA = 0x7<<1; //TCA_SPLIT_CLKSEL_DIV1024_gc = (0x07<<1)
	TCA0.SPLIT.CTRLA |=1;//Enable
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_LCMP0_bm; //TCA_SPLIT_LCMP0_bm  0x10 /* Low Compare 0 Interrupt Enable bit mask. */
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm; //0x02    /* High Underflow Interrupt Enable bit mask. */

	while(1)
	{
	//                    -------- Tram --------
		while (tram==0)
		{
		//                -------- Pezoi --------
			while (pezoi==0)
			{
				PORTD.OUTCLR= 0b00000100; //Car LED is on
			}
			if(pezoi==1)
			{	cli();
				TCA0.SPLIT.HCNT = del; //T2,T3
				TCA0.SPLIT.HCMP0= 0;
				pezoi=0;
				sei();
			}	
			if(tram==1)
			{
				cli();
				TCA0.SPLIT.HCNT = del; //T2,T3
				TCA0.SPLIT.HCMP0= 0;
				tram=0;
				sei();
			}
			PORTD.OUT |= 0b00000101; //off
			PORTD.OUTCLR= 0b00000100; //Car LED is on
		}
	}
	cli();
}

ISR(PORTF_PORT_vect)
{
	cli();
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000001; //Pezoi LED is on
	pezoi=1;
	sei();
}

ISR(TCA0_LCMP0_vect) //
{
	cli();
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000011; //Tram & Pezoi LEDs are on
	tram=1;
	sei();
}

ISR(TCA0_HUNF_vect) //
{
	cli();
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000011; //Tram & Pezoi LEDs are on
	time_3=1;
	sei();
}
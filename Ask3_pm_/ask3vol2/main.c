#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

int count=0; //count=1 starts the fan, 2 changes the duty cycle, 3 stops the fan
int period= 20;//blades speed
int bladerise = 0;//logical flag
int baserise = 0;//logical flag

int main(void)
{
	//LEDs:
	PORTD.DIR |= 0b00000111; //LEDs are output. 2:ADC, 1:base, 0:blades
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	//TCA0:
	//WGM Low Byte
	TCA0.SPLIT.CTRLD = 1; //split mode ON
	TCA0.SPLIT.CTRLB|= TCA_SPLIT_LCMP0EN_bm; //WGM Low Byte
	TCA0.SPLIT.CTRLB|= TCA_SPLIT_HCMP0EN_bp; //WGM High Byte
	TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; //1024
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm;// HUNF Interrupt Enable
	TCA0.SPLIT.INTCTRL |= TCA_SPLIT_LUNF_bm;// LUNF Interrupt Enable

	/*
	//BASE Timer 
	TCA0.SPLIT.HPER = 240; //40%
	TCA0.SPLIT.HCMP0= 60;
	TCA0.SPLIT.HCNT = 20;
	//BLADES Timer:
	TCA0.SPLIT.LPER = period*2; //50%
	TCA0.SPLIT.LCMP0 = period;
	TCA0.SPLIT.LCNT = 0;
	*/ 
	//ADC:
	ADC0.CTRLA = ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc; //Debug Mode
	ADC0.DBGCTRL = ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.CTRLA |= ADC_FREERUN_bm;
	ADC0.CTRLE = ADC_WINCM0_bm; //Threshold. Result < Winlt
	ADC0.WINLT = 10; //
	ADC0.INTCTRL = ADC_WCMP_bm; //WCM Interrupt
	ADC0.CTRLA |= ADC_ENABLE_bm; //enable//ADC_WINCM0_bm
	sei();
	
	while(1)										// ### THE LOOP ###
	{
		;
	}
	cli();
}

ISR(PORTF_PORT_vect)
{
	cli();
	count++;
	switch(count)
	{
		case 1://start
			PORTD.OUT |= 0b00000111;//LEDs are off
			ADC0.CTRLA |= ADC_FREERUN_bm;
			ADC0.CTRLA |= ADC_ENABLE_bm;
			ADC0.COMMAND |= ADC_STCONV_bm;
				//BASE Timer
			TCA0.SPLIT.HPER = 100; //40%
			TCA0.SPLIT.HCMP0= 40;
			TCA0.SPLIT.HCNT = 20;
				//BLADES Timer:
			TCA0.SPLIT.LPER = period*2; //50%
			TCA0.SPLIT.LCMP0 = period;
			TCA0.SPLIT.LCNT = 0;
			TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; //1024
			TCA0.SPLIT.CTRLA |=1;//Enable
			baserise=0; //just in case the fan stops by adc
			bladerise=0; //just in case the fan stops by adc
			break;
		case 2:// period double
			period*=2;
			break;
		case 3://stop
			PORTD.OUT |= 0b00000111;//LEDs are off
			ADC0.CTRLA = 0;
			TCA0.SPLIT.CTRLA =0;
			period/=2;
			count=0;
			break;
		default: //just in case
			count=1;
			PORTD.OUT |= 0b00000111;//LEDs are off
			PORTD.OUTCLR= 0b00000011;//Base and blades activated
			break;
	}
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	sei();
}

ISR(TCA0_HUNF_vect) //BASE Timer
{
	cli();
	PORTD.OUT=0b00000111; //LEDs are off
	if (baserise==0)
	{
		PORTD.OUT = 0b00000111;//LEDs are off
		PORTD.OUTCLR= 0b00000010; //base led is on
		baserise==1;
	}
	else if(baserise==1)
	{
		PORTD.OUT = 0b00000111;//LEDs are off
		baserise==0;
	}
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;

	sei();
}

ISR(TCA0_LUNF_vect) //BLADES Timer
{
	cli();
	PORTD.OUT=0b00000111; //LEDs are off
	if (bladerise==0)
	{
		PORTD.OUT = 0b00000111;//LEDs are off
		PORTD.OUTCLR= 0b00000001; //base led is on
		bladerise==1;
	}
	else if(bladerise==1)
	{
		PORTD.OUT = 0b00000111;//LEDs are off
		bladerise==0;
	}
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUT = 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000011; //
	sei();
}

ISR(ADC0_WCOMP_vect)
{
	cli();
	PORTD.OUT = 0b00000111;//Base and blades disabled
	PORTD.OUTCLR= 0b00000100;//object near blades LED is ON
	TCA0.SPLIT.CTRLA =0;
	ADC0.CTRLA = 0;
	if (count>=2)period/=2;
	count=0;//to be ready for the next button press
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	sei();
}


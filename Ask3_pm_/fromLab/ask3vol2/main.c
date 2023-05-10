#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

int count=0; //count=1 starts the fan, 2 changes the duty cycle, 3 stops the fan 
int period= 20;//lades speed

int main(void)
{
	//LEDs:
	PORTD.DIR |= 0b00000111; //LEDs are output. 2:ADC, 1:base, 0:blades
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	//TCA0:
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;//WGM
	TCA0.SPLIT.CTRLD = 1; //split mode ON
	TCA0.SPLIT.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; //1024
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm;// HUNF Interrupt Enable
	TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm;// LUNF Interrupt Enable
	TCA0.SPLIT.CTRLA |=1;//Enable
		//BASE Timer
	TCA0.SPLIT.HPER = 160; //40%
	TCA0.SPLIT.HCMP0= 40;
		//BLADES Timer:
	TCA0.SPLIT.LPER = period*2; //50%
	TCA0.SPLIT.LCMP0 = period;
	//ADC:
	ADC0.CTRLA = ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc; //Debug Mode
	ADC0.DBGCTRL = ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.WINLT = 10; //
	ADC0.INTCTRL = ADC_WCMP_bm; //WCM Interrupt
	ADC0.CTRLA |= ADC_ENABLE_bm; //
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
		case 1:
			PORTD.OUT |= 0b00000111;//LEDs are off
			ADC0.CTRLA |= 0;
			TCA0.SPLIT.CTRLA |=0;
			PORTD.OUTCLR= 0b00000011;//Base and blades activated
			break;
		case 2:
			period*=2;
			break;
		case 3:
			PORTD.OUT |= 0b00000111;//LEDs are off
			count=0;
			break;
		default:
			count=1;
			PORTD.OUT |= 0b00000111;//LEDs are off
			PORTD.OUTCLR= 0b00000011;//Base and blades activated
			break;
	}
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	sei();
}

ISR(TCA0_HUNF_vect) //
{
	cli();
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000011; //Tram & Pezoi LEDs are on

	sei();
}

ISR(TCA0_LUNF_vect) //
{
	cli();
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000011; //Tram & Pezoi LEDs are on

	sei();
}

ISR(ADC0_WCOMP_vect)
{
	cli();
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	sei();
}


//paradeigma
/*
int main(void)
{
 PORTD.DIR |= PIN1_bm; //PIN is output
 //prescaler=1024
 TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
 TCA0.SINGLE.PER = 254; //select the resolution
 TCA0.SINGLE.CMP0 = 127; //select the duty cycle.Here it is 50%
 //select Single_Slope_PWM
 TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
 //enable interrupt Overflow
 TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
 //enable interrupt COMP0
 TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
 TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
 sei();
 while (1){;}
 }

 ISR(TCA0_OVF_vect){
	 cli();
	 int intflags = TCA0.SINGLE.INTFLAGS;
	 TCA0.SINGLE.INTFLAGS = intflags;
	 PORTD.OUT |= PIN1_bm; //PIN is off
	 sei();
 }

 ISR(TCA0_CMP0_vect){
	 cli();
	 //clear the interrupt flag
	 int intflags = TCA0.SINGLE.INTFLAGS;
	 TCA0.SINGLE.INTFLAGS = intflags;
	 PORTD.OUTCLR |= PIN1_bm; //PIN is off
	 sei();
 }
 */

 /*
 Ορισμός Prescaler Χρονιστή ?
 TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc.
 ? Ορισμός περιόδου (ορίζεται η μέγιστη τιμή TOP μέχρι την οποία θα μετρήσει
 ο μετρητής) ? TCA0.SINGLE.PER = value.
 ? Ορισμός κύκλου λειτουργίας του παλμού (duty cycle) ?
 TCA0.SINGLE.CMP0 = value.
 ? Επιλογή δημιουργίας κυματομορφών (Waveform Generation Mode) ?
 TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc.
 ? Ενεργοποίηση χρονιστή ?
 TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm.
 */
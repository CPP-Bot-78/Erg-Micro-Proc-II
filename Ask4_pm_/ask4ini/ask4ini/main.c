

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define pin5on 0b00100000 
#define pin6on 0b01000000 
#define pin56on 0b01100000 

void enabling(); 
void disabling();
void alarm();

int alarm_count = 0; //counts up to 3 tries, then alarm starts
int alarm_mode = 0; //if 0 -> enabling, if 1 -> disabling // na dw pou prepei na allaksei
int timer_mode = 0; //if 0 -> timer, if 1 -> pwm
int alarm_brake = 0; // from adc, if 1 alarm will be ready to break //einai o metrhthts ths adc, kseroume sthn isr tou tca an prokeitai gia ton synargemo
int correct_pass = 0; //counts up to 4, then password is correct

int main()
{   
            //Ports #####
    PORTD.DIR |= 0b00000001;
	PORTD.OUT |= PIN0_bm; //LED is off
    PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
			//TCA #####
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;    //enable interrupt Overflow
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;  //enable interrupt COMP0
	        // ADC #####
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //Enable Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINLT
    sei();
    while (1){;}

}

ISR(PORTF_PORT_vect)
{
	cli();
	if(alarm_mode==0) enabling();
	else if(alarm_mode==1) disabling();
	sei();
}

void enabling()
{
	if(correct_pass == 0 || correct_pass == 2) // correct pass =1,3 ->6 || 0,2 ->5
	{
		if( (pin5on && PORTF.INTFLAGS) || (pin56on && PORTF.INTFLAGS) )
		{
			correct_pass++;
			int y = pin5on;
			PORTF.INTFLAGS=y;
		}
		else
		{
			correct_pass=0;
			int y = PORTF.INTFLAGS;
			PORTF.INTFLAGS=y;
		}
	}
	else if(correct_pass == 1 || correct_pass == 3)
	{
		if(pin6on && PORTF.INTFLAGS)
		{
			correct_pass++;
			int y = pin6on;
			PORTF.INTFLAGS=y;
		}
		else
		{
			correct_pass=0;
			int y = PORTF.INTFLAGS;
			PORTF.INTFLAGS=y;
		}
	}
	if(correct_pass == 4)
	{
		correct_pass =0;
		TCA0.SINGLE.CTRLB = 0;//normal
		TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc
		TCA0.SINGLE.CNT = 0; //clear counter
		TCA0.SINGLE.CMP0 = 3; //count up to 20 to get out
		TCA0.SINGLE.CTRLA |=1;//Enable
		int y = PORTF.INTFLAGS;
		PORTF.INTFLAGS=y;
	}
}

void disabling()
{
	if (alarm_count==3) alarm();
	if(correct_pass == 0 || correct_pass == 2) // correct pass =1,3 ->6 || 0,2 ->5
	{
		if( (pin5on && PORTF.INTFLAGS) || (pin56on && PORTF.INTFLAGS) )
		{
			correct_pass++;
			int y = pin5on;
			PORTF.INTFLAGS=y;
		}
		else
		{
			alarm_count++;
			correct_pass=0;
			int y = PORTF.INTFLAGS;
			PORTF.INTFLAGS=y;
			disabling();
		}
	}
	else if(correct_pass == 1 || correct_pass == 3)
	{
		if(pin6on && PORTF.INTFLAGS)
		{
			correct_pass++;
			int y = pin6on;
			PORTF.INTFLAGS=y;
		}
		else
		{
			alarm_count++;
			correct_pass=0;
			int y = PORTF.INTFLAGS;
			PORTF.INTFLAGS=y;
			disabling();
		}
	}
	if(correct_pass == 4)
	{
		alarm_count=0;
		alarm_mode=0;
		timer_mode = 0; 
		alarm_brake = 0;
		correct_pass =0;
		ADC0.CTRLA = 0; //disable adc
		TCA0.SINGLE.CTRLA = 0; //disable tca
		PORTD.OUT= PIN0_bm; // Led is off
		int y = PORTF.INTFLAGS;
		PORTF.INTFLAGS=y;
	}
}

ISR(TCA0_OVF_vect)
{
    cli();
    int intflags =TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUTCLR = PIN0_bm; //LED is on
    sei();
}

ISR(TCA0_CMP0_vect)
{
    cli();
	if (alarm_brake==1) alarm();//einai o metrhthts ths adc
	if(!timer_mode)//timer
	{
		ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
		ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
		ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	}
	else//pwm cmp
	{
		PORTD.OUT = PIN0_bm; //LED is off
	}
	
    int intflags = TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;
    sei();
}

ISR(ADC0_WCOMP_vect)// na mhn metraei o metrhths synexeia
{
    cli();
	PORTD.OUTCLR= PIN0_bm; //LED 0 is on
	alarm_mode=1;//to get in the right path from portf isr
	alarm_brake=1; //ready to brake in tca cmp isr ,//einai o metrhthts ths adc
    TCA0.SINGLE.CTRLB = 0;//normal
    TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc
    TCA0.SINGLE.CNT = 0; //clear counter
    TCA0.SINGLE.CMP0 = 3; //count up to 30 to start the alarm
    TCA0.SINGLE.CTRLA |=1;//Enable
    int intflags = ADC0.INTFLAGS;
    ADC0.INTFLAGS = intflags;
    sei();
	disabling();
}

void alarm()
{
	alarm_brake=0;//gia na mhn ksanaboume ston timer stin alarm
	alarm_count++;//gia na mhn ksanabei se aythn thn synarthsh
	timer_mode=1; //pwm mode in tca cmp isr
	TCA0.SINGLE.CTRLA = 0;
	TCA0.SINGLE.CNT=0;
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.PER = 254; //resolution
	TCA0.SINGLE.CMP0 = 127; //duty cycle
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;  //select Single_Slope_PWM
	TCA0.SINGLE.CTRLA |=1;//Enable
}

/* //Paradeigma
    PORTD.DIR |= PIN1_bm; //PIN is output
    PORTD.OUTCLR= PIN1_bm; //LED is on
    PORTD.OUT |= PIN1_bm; //LED is off
    //prescaler=1024
    TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
    TCA0.SINGLE.PER = 254; //select the resolution
    TCA0.SINGLE.CMP0 = 127; //select the duty cycle
    //select Single_Slope_PWM
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    //enable interrupt Overflow
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    //enable interrupt COMP0
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
    sei();
    while (1){;}

*/

//me tis synarthseis eksasfalizoume epishs oti an xtyphsei to interrupt tou metrhth(gia thn apenergopoihsh) tha pame kateyueian sthn isr toy

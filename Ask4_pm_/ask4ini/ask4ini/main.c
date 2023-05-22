#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define pin5on 0b00001000 //nomizw
#define pin6on 0b00001100 //nomizw

int main()
{   
            //Ports #####
    PORTD.DIR |= 0b00000111; 
    //PORTD.OUTCLR= PIN1_bm; //LED is on
    //PORTD.OUT |= PIN1_bm; //LED is off
    PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc
            // TCA PWM #####
    TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
    TCA0.SINGLE.PER = 254;      //select the resolution
    TCA0.SINGLE.CMP0 = 127;     //select the duty cycle
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;  //select Single_Slope_PWM
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;    //enable interrupt Overflow
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;  //enable interrupt COMP0
            // TCA Count #####
    TCA0.SINGLE.CNT = 0; //clear counter
    TCA0.SINGLE.CTRLB = 0;
    TCA0.SINGLE.CMP0 = ped;
    TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc уел 224
    TCA0.SINGLE.CTRLA |=1;//Enable
            // ADC #####
    ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
    ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
    ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
    ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //Enable Debug Mode
    ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Window Comparator Mode
    ADC0.WINLT |= 10; //Set threshold
    ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
    ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINLT
    ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion

    sei();
    while (1){;}

}

ISR(TCA0_OVF_vect)
{
    cli{};
    int intflags =TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;

    sei();
}

ISR(TCA0_CMP0_vect)
{
    cli{};
    int intflags = TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;

    sei();
}

ISR(ADC0_WCOMP_vect)
{
    cli{};
    int intflags = ADC0.INTFLAGS;
    ADC0.INTFLAGS = intflags;

    sei();
}

ISR(PORTF_PORT_vect)
{
    cli{};
    int y = PORTF.INTFLAGS;
    PORTF.INTFLAGS=y;

    sei();
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
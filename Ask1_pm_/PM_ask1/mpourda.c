#include <avr/io.h>
int x=0;
int main() {
//(��� 219, 224, 205) 16-bit counter high and low
	TCA0.SINGLE.CTRLA = 0; //Disable
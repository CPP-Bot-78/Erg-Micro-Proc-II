#include <avr/io.h>
#include <stdlib.h> // για την συνάρτηση exit();
#include <avr/interrupt.h>

#define freerun 3  // ο χρονος για το free running mode
#define singleconv 2 //ο χρονος για το single conversion
#define turnaround 5 //ο χρονος για να στριψει δεξια ή αριστερα
#define turning180degrees 15 //ο χρονος για να κάνει 180 μοιρων στροφη
#define maxAngle 4 // ο μέγιστος αριθμός γωνιών. Το καθορίζουμε εμείς για κάθε δωματιο. Τώρα είναι 4(απλο 4γωνο)

void turn();

int conv=0; //λογικό flag για το conversion
int gwnies=0; // αριθμος των γωνιών
int anapoda=0;//λογικό flag για το αν ειναι αναποδα. Όταν είναι ανάποδα η τιμή του είναι ίση με 1
int turning=0; //λογικό flag που δείχνει ότι η συσκευή στρίβει δεξιά ή αριστερά. Χρησιμοποιείται για τον μετρητή χρόνου  των στροφών
int turned=0; // λογικό flag που δείχνει ότι η συσκευή τελείωσε το στρίψιμο ή την στροφή 180 μοιρών. Χρησιμοποιείται για τον μετρητή χρόνου των στροφών και της στροφής κατα 180 μοίρες.
int turning180=0; // λογικό flag που δείχνει ότι η συσκευή στρίβει 180 μοίρες.

int main(void)
{
	sei();
	//Αρχικοποίηση ports
	PORTD.DIR |= 0b00000111; //last 3 PINs are output
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	//Αρχικοποίηση μετρητή
	TCA0.SINGLE.CNT = 0; //καθάρισμα μετρητή
	TCA0.SINGLE.CTRLB = 0; //Normal Mode
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; //Ενεργοποίηση Interrupt για τον TCA0
	
	//Αρχικοποίηση ADC
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; //Ενεργοποίηση ADC (όχι μετρήσεων)
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The 7th bit //Ενεργοποίηση Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.WINLT |= 10; //Κάτωφλι. Όταν ο adc σε single conversion πάρει τιμή κάτω από αυτή την τιμή σημαίνει έχει μπροστά(και κοντά) τοίχο και πρέπει να στρίψει.
	ADC0.WINHT |= 10; //Όταν ο adc σε free running mode πάρει τιμή πάνω από αυτή την τιμή σημαίνει πως είναι δεν έχει τοίχο δίπλα της και πρέπει να στρίψει.
	ADC0.INTCTRL |= ADC_WCMP_bm; //Ενεργοποίηση Interrupt για το WCM
	
	while(1)
	{
		ADC0.CTRLE = ADC_WINCM1_bm; //Interrupt when RESULT > WINHT, θέλουμε να στριψεί όταν δεν υπάρχει τοίχος κοντά, όταν η απόσταση από τοίχο στα πλάγια είναι μεγαλύτερη του 10
		ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		PORTD.OUTCLR |= 0b00000010; // Πάμε ευθεία
		
		TCA0.SINGLE.CNT = 0;
		TCA0.SINGLE.CMP0 = freerun; //μετράμε για χρόνο ίσο με freerun θέλοντας κάπως να προσομοιώσουμε τα 3 δευτερόλεπτα
		TCA0.SINGLE.CTRLA |=1;//Ξεκινά η μέτρηση
		while(conv==0)
		{
			;//παγιδεύουμε τον μετρητή
		}
		turn();
		ADC0.CTRLA = 0x01; //καθαρίζουμε το free running mode bit κρατώντας το ressel bit 0 και το enable bit 1
		ADC0.CTRLE = ADC_WINCM0_bm; //Interrupt when RESULT < WINLT θέλουμε να στριψεί όταν υπάρχει τοίχος κοντά, όταν η απόσταση από τοίχο μπροστά είναι μικρότερη του 10
		ADC0.COMMAND |= ADC_STCONV_bm;//Start Conversion
		TCA0.SINGLE.CNT = 0;
		TCA0.SINGLE.CMP0 = singleconv; //μετράμε για χρόνο ίσο με singleconv θέλοντας κάπως να προσομοιώσουμε τα 2 δευτερόλεπτα
		TCA0.SINGLE.CTRLA |=1;//Ξεκινά η μέτρηση
		while(conv==1) // εδώ είναι 1. Με λογικό 0 μπαίνει στην πάνω while, με λογικό 1 σε αυτήν.
		{
			;//παγιδεύουμε τον μετρητή
		}
		turn();
		turning=0;
		turning180=0;
		if(gwnies>=maxAngle){exit(EXIT_ SUCCESS);}  // Αν πηγαίνει κανονικά κι έχει φτάσει στην αρχική γωνία το πρόγραμμα τερματίζει με επιτυχία.
		if(gwnies = 0 && anapoda==1){exit(EXIT_ SUCCESS);} // Αν πηγαίνει ανάποδα κι έχει φτάσει στην αρχική γωνία το πρόγραμμα τερματίζει με επιτυχία.
		cli();
	}
}

ISR(ADC0_WCOMP_vect)
{
	cli();//δεν θέλουμε interrupts μέσα στην ISR
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	PORTD.OUT |= 0b00000111; //LEDs are off
	if(anapoda==0)
	{
		gwnies++; //αυξάνουμε τις γωνίες όσο πηγαίνουμε κανονικά. Όταν θα φτάσει το max θα ολοκληρωθεί το πρόγραμμα.
		if(ADC0.RESL>10) //  Το περιεχόμενο του RESL είναι μεγαλύτερο του 10 άρα ήρθε interrupt από το free running mode. Επομένως στρίβουμε δεξιά
		PORTD.OUTCLR |= 0b00000001; //δεξιά
		else if(ADC0.RESL<10)  //Το περιεχόμενο του RESL είναι μικρότερο του 10 άρα ήρθε interrupt από το single conversion mode. Επομένως στρίβουμε αριστερά
		PORTD.OUTCLR |= 0b00000100; //αριστερά
	}
	else if(anapoda==1)
	{
		gwnies--;//μειώνουμε τις γωνίες όσο πηγαίνουμε ανάποδα. Όταν θα φτάσει το 0 θα ολοκληρωθεί το πρόγραμμα
		if(ADC0.RESL>10) // τα ίδια με πάνω απλά είναι ανάποδα επειδή πάμε προς την αντίθετη κατεύθυνση
		PORTD.OUTCLR |=  0b00000100;//αριστερά
		else if(ADC0.RESL<10)
		PORTD.OUTCLR |= 0b00000001; //δεξιά
	}
	turning=1;// ενεργοποίηση του flag ώστε να μπούμε στον μετρητή του χρόνου που θα πάρει για να στρίψει.
	sei();//δεχόμαστε ξανά interrupts
}

ISR(PORTF_PORT_vect) //ανάποδη λειτουργία
{
	cli();//δεν θέλουμε interrupts μέσα στην ISR
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	PORTD.OUT |= 0b00000111; //LEDs are off
	PORTD.OUTCLR= 0b00000111; //all Leds are ON, η συσκευή γύρισε κατά 180 μοίρες. Τα led σβήνουν μετά το τέλος του μετρητή στην main
	if (anapoda ==0) {anapoda=1;} // εναλλαγή του flag ανάλογα με την τιμή που ήδη είχε
	else if(anapoda ==1) {anapoda=0;}
	turning180=1;// ενεργοποίηση του flag ώστε να μπούμε στον μετρητή του χρόνου που θα πάρει για να στρίψει κατά 180 μοίρες.
	sei();//δεχόμαστε ξανά interrupts
}

ISR(TCA0_CMP0_vect)
{
	cli();//δεν θέλουμε interrupts μέσα στην ISR
	TCA0.SINGLE.CTRLA = 0; //Disable
	//clear flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;
	if (conv ==0) {conv=1;} // εναλλαγή του flag ανάλογα με την τιμή που ήδη είχε
	else if(conv ==1) {conv=0;}
	turned=1; // ενεργοποίηση του flag ώστε να βγούμε από τον μετρητή του χρόνου που θα πάρει για να στρίψει πλάγια ή κατά 180 μοίρες.
	sei();//δεχόμαστε ξανά interrupts
}

void turn()
{
			if(turning==1)
		{
			turned=0;//σημαντικό καθώς θα μπορούσε να έχει καθαριστεί από προηγούμενη μέτρηση καθώς παίρνει την τιμή 1 σε κάθε επίσκεψη στην ISR
			TCA0.SINGLE.CNT = 0;
			TCA0.SINGLE.CMP0 = turnaround;// χρόνος για να στριψει
			TCA0.SINGLE.CTRLA |=1;//Ξεκινά η μέτρηση
			while(turned==0)
			{
				;// παγιδεύουμε τον μετρητή. Το flag turned αλλάζει από την ISR του TCA
			}
			PORTD.OUT |= 0b00000111; //LEDs are off
			PORTD.OUTCLR |= 0b00000010; //πάμε ευθεία
		}
		if(turning180==1)
		{
			turned=0;//σημαντικό καθώς θα μπορούσε να έχει καθαριστεί από προηγούμενη μέτρηση καθώς παίρνει την τιμή 1 σε κάθε επίσκεψη στην ISR
			TCA0.SINGLE.CNT = 0;
			TCA0.SINGLE.CMP0 = turning180degrees;//χρόνος για να στριψει 180 μοιρες
			TCA0.SINGLE.CTRLA |=1;//Ξεκινά η μέτρηση
			while(turned==0)
			{
				;// παγιδεύουμε τον μετρητή. Το flag turned αλλάζει από την ISR του TCA
			}
			PORTD.OUT |= 0b00000111; //LED are off
			PORTD.OUTCLR |= 0b00000010; //πάμε ευθεία
		}
}


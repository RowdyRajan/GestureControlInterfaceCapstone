/*
This prgram will simulate finger touching when falshed on to
the avr with the combination of buttons and LEDs
*/
#include <avr/io.h>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

//Define functions
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay

int main (void)
{
    ioinit(); //Setup IO pins and defaults

    while(1)
    {
		unsigned char i = PIND;
		unsigned char j = PIND;
		unsigned char k = PIND;
		if(CHECK_BIT(i, 2) == 0){
			PORTC = 0x01;
			//PORTB = 0xFF;
			//PORTD = 0xFF;
			//delay_ms(500);
		}
		if(CHECK_BIT(j,3) == 0){
			PORTC = 0x02;
			//PORTB = 0xFF;
			//PORTD = 0xFF;
			//delay_ms(500);
		}
		if (CHECK_BIT(k,4) == 0){
			PORTC = 0x04;
			//PORTB = 0xFF;
			//PORTD = 0xFF;
			//delay_ms(500);
		}
		else{
			PORTC = 0x00;
			//PORTB = 0x00;
		}
		
		//PORTC = 0x02;
		//PORTB = 0xFF;
		//PORTD = 0xFF;
		//delay_ms(500);
		
		//PORTC = 0x04;
		//PORTB = 0xFF;
		//PORTD = 0xFF;
		//delay_ms(500);
		
		//PORTC = 0x00;
		//PORTB = 0x00;
		//PORTD = 0x00;
		//delay_ms(500);
    }
   
    return(0);
}

void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11111111; //All outputs
    DDRC = 0b11111111; //All outputs
    DDRD = 0b00000000; //PORTD (RX on PD0)
	PORTD = 0b00000000;
}

//General short delays
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}

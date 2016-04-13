/*
This prgram will simulate finger touching when falshed on to
the avr with the combination of buttons and LEDs
*/

/* Definitions */
#define F_CPU 1000000							//AVR Clock Speed, in Hz
#define BAUD 9600								//Baud Rate, equal to XBees
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))	//Checks the pin input at specific position

/* Library Inclusions */
#include <avr/io.h>		
#include <stdlib.h>
#include <stdio.h>								//Used for I/O
#include <util/setbaud.h>						//Used to set Baud Rate
#include "mpu6050registers.h"
#include "mpu6050.h"
#include "i2cmaster.h"

//Define functions
//======================
void ioinit(void);      		//Initializes IO
void delay_ms(uint16_t x); 		//General purpose delay
static void uart_9600(void); 	//Initialize baud rate and USART
void uart_putchar(char c, FILE *stream); 		//Used to test xBee comm of UART
void uart_putdouble(double d);
void uart_putint16(uint8_t i);
//======================

int main (void)
{
	/* Initialize the AVR*/
	uart_9600();
	ioinit();
	
	double daxg = 0;
	double dayg = 0;
	double dazg = 0;
	double dgxds = 0;
	double dgyds = 0;
	double dgzds = 0;
	int16_t ax = 0;
	int16_t ay = 0;
	int16_t az = 0;
	int16_t initax = 0;
	int16_t initay = 0;
	int16_t initaz = 0;
	int16_t gx = 0;
	int16_t gy = 0;
	int16_t gz = 0;
	int count = 0;
	int iterator = 0;
	double accz[100];
	//double accx[100];
	double accy[100];

	FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &uart_output;

    while(1)
    {
		/* Pull in the current status of inputs on D port (Buttons) */
		unsigned char i = PIND;
		unsigned char j = PIND;
		unsigned char k = PIND;
		unsigned char l = PINB;
		
		/* If fore finger is high, output it*/
		if(CHECK_BIT(i, 5) != 0){
			delay_ms(10);
			PORTC = 0x08;
			
			
			mpu6050_getConvData(&daxg, &dayg, &dazg, &dgxds, &dgyds, &dgzds);
			mpu6050_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
			
			initax = (int16_t) (daxg*1000);
			initay = (int16_t) (dayg*1000);
			initaz = (int16_t) (dazg*1000);
	
			count = 0;
			
			while(CHECK_BIT (i, 5) != 0){

				
				if(count < 100){
					mpu6050_getNextData(&daxg, &dayg, &dazg, &dgxds, &dgyds, &dgzds, ax, ay, az, gx, gy, gz);
					accz[count] = dazg;
					accy[count] = dayg;
					count++;
				}
				i = PIND;
				
			}
			
			printf("C%d\n",count);
			delay_ms(5);
			printf("%d\n", initax);
			delay_ms(5);
			printf("%d\n", initay);
			delay_ms(5);
			printf("%d\n", initaz);
			delay_ms(5);
			
			for(iterator = 0; iterator < count ;iterator++){
				printf("%d\n", (int16_t) (accy[iterator]*1000));
				delay_ms(5);
				printf("%d\n", (int16_t) (accz[iterator]*1000));
			}
			
			printf("E");
			delay_ms(100);

		}
		/* If middle finger is high, output it*/
		else if (CHECK_BIT(j,6) != 0){
			delay_ms(10);
			PORTC = 0x04;
			uart_putint16(0x32);
		}	
		/* If ring finger is high, output it*/
		else if (CHECK_BIT(k,7) != 0){
			delay_ms(10);
			PORTC = 0x02;
		}
		/* If pinky finger is high, output it*/
		else if(CHECK_BIT(l,0) != 0){
			delay_ms(10);
			PORTC = 0x01;
			printf("test\n");
		}
		/* If no fingers are pressed, set all outputs to 0*/
		else{
			PORTC = 0x00;
		}
		
    }
   
    return(0);
}


/* Sets the baud values within the AVR equal to the baud rate of XBEES (9600 baud)*/
static void uart_9600(void)
	{
	/* Set the baud values */
	UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
	
	/**/
	#if USE_2X
    UCSR0A |= _BV(U2X0);
	#else
    UCSR0A &= ~(_BV(U2X0));
	#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
   }
   
void uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void uart_putint16(uint8_t i) {
    UDR0 = i;
    loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
}
void uart_putdouble(double d){
	UDR0 = d;
    loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
}

void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11111110; //Pinky Finger on PB0
    DDRC = 0b11111111; //All outputs
    DDRD = 0b00000010; //PORTD (RX on PD0, TX on PD1, Fore on PD5, Middle on PD6, Ring on PD7)
	PORTC = 0b00000000;
	mpu6050_init();
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


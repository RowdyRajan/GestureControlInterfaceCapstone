/*
 * This is the main file for the integrated Gesture Control Interface project
 *
 * Written by: Eric Smith, Rajan Jassal, Chris Chmilar
 * Date: 2016-APR-05
 *
 * Licence?
 *
 * Program Flow Overview:
 *
 * The ISP1362 USB controller chip is set up before the main loop by first
 * reseting it, then configuring it so it is ready to connect.
 * The hardware interrupt handler for the USB component is also registered here.
 *
 * I can finish this after demo for the report but I think itll also help me in cleaning up the code, which in turn will also help with integratoin and timing fixes.
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //For usleep
//#include <includes.h>
#include "altera_up_avalon_rs232.h"
#include "altera_up_avalon_rs232_regs.h"

#include "system.h"        //Included to access names of base addresses for various system components
#include "basic_io.h"      //I don't think this is needed. It includes unctions for GPIO pins, seven_seg and larger sleep functions. Will probably be useful if I want to use seven segment display to show partial data.
#include "sys/alt_irq.h"   //To register Alt-Irq, but if I use Hal_aquire I don't need it here.

//USB lib files
#include "BASICTYP.h" //Defines types: UCHAR, BOOL etc. and structs common to most files.
#include "ISR.h"      //Interrupt handlers and associated functions
#include "USB.h"      //Defines some constants and structs. PROBABLE CHANGE HERE
#include "MAINLOOP.h" //USB packet handlers
#include "HAL4D13.h"  //ISP1362 access and control library
#include "CHAP_9.h"   //Functions to handle USB standard requests as specified in USB specification chapter 9.
#include "usb_irq.h"  //For enabling and disabling IRQs

#define SW_READ 1
#define SW_WRITE 2
#define WRITE_FIFO_EMPTY 0x80
#define READ_FIFO_EMPTY 0x0

#define GESTURE_NULL	0x00
#define GESTURE_UP 		0x01
#define GESTURE_DOWN 	0x02
#define GESTURE_LEFT 	0x03
#define GESTURE_RIGHT 	0x04

#define XY_DATA_PRINT 0
#define RX_REMOVE	0

/* Local function forward declarations */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new);
UCHAR readGesture();
void mapGestureToKeycodes(HID_KEYS_REPORT * new, UCHAR gesture_code);
//void resetInitialUSB();

int orientation(int x, int y, int z);
int largestPalmUp(int x,int y,int z);
int largestPalmRight(int x,int y,int z);
int largestPalmLeft(int x,int y,int z);
int largest(int x,int y,int z);
int noMovement(int x, int y, int z);

//-------------------------------------------------------------------------
//  Global Variable
D13FLAGS bD13flags;
USBCHECK_DEVICE_STATES bUSBCheck_Device_State;
CONTROL_XFER ControlData;
IO_REQUEST idata ioRequest;
HID_SETTINGS hid_settings;
HID_KEYS_REPORT new_report;
//-------------------------------------------------------------------------

//Time tracking functions
alt_u32 clk_ticks; //Ticks elapsed since reset (alt_nticks)
alt_u32 clk_rate; //Ticks per second (call alt_ticks_per_second)

int main() {
	/* Local variables */
	/* USB variables */
	HID_KEYS_REPORT old_report;

	BOOL bufferAvailable, readyToSend, watchdog_required;
	UCHAR gesture;
	UCHAR send_buffer[8];
	int i;

	/* Serial - gesture recognition variables */
	alt_u32 write_FIFO_space;
	alt_u16 read_FIFO_used;
	alt_u8 data_W8, data_R8;

	int enter = 0;
	unsigned p_error;
	alt_up_rs232_dev* rs232_dev;

	// open the RS232 UART port
	rs232_dev = alt_up_rs232_open_dev("/dev/rs232_0");
	if (rs232_dev == NULL)
		alt_printf("Error: could not open RS232 UART\n");
	else
		alt_printf("Opened RS232 UART device\n");
	alt_up_rs232_enable_read_interrupt(rs232_dev);
	//alt_up_rs232_disable_read_interrupt(rs232_dev);

	alt_u8 test;
	alt_u8 error = 0;
	int streamStart = 0;
	int offsetStart = 0;
	char buffer[8];
	int measurements[6];
	int inc_i = 0;
	int measurementNumber = 0;
	int clearCounter;
	int axAverage = 0;
	int ayAverage = 0;
	int azAverage = 0;
	int numberOfMeasurements = 0;

	int aoffsetx;
	int aoffsety;
	int aoffsetz;
	int goffsetx;
	int goffsety;
	int goffsetz;

	double pOldx = 0;
	double pNewx = 0;
	double pOldy = 0;
	double pNewy = 0;
	double pOldz = 0;
	double pNewz = 0;

	double aOldx = 0;
	double aNewx = 0;
	double aOldy = 0;
	double aNewy = 0;
	double aOldz = 0;
	double aNewz = 0;

	double vOldx = 0;
	double vNewx = 0;
	double vOldy = 0;
	double vNewy = 0;
	double vOldz = 0;
	double vNewz = 0;

	int VAveragex = 0;
	int VAveragey = 0;
	int VAveragez = 0;

	int cycleCount = 0;
	int count;
	int countStart = 0;
	int counti = 0;

	int ax[100];
	int ay[100];
	int az[100];

	int initax = 0;
	int initay = 0;
	int initaz = 0;

	int initStart = 0;
	int initCounter = 0;

	int gloveOrientation = 0;
	int movementDirection = 0;

	/* Configure ISP1362 chip */
	disable_irq();
	disconnect_USB_controller();
	usleep(1000000);
	Hal4D13_ResetDevice(); //This is good because its a legit ISP1362 call
	//changed state settings to reset values -- If possible I would like to remove these.
	hid_settings.state = reset;
	watchdog_required = FALSE;
	bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
	bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
	bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
	bUSBCheck_Device_State.State_bits.RESET_BITS = 0;
	usleep(1000000);
	reconnect_USB_controller();
	CHECK_CHIP_ID(); //Checks chip ID and prints it to console. Is not actually required
	Hal4D13_AcquireD13(USB_0_BASE,(void*)usb_isr); //Registers main ISR. very important.
	enable_irq();

	/* Initialise HID report structures to zero */
	new_report.modifier = 0x00;
	new_report.reserved = 0x00;
	for(i=0; i<6; i++){
		new_report.keycode[i] = 0x00;
	}

	old_report.modifier = 0x00;
	old_report.reserved = 0x00;
	for(i=0; i<6; i++){
		old_report.keycode[i] = 0x00;
	}

	gesture = 0x00;
	bufferAvailable = TRUE;

	while(1) {

		/* Interrupt service routines modify state bits, calls handler based on state */
		if (bUSBCheck_Device_State.State_bits.RESET_BITS == 1) {
			disable_irq();
			break;
		}
		if (bD13flags.bits.suspend) {
			disable_irq();
			bD13flags.bits.suspend= 0;
			enable_irq();
			change_suspend_state();
		} // Suspend Change Handler
		if (bD13flags.bits.DCP_state == USBFSM4DCP_SETUPPROC) {
			disable_irq();
			SetupToken_Handler();
			enable_irq();
		} // Setup Token Handler
		if ((bD13flags.bits.DCP_state == USBFSM4DCP_REQUESTPROC) && !ControlData.Abort) {
			disable_irq();
			bD13flags.bits.DCP_state = 0x00;
			DeviceRequest_Handler();
			enable_irq();
		} // Device Request Handler

		//TODO: Revisit how toSend is decided. Now bufferAvailable
		if(bD13flags.bits.DCP_state == USBFSM4DCP_INTR1DONE) {
			bD13flags.bits.DCP_state = 0x00;
			if(Hal4D13_GetErrorCode(EPINDEX4EP01) & 0x01) {
				//I feel like I need to do something in here. We get interrupts from the other side when packet is received.
				if(old_report.modifier & 0x04) {
					gesture = 0x05;
				} else {
					gesture = 0x00;
				}

				bufferAvailable = TRUE;
			}
		}

		if(hid_settings.state == connected) {

			UCHAR status = Hal4D13_GetEndpointStatusWOInteruptClear(EPINDEX4EP01);
			if(!(status & 0x60)) {
				//bufferAvailable = TRUE;
				//gesture = 0x00;
			}

			read_FIFO_used = alt_up_rs232_get_used_space_in_read_FIFO(rs232_dev);

			/*
			if (read_FIFO_used > READ_FIFO_EMPTY){
				alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
				alt_printf("%c", data_R8);
				//if(data_R8 == '\n'){
					//tcount++;
					//printf("tcount %d\n", tcount);
				//}
			}*/

			//printf("Where you at BAE?\n");
			if (read_FIFO_used > READ_FIFO_EMPTY){
				//printf("Right he-ya!\n");
				alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
				//printf("counti %d\n",counti);
				//printf("streamstart: %d\n", streamStart);
				//printf("countstart: %d\n", countStart);
				//alt_printf("%c", data_R8);
				if(!streamStart && data_R8 == 'C'){
					printf("Start\n");
					printf("---------\n");
					streamStart = 1;
					countStart = 1;
					cycleCount = 0;
					counti = 0;
					inc_i = 0;
					//printf("%d\n", i);
					continue;
				}


				if(streamStart && countStart && !initStart && (data_R8 == '\n')){
					streamStart = 1;
					countStart = 0;
					cycleCount = 0;
					count = atoi(buffer);
					counti = 0;
					inc_i=0;

					initStart = 1;
					initCounter = 0;

					printf("Count Start %d----------------------\n",count);
					for(clearCounter = 0; clearCounter <8; clearCounter++){
						buffer[clearCounter] = '\0';
					}
					continue;
				}

				if(streamStart && !countStart && initStart && data_R8 == '\n'){
					switch (initCounter){
						case 0:
							initax = atoi(buffer);
							initCounter++;
							break;
						case 1:
							initay = atoi(buffer);
							initCounter++;
							break;
						case 2:
							initaz = atoi(buffer);
							initCounter = 0;
							initStart = 0;
							counti = 0;
							break;
					}

					inc_i=0;
					for(clearCounter = 0; clearCounter <8; clearCounter++){
						buffer[clearCounter] = '\0';
					}
					continue;

				}


				if(streamStart && !countStart && !initStart && data_R8 == '\n'){
					//printf("counti %d\n", counti);
					switch (cycleCount){

						case 0:
							ay[counti] = atoi(buffer);
							//printf("ay: %d\n",ay[counti]);

							vNewy = vOldy + (ay[counti] );
							VAveragey += vNewy;
							vOldy = vNewy;
							cycleCount++;
							break;
						case 1:
							az[counti] = atoi(buffer);
							//printf("counti %d\n", counti);
							//printf("az: %d\n",az[counti]);

							vNewz = vOldz + (az[counti] );
							VAveragez += vNewz;
							vOldz = vNewz;
							counti++;
							//printf("counti %d\n", counti);
							cycleCount = 0;
							break;
					}

					inc_i = 0;
					for(clearCounter = 0; clearCounter <8; clearCounter++){
						buffer[clearCounter] = '\0';
					}
					//printf("%measurement %d: %d\n", measurementNumber, measurements[measurementNumber]);
					continue;
				}

				if(streamStart && !countStart && data_R8 == 'E'){
								//printf("Counti End %d----------------------\n",counti);
								//printf("Count End %d----------------------\n",count);

								int test;
#if XY_DATA_PRINT
								for(test=0;test<count;test++){
									//printf("test %d\n",test);
									printf("az: %d\n",az[test]);
								}
								printf("---------\n");
								for(test=0;test<count;test++){
									//printf("test %d\n",test);
									printf("ay: %d\n",ay[test]);
								}
								printf("---------\n");
								/*
								for(test=0;test<count;test++){
									//printf("test %d\n",test);
									printf("ax: %d\n",ax[test]);
								}*/
								//printf("---------\n");
#endif

								printf("Initial x: %d\n", initax);
								printf("Initial y: %d\n", initay);
								printf("Initial z: %d\n", initaz);

								//VAveragex = VAveragex/count;
								VAveragey = VAveragey/count;
								VAveragez = VAveragez/count;
								//printf("Vx average: %d\n" ,VAveragex);
								printf("Vy average: %d\n" ,VAveragey);
								printf("Vz average: %d\n" ,VAveragez);
								gloveOrientation = orientation(initax,initay,initaz);

								switch (gloveOrientation){
									case 0:
										printf("Palm down\n");
										movementDirection = largest(0, VAveragey, VAveragez);
										break;

									case 1:
										printf("Palm left\n");
										movementDirection = largestPalmLeft(0, VAveragey, VAveragez);
										break;

									case 2:
										printf("Palm right\n");
										movementDirection = largestPalmRight(0, VAveragey, VAveragez);
										break;
									case 3:
										printf("Palm up\n");
										movementDirection = largestPalmUp(0, VAveragey, VAveragez);
										break;

								}

								if(noMovement(0, VAveragey, VAveragez)){
									movementDirection = 6;
								}
								switch (movementDirection){
									case 0:
										printf("Forward\n");
										break;
									case 1:
										printf("Backward\n");
										break;
									case 2:
										gesture = GESTURE_LEFT;
										printf("Left\n");
										break;
									case 3:
										gesture = GESTURE_RIGHT;
										printf("Right\n");
										break;
									case 4:
										gesture = GESTURE_UP;
										printf("Up\n");
										break;
									case 5:
										gesture = GESTURE_DOWN;
										printf("Down\n");
										break;
									case 6:
										gesture = GESTURE_NULL;
										printf("No movement\n");
										break;
								}

								if(counti == count){
									printf("Correct number of Measurements\n");
								}else{
									printf("Did not get correct count\n");
								}
								printf("End\n");

								streamStart = 0;
								cycleCount = 0;
								counti = 0;
								inc_i = 0;
								VAveragex = 0;
								VAveragey = 0;
								VAveragez = 0;

								count = 0;


								vOldx = 0;
								vOldy = 0;
								vOldz = 0;
								printf("END----------------------------\n");
								continue;
							}

				if(streamStart){

					buffer[inc_i] = data_R8;
					//printf("%d\n", counti);
					//alt_printf("%c", buffer[i]);
					//alt_printf("i:%d, buffer i: %c\n", i, buffer[i]);
					inc_i++;
					continue;
				}
			}

		disable_irq();
		mapGestureToKeycodes(&new_report, gesture);
		enable_irq();

		/* See if we should send the next report */
		disable_irq();
		readyToSend = reportChanged(old_report, new_report) && bufferAvailable;
		if(readyToSend || watchdog_required) {
			send_buffer[0] =  new_report.modifier;
			send_buffer[1] =  new_report.reserved;
			send_buffer[2] =  new_report.keycode[0];
			send_buffer[3] =  new_report.keycode[1];
			send_buffer[4] =  new_report.keycode[2];
			send_buffer[5] =  new_report.keycode[3];
			send_buffer[6] =  new_report.keycode[4];
			send_buffer[7] =  new_report.keycode[5];

			Hal4D13_LockDevice(0);
			Hal4D13_WriteEndpoint(EPINDEX4EP01, send_buffer, 8);
			bufferAvailable = FALSE;
			readyToSend = FALSE;
			watchdog_required = FALSE;
		}
		enable_irq();

		/* Update old report */
		old_report.keycode[0] = new_report.keycode[0]; //We only use one key slot so only need to to update one
		old_report.modifier = new_report.modifier;

		if(new_report.keycode[0] != 0)
			IOWR(GREEN_LEDS_BASE, 0, new_report.keycode[0]);
		} //If Connected

		if(hid_settings.state == reset){
			gesture = 0x00;
			bufferAvailable = TRUE;
		}

  	} //While

  return 0;
}//Main

/* Check to see if data in report has changed */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new) {
	BOOL rv = FALSE;
	int i;

	if(old.modifier != new.modifier) {
		rv = TRUE;
	}
	for(i=0; i<6; i++){
		if(old.keycode[i] != new.keycode[i]){
			rv = TRUE;
		}
	}
	return rv;
}

UCHAR readGesture() {
	UCHAR gest_code = 0x00;
	//Place gesture recognition code here. TODO.
	return gest_code;
}

void mapGestureToKeycodes(HID_KEYS_REPORT * new, UCHAR gesture_code) {
	switch(gesture_code) {
		case(GESTURE_NULL) :
			new->modifier = 0x00;
			new->keycode[0] = 0x00;	//Pg-Up
			break;
		case(GESTURE_UP) :
			new->modifier = 0x00;
			new->keycode[0] = 0x61;	//Pg-Up
			break;
		case(GESTURE_DOWN) :
			new->modifier = 0x00;
			new->keycode[0] = 0x5B;	//Pg-Dn
			break;
		case(GESTURE_LEFT) :
			new->modifier = 0x06;	//Left Alt
			new->keycode[0] = 0x2B;	//Tab
			break;
		case(GESTURE_RIGHT) :
			new->modifier = 0x04;	//Left Alt
			new->keycode[0] = 0x2B;	//Tab
			break;
		default :
			new->modifier = 0x04;
			new->keycode[0] = 0x00;
			break;
	}
}

int noMovement(int x, int y, int z){
	if((abs(x) < 1500) && (abs(y) < 1500) && (abs(z) < 1500) ){
		return 1;
	}
	return 0;
}

int largest(int x,int y,int z){
	if(abs(x) >= abs(y) && abs(x) >= abs(z)){
		if(x>=0){
			return 0;
		} else{
			return 1;
		}
	}else if(abs(y) >= abs(z)){
		if(y>=0){
			return 2;
		} else{
			return 3;
		}
	}else {
		if(z>=0){
			return 4;
		} else{
			return 5;
		}
	}
}
int largestPalmLeft(int x,int y,int z){
	if(abs(x) >= abs(y) && abs(x) >= abs(z)){
		if(x>=0){
			return 0;
		} else{
			return 1;
		}
	}else if(abs(y) >= abs(z)){
		if(y>=0){
			return 4;
		} else{
			return 5;
		}
	}else {
		if(z>=0){
			return 3;
		} else{
			return 2;
		}
	}
}

int largestPalmRight(int x,int y,int z){
	if(abs(x) >= abs(y) && abs(x) >= abs(z)){
		if(x>=0){
			return 0;
		} else{
			return 1;
		}
	}else if(abs(y) >= abs(z)){
		if(y>=0){
			return 5;
		} else{
			return 4;
		}
	}else {
		if(z>=0){
			return 2;
		} else{
			return 3;
		}
	}
}

int largestPalmUp(int x,int y,int z){
	if(abs(x) >= abs(y) && abs(x) >= abs(z)){
		if(x>=0){
			return 0;
		} else{
			return 1;
		}
	}else if(abs(y) >= abs(z)){
		if(y>=0){
			return 3;
		} else{
			return 2;
		}
	}else {
		if(z>=0){
			return 5;
		} else{
			return 4;
		}
	}
}

//function to get the orientations. 0 means palm towards the floor. 1 is palm towards the left
//2 is palm towards the right. 3 is palm towards the roof.
int orientation(int x, int y, int z){
	if(abs(z) >= abs(y) && abs(z) >= abs(x)){
			if(z>=0){
				return 0;
			} else{
				return 3;
			}
		}else if(abs(y) >= abs(x)){
			if(y>=0){
				return 1;
			} else{
				return 2;
			}
		}else {
			if(x>=0){
				return 0;
			} else{
				return 0;
			}
		}
}

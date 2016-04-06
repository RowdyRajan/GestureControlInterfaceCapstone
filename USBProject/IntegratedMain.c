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

#include "system.h"        //Included to access names of base addresses for various system components
#include "basic_io.h"      //I don't think this is needed. It includes unctions for GPIO pins, seven_seg and larger sleep functions. Will probably be useful if I want to use seven segment display to show partial data. 
#include "sys/alt_irq.h"   //To register Alt-Irq, but if I use Hal_aquire I don't need it here.

 #include <unistd.h>  //For usleep
#include <stdio.h>    //Printf, though this may not be required since a version that accesses the NiosII console is generated in HAL
#include <stdlib.h>   //hmm. Not sure. ill figure it out later.  

//USB lib files
#include "BASICTYP.h" //Defines types: UCHAR, BOOL etc. and structs common to most files.
#include "ISR.h"      //Interrupt handlers and associated functions
#include "USB.h"      //Defines some constants and structs. PROBABLE CHANGE HERE
#include "MAINLOOP.h" //USB packet handlers
#include "HAL4D13.h"  //ISP1362 access and control library
#include "CHAP_9.h"   //Functions to handle USB standard requests as specified in USB specification chapter 9.
#include "usb_irq.h"  //For enabling and disabling IRQs

/* Local function forward declarations */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new);
UCHAR readGesture();
void mapGestureToKeycodes(HID_KEYS_REPORT * new, UCHAR gesture_code);

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
#if 0
int main() {
	/* Local variables */
	HID_KEYS_REPORT old_report;

	BOOL bufferAvailable, readyToSend, watchdog_required;
	UCHAR gesture;
	UCHAR send_buffer[8];
	int i;

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
			}
		}

		UCHAR status = Hal4D13_GetEndpointStatusWOInteruptClear(EPINDEX4EP01);
		if(!(status & 0x60)) {
			bufferAvailable = TRUE;
		}
		printf("Status bits are  = %x\n", status);

		if(hid_settings.state == connected) {
			
			/* Check for gesture */
			disable_irq();
			gesture = readGesture();
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
		}

	} //End While(1)
	return 0;
 }//End Main

/* Check to see if data in report has changed */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new) {
	BOOL rv = FALSE;
	int i;

	if(old.modifier == new.modifier) {
		rv = TRUE;
	}
	for(i=0; i<6; i++){
		if(old.keycode[i] == new.keycode[i]){
			rv = TRUE;
		}
	}
	return rv;
}

#define GESTURE_UP 		0x01
#define GESTURE_DOWN 	0x02
#define GESTURE_LEFT 	0x03
#define GESTURE_RIGHT 	0x04

UCHAR readGesture() {
	UCHAR gest_code = 0x00;
	//Place gesture recognition code here. TODO.
	return gest_code;
}

void mapGestureToKeycodes(HID_KEYS_REPORT * new, UCHAR gesture_code) {
	switch(gesture_code) {
		case(GESTURE_UP) :
			new->modifier = 0x00;
			new->keycode[0] = 0x61;	//Pg-Up
			break;
		case(GESTURE_DOWN) :
			new->modifier = 0x00;
			new->keycode[0] = 0x5B;	//Pg-Dn
			break;
		case(GESTURE_LEFT) :
			new->modifier = 0x04;	//Left Alt
			new->keycode[0] = 0x2B;	//Tab
			break;
		case(GESTURE_RIGHT) :
			new->modifier = 0x04;	//Left Alt
			new->keycode[0] = 0x2B;	//Tab
			break;
		default :
			new->modifier = 0x00;
			new->keycode[0] = 0x00;
			break;
	}
}
#endif

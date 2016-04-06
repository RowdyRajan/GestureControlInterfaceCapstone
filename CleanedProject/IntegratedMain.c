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
#include "BASICTYP.h" //Defines types for basic things: UCHAR, BOOL etc.
#include "COMMON.h"   //Defines typedefs for structs and macros that are used throughout many files.
#include "ISR.h"      //Interrupt handlers and associated functions
#include "USB.h"      //Defines some constants and structs. PROBABLE CHANGE HERE (put into usb_info.h, usb_info.c)
#include "MAINLOOP.h" //USB packet handlers
#include "HAL4D13.h"  //ISP1362 access and control library
#include "CHAP_9.h"   //Functions to handle USB standard requests as specified in USB specification chapter 9.

/* Local function forward declarations */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new);
UCHAR readGesture();
void mapGestureToKeycodes(HID_KEYS_REPORT new, UCHAR gesture_code);


//-------------------------------------------------------------------------
//  Global Variable
D13FLAGS bD13flags;
USBCHECK_DEVICE_STATES bUSBCheck_Device_State;
CONTROL_XFER ControlData;
IO_REQUEST idata ioRequest;
//-------------------------------------------------------------------------

HID_KEYS_REPORT new_report;
HID_KEYS_REPORT old_report;

 int main() {

	/* Configure ISP1362 chip */
	disable(); //disable interrupts
	disconnect_USB(); // I also don't like these functions. It isn't named for what it actually does and there are a lot of winding sub-function calls within
	usleep(1000000);
	
	Hal4D13_ResetDevice(); //This is good because its a legit ISP1362 call
	
	//changed state settings to reset values -- If possible I would like to remove these.
	bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
	bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
	bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
	bUSBCheck_Device_State.State_bits.RESET_BITS = 0;
	
	usleep(1000000);
	reconnect_USB(); //Change this call. Its quite silly much like disconnect. 
	CHECK_CHIP_ID(); //Just checks the chip ID doesn't actually do anything I need, but I suppose I could abort if the wrong chip is detected
	Hal4D13_AcquireD13(USB_0_BASE,(void*)usb_isr); //Registers isr. very important.
	enable(); //reenable interrupts

	/* Initialize HID report structures to zero */
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
		//I want to change this state system. I'm not a huge fan, although I suppose its fine.. I just definitely need to chnage how toSend is changed.
		/* Interrupt handlers modify state bits, while loop branches to handler based on state */
		if (bUSBCheck_Device_State.State_bits.RESET_BITS == 1) {
			disable();
			break;
		}
      if (bD13flags.bits.suspend) {
         disable();
         bD13flags.bits.suspend= 0;
         enable();
         suspend_change();
		} // Suspend Change Handler
		if (bD13flags.bits.DCP_state == USBFSM4DCP_SETUPPROC) {
			disable();
          SetupToken_Handler();
          enable();
      } // Setup Token Handler
      if ((bD13flags.bits.DCP_state == USBFSM4DCP_REQUESTPROC) && !ControlData.Abort) {
			disable();
			bD13flags.bits.DCP_state = 0x00;
			DeviceRequest_Handler();
			enable();
		} // Device Request Handler
		if(bD13flags.bits.DCP_state == USBFSM4DCP_INTR1DONE) {
			bD13flags.bits.DCP_state = 0x00;
			if(Hal4D13_GetErrorCode(EPINDEX4EP01) & 0x01)
				toSend = TRUE;
		}

		if(State == Connected) {
			




			/* Update old report */
			old_report.keycode[0] = new_report.keycode[0]; //We only use one key slot so we only update one
			old_report.modifier = new_report.modifier;

			reset_timeout_watchdog_if_needed(); //TODO
		}




	} //End While(1)
 }//End Main

/* Check to see if data in report has changed */
BOOL reportChanged(HID_KEYS_REPORT old, HID_KEYS_REPORT new) {
	BOOL rv = FALSE;
	if(old.modifier == new.modifier)
	(old.keycode[0] == new.keycode[0]))
		return TRUE;
	else
		return FALSE;
}

UCHAR readGesture() {
	UCHAR gest_code = 0x00;
 
	//Place gesture recognition code here. TODO.

	return gest_code;
}

void mapGestureToKeycodes(HID_KEYS_REPORT new, UCHAR gesture_code) {

}
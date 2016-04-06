/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include "system.h"
#include "basic_io.h"
#include "sys/alt_irq.h"

//  for USB
#include "BASICTYP.h"
#include "COMMON.h"
#include "ISR.h"
#include "USB.h"
#include "MAINLOOP.h"
#include "HAL4D13.h"
#include "CHAP_9.h"

#include "ISP1362_HAL.h"

#include "altera_avalon_pio_regs.h"
#include "alt_types.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------------------------------------------
//  Global Variable
D13FLAGS bD13flags;
USBCHECK_DEVICE_STATES bUSBCheck_Device_State;
CONTROL_XFER ControlData;
IO_REQUEST idata ioRequest;

IDLE_TIMER idletime;
HID_KEYS_REPORT hid_report;
HID_KEYS_REPORT2 hid_report2;
//-------------------------------------------------------------------------

UCHAR gest_change = 0;

#include <stdio.h>

//Loop variable
int i;

int Gesture_Recognition();
void setKeys(int, HID_KEYS_REPORT2 *);
void sendReport(UCHAR, HID_KEYS_REPORT *);

int main()
{
  printf("Hello from Nios II!\n");
  idletime.wait_time = -1;
  disable();
  disconnect_USB();
  usleep(1000000);
  Hal4D13_ResetDevice();
  bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
  bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
  bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
  bUSBCheck_Device_State.State_bits.RESET_BITS = 0;
  usleep(1000000);
  reconnect_USB();
  CHECK_CHIP_ID();
  Hal4D13_AcquireD13(USB_0_BASE,(void*)usb_isr);
  enable();
  bD13flags.bits.verbose=1;
  //PUCHAR switches;
  //Set Timer. Or Ready Timer.

  int gesture;
  UCHAR send_buffer[8];

  HID_KEYS_REPORT2 old_keys;
  old_keys.modifier = 0x00;
  old_keys.reserved = 0x00;
  //old_keys.keycode = 0x00;
  for(i=0; i<6; i++){
	  old_keys.keycode[i] = 0x00;
  }
  HID_KEYS_REPORT2 new_keys;
  new_keys.modifier = 0x00;
  new_keys.reserved = 0x00;
  //new_keys.keycode = 0x00;
  for(i=0; i<6; i++){
	  new_keys.keycode[i] = 0x00;
  }

  hid_report.modifier = 0x00;
  hid_report.reserved = 0x00;
  hid_report.keycode = 0x00;

  hid_report2.modifier = 0x00;
  hid_report2.reserved = 0x00;

  for(i=0; i<6; i++){
	  hid_report2.keycode[i] = 0x00;
  }

  BOOL toSend = TRUE;
  BOOL changed = TRUE;

  //USHORT time;
  while (1)
  {
        if (bUSBCheck_Device_State.State_bits.RESET_BITS == 1)
        {
          disable();
          break;
        }
        if (bD13flags.bits.suspend)
        {
          disable();
          bD13flags.bits.suspend= 0;
          enable();
          suspend_change();
        } // Suspend Change Handler
        if (bD13flags.bits.DCP_state == USBFSM4DCP_SETUPPROC)
        {
          disable();
          SetupToken_Handler();
          enable();
        } // Setup Token Handler
        if ((bD13flags.bits.DCP_state == USBFSM4DCP_REQUESTPROC) && !ControlData.Abort)
        {
          disable();
          bD13flags.bits.DCP_state = 0x00;
          DeviceRequest_Handler();
          enable();
        } // Device Request Handler
        if(bD13flags.bits.DCP_state == USBFSM4DCP_INTR1DONE){
        	bD13flags.bits.DCP_state = 0x00;
        	if(Hal4D13_GetErrorCode(EPINDEX4EP01) & 0x01)
        		toSend = TRUE;

        }

        if(idletime.wait_time > -1) {
        	/* Put code here to control key changes (Gesture Recognition) */

        	gesture = Gesture_Recognition();

        	//if(gesture != NULL) {
        	setKeys(gesture, &new_keys);
        	setKeys(gesture, &hid_report2);
        	//}
        	if((old_keys.keycode != new_keys.keycode) || (old_keys.modifier != new_keys.modifier)) {
        		changed = TRUE;
        	}
        	if(toSend && changed) {
        		send_buffer[0] =  new_keys.modifier;
        		send_buffer[1] =  new_keys.reserved;
        		send_buffer[2] =  new_keys.keycode[0];
        		send_buffer[3] =  new_keys.keycode[1];
        		send_buffer[4] =  new_keys.keycode[2];
        		send_buffer[5] =  new_keys.keycode[3];
        		send_buffer[6] =  new_keys.keycode[4];
        		send_buffer[7] =  new_keys.keycode[5];
        		//printf("Modifier = %x : Keycode = %x\n", send_buffer[0], send_buffer[2]);

        		Hal4D13_LockDevice(0);
        		int len = Hal4D13_WriteEndpoint(EPINDEX4EP01, send_buffer, 8);
        		printf("BufFilledTo = %x\n", len);

        		//send_buffer[0] =  new_keys.modifier;
        		 //       		send_buffer[1] =  new_keys.reserved;
        		  //      		send_buffer[2] =  0x00;

        		//len = Hal4D13_WriteEndpoint(EPINDEX4EP01, send_buffer, 8);
        		toSend = FALSE;
        	}

        	old_keys.keycode[0] = new_keys.keycode[0];
        	old_keys.modifier = new_keys.modifier;
        	if(new_keys.keycode != 0)
        		IOWR(GREEN_LEDS_BASE, 0, new_keys.modifier);
            //switches = (PUCHAR) IORD(SWITCHES_BASE, 0);

        }

        usleep(1);
  }
  return 0;
}

void sendReport(UCHAR endpoint_index, HID_KEYS_REPORT * report) {
	//Hal4D13_WriteEndpoint
}

int Gesture_Recognition() {
	if(gest_change == 1) {
		gest_change = 2;
	} else if (gest_change == 2) {
		gest_change = 3;
	} else if (gest_change == 3) {
		gest_change = 0;
	} else {
		gest_change = 1;
	}
	return gest_change;
}

void setKeys(int gest, HID_KEYS_REPORT2 * keys) {

	switch(gest) {
		case(1):	// Alt-Tab
			keys->modifier = 0x00;		//Left Alt
			keys->keycode[0] = 0x0A;	//Tab key
			break;
		case(2):
			keys->modifier = 0x00;		//Left Alt + Left Shift
			keys->keycode[0] = 0x00;	//Tab
			break;
		case(3):
			keys->modifier = 0x00;		//None
			keys->keycode[0] = 0x2B;	//Pg-Up
			break;
		case(4):
			keys->modifier = 0x00;		//None
			keys->keycode[0] = 0x00;	//Pg-Dwn
			break;
		default:
			keys->modifier = 0x00;
			keys->keycode[0] = 0x00;
			break;
	}
}

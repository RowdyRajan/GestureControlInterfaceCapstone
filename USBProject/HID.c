/*
 * This file contains the functions for handling HID class requests.
 * 
 * They are called from the MAINLOOP file within DeviceRequest_Handler.
 * Most are not required for an HID keyboard; The only manditory class requests
 * are:
 *    - SetIdle
 *    - GetIdle
 *    - GetReport
*/

#include <stdio.h>
#include "BasicTyp.h"
#include "Chap_9.h"


/* Required global variables */
extern CONTROL_XFER ControlData; // Holds data from reveived packets
extern HID_KEYS_REPORT new_report;
extern HID_SETTINGS hid_settings;

void HID_Get_Report() {
   UCHAR   bDescriptor =      MSB(ControlData.DeviceRequest.wValue);
   UCHAR   bDescriptorIndex = LSB(ControlData.DeviceRequest.wValue);
   USHORT  bLen = ControlData.DeviceRequest.wLength;
   // printf("bDescri = %d : ",bDescriptor);
   // printf("Index = %d\n",bDescriptorIndex);

   if(sizeof(new_report) <= bLen){
      Chap9_BurstTransmitEP0((PUCHAR)&new_report, sizeof(new_report));
   }
   hid_settings.state = connected;
}

void HID_Get_Idle() {
   printf("Get Idle\n");
}

void HID_Get_Protocol() {
   printf("Get Protocol\n");
}

void HID_Set_Report() {
   printf("Set Report\n");
}

void HID_Set_Idle() {
   printf("Set Idle\n");

   UCHAR duration = MSB(ControlData.DeviceRequest.wValue);
   UCHAR reportID = LSB(ControlData.DeviceRequest.wValue);

   printf("Duration: %x, ReportID: %x\n", duration, reportID);

   hid_settings.idle_time = 0;
   if(duration != 0) {
	   hid_settings.idle_time = duration * 4; //wait time in milliseconds
   }
   if(reportID != 0) {
      printf("ReportID = %ud\n", reportID);
   }
   hid_settings.state = connected;
}

void HID_Set_Protocol() {
   printf("Set Protocol\n");
}

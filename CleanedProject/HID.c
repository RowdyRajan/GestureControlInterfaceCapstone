/*
 * This file contains the functions for handling HID class requests.
 * 
 * They are called from the MAINLOOP file within DeviceRequest_Handler.
 * Most are not required for an HID keyboard; The only manditory class requests
 * are:
 *    - SetIdle
 *    - GetIdle
 *    - GetReport
 *
*/

#include <stdio.h>
#include "BasicTyp.h"
#include "HAL4d13.h"


/* Required global variables */
extern CONTROL_XFER ControlData; // Holds data from reveived packets

void HID_Get_Report() {
   UCHAR   bDescriptor =      MSB(ControlData.DeviceRequest.wValue);
   UCHAR   bDescriptorIndex = LSB(ControlData.DeviceRequest.wValue);
   USHORT   bLen = ControlData.DeviceRequest.wLength;
   // printf("bDescri = %d : ",bDescriptor);
   // printf("Index = %d\n",bDescriptorIndex);

   if(sizeof(hid_report) <= bLen){
      Chap9_BurstTransmitEP0((PUCHAR)&hid_report, sizeof(hid_report));
   }
   //printf("Get Report\n");
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

   idletime.wait_time = 0;
   if(duration != 0) {
      idletime.wait_time = duration * 4; //wait time in milliseconds
      printf("wait_time set to = %d millis\n", idletime.wait_time);
   }
   if(reportID != 0) {
      printf("ReportID = %ud\n", reportID);
   }
}

void HID_Set_Protocol() {
   printf("Set Protocol\n");
}
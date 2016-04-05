
#ifndef __MAINLOOP_H__
#define __MAINLOOP_H__

#include "BasicTyp.h"

//***************************************************************************
#define DMA_IDLE        0
#define DMA_RUNNING 1
#define DMA_PENDING 2
#define DMA_IN_DONE 3
// **************************************************************************
#define DMA_BUFFER_SIZE     65536

// Endpoint configuration flags
#define ENDPOINT_FIFO_ENABLE            0x80 // Enables an end-point buffer memory (disabled will not be allocated memory)
#define ENDPOINT_DIR_IN		            0x40
#define ENDPOINT_DIR_OUT	            0x00
#define ENDPOINT_DBLBUF_EN         		0x20
#define ENDPOINT_IS_ISO    	        	0x10 // Is either ISO (If set) or Bulk / Interrupt (If not set)
#define ENDPOINT_NOT_ISO				0x00

#define ENDPOINT_ISO_SIZE_MASK        	0x0F
#define ENDPOINT_NONISO_SIZE_MASK       0x07
#define ENDPOINT_RFB_EN            		0x08

   //*************************************************************************
   // Function Prototypes
   //*************************************************************************
void SetupToken_Handler(void);
void DeviceRequest_Handler(void);
void help_devreq(UCHAR type, UCHAR req);
void disconnect_USB(void);
void connect_USB(void);
void reconnect_USB(void);
void config_endpoint(void);
void suspend_change(void);
void ML_AcknowledgeSETUP(void);
void ML_StallEP0(void);
void ML_Reserved(void);

void Get_Report();
void Get_Idle();
void Get_Protocol();
void Set_Report();
void Set_Idle();
void Set_Protocol();

unsigned short CHECK_CHIP_ID(void);


#endif


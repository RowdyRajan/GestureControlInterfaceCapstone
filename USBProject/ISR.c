
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "BasicTyp.h"
#include "USB.h"
#include "HAL4D13.h"
#include "isr.h"
#include "mainloop.h"
#include "usb_irq.h"
#include "chap_9.h"
#include "isp1362_includes.h"
#include "system.h"

// ************************************************************************
//  Public static data
// ************************************************************************

extern D13FLAGS bD13flags;
extern USBCHECK_DEVICE_STATES bUSBCheck_Device_State;
extern CONTROL_XFER ControlData;
extern HID_SETTINGS hid_settings;
extern HID_KEYS_REPORT new_report;

// ************************************************************************
//  ISR Subroutine
// ************************************************************************
void fn_usb_isr(void)
{
    disable_irq();
    Hal4D13_LockDevice(0);
    enable_irq();
}

void usb_isr(void)
{
    ULONG   i_st;
    disable_irq();
    Hal4D13_LockDevice(0); //Unlock ISP1362 buffers to read/write
    bD13flags.bits.At_IRQL1 = 1;
    i_st = Hal4D13_ReadInterruptRegister();
    if(i_st != 0) 
    {
        if(i_st & D13REG_INTSRC_BUSRESET)
            Isr_BusReset();
        else if(i_st & D13REG_INTSRC_SUSPEND)
            Isr_SuspendChange();
        else if(i_st & D13REG_INTSRC_EOT)
            Isr_DmaEot();
        else if(i_st & (D13REG_INTSRC_SOF|D13REG_INTSRC_PSEUDO_SOF))
            Isr_SOF();
        else if(i_st & D13REG_INTSRC_SHORT_PACKET)
            Isr_SHORT_PACKET();
        else if(i_st & D13REG_INTSRC_RESUME)
        {
            printf("RESUME\n");
            Hal4D13_LockDevice(0);
        }
        else 
        {
            if(i_st & D13REG_INTSRC_EP0OUT)  
                Isr_Ep00RxDone();
            if(i_st & D13REG_INTSRC_EP0IN)
                Isr_Ep00TxDone();
            if(i_st & D13REG_INTSRC_EP01)
            	Isr_Ep01Done();
        }
    }
    enable_irq();
	bD13flags.bits.At_IRQL1 = 0;
}

void Isr_BusReset(void)
{
    printf("BUS RESET\n");
    reconnect_USB_controller();
    bD13flags.bits.DCP_state = 0x00;
    bD13flags.bits.dma_disable=0x00;
    hid_settings.state = reset;
    bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
    bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
    bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
    bUSBCheck_Device_State.State_bits.RESET_BITS = 0;

	new_report.modifier = 0x00;
	new_report.reserved = 0x00;
	new_report.keycode[0] = 0x00;

    //TODO: Reset Everything needed to reset program. Basically.

    config_endpoints();
}

void Isr_SuspendChange(void)
{
    bD13flags.bits.suspend=1;
}

void Isr_SHORT_PACKET(void)
{
    printf("SHORT PACKET INTERRUPT\n");
}

void Isr_DmaEot(void)
{
    bD13flags.bits.dma_disable =0;

    if(bD13flags.bits.dma_state == DMA_PENDING)
        bD13flags.bits.setup_dma = 1;
    else
        bD13flags.bits.dma_state = DMA_IDLE;
    printf("EOT reached!!!\n");
    if(bD13flags.bits.verbose==1)
        printf("dma_counter = %x\n", Hal4D13_GetDMACounter());
    printf("\n");       
}


void Isr_SOF(void)
{

}

void Isr_Ep00RxDone(void)
{
    UCHAR       ep_last, i;
    //printf("RX %d\n", bUSBCheck_Device_State.State_bits.FOUR_RESERVED_BITS++);
    ep_last = Hal4D13_GetEndpointStatusWInteruptClear(EPINDEX4EP0_CONTROL_OUT); /* Clear interrupt flag */


    if (ep_last & D13REG_EPSTS_SETUP)
    {
        if(bD13flags.bits.DCP_state == USBFSM4DCP_REQUESTPROC)
        {
            ControlData.Abort = 1;
            bD13flags.bits.DCP_state = USBFSM4DCP_SETUPPROC;
        }
        else
            bD13flags.bits.DCP_state = USBFSM4DCP_SETUPPROC;
        //printf("StateMain : %x\n",bD13flags.bits.DCP_state);
    }
    else /* not a setup packet, just a Data Out Packet */
    {
    	//printf("StateElse : %x\n",bD13flags.bits.DCP_state);
        switch (bD13flags.bits.DCP_state)
        {

        case USBFSM4DCP_DATAOUT:

            i = Hal4D13_ReadEndpoint(EPINDEX4EP0_CONTROL_OUT, ControlData.dataBuffer + ControlData.wCount,
                EP0_PACKET_SIZE);

            ControlData.wCount += i;
            if( i != EP0_PACKET_SIZE || ControlData.wCount >= ControlData.wLength)
            {
                bD13flags.bits.DCP_state = USBFSM4DCP_REQUESTPROC;

            }
            break;
        case USBFSM4DCP_HANDSHAKE:
        	//printf("HANDSHAKE\n");
            bD13flags.bits.DCP_state = USBFSM4DCP_IDLE;
            break;

        case USBFSM4DCP_STALL:      break;

        case USBFSM4DCP_SETUPPROC:  break;

        case USBFSM4DCP_REQUESTPROC:break;

        case USBFSM4DCP_DATAIN:     break;

        default:
            printf("Fatal Error: Isr_Ep0RxDone ReadLastTranactionStatus=%x\n",ep_last);
            bD13flags.bits.DCP_state = USBFSM4DCP_STALL;
            Hal4D13_StallEP0InControlWrite();
            break;
        }
    }
    return ;
}

void Isr_Ep00TxDone(void)
{
	/* Control IN interrupt */
    short i = ControlData.wLength - ControlData.wCount;
    UCHAR ep_last;
    Hal4D13_ReadInterruptRegister();
    ep_last = Hal4D13_GetEndpointStatusWInteruptClear(EPINDEX4EP0_CONTROL_IN); /* Clear interrupt flag */

    switch (bD13flags.bits.DCP_state)
    {
    	case USBFSM4DCP_HANDSHAKE:
    		break;
    	case USBFSM4DCP_DATAIN:
    		if( i >= EP0_PACKET_SIZE)
			{
				Hal4D13_WriteEndpoint(EPINDEX4EP0_CONTROL_IN,ControlData.Addr.pData + ControlData.wCount, EP0_PACKET_SIZE);
				ControlData.wCount += EP0_PACKET_SIZE;
				// State remains at USBFSM4DCP_DATAIN
			}
			else if( i != 0)
			{
				Hal4D13_WriteEndpoint(1, ControlData.Addr.pData + ControlData.wCount, i);
				ControlData.wCount += i;
				bD13flags.bits.DCP_state = USBFSM4DCP_HANDSHAKE;
			}
			else if (i == 0)
			{
				bD13flags.bits.DCP_state = USBFSM4DCP_HANDSHAKE;
				Hal4D13_SingleTransmitEP0(0, 0);
			}
			break;
		case USBFSM4DCP_REQUESTPROC: break;
		case USBFSM4DCP_IDLE:     	 break;
		case USBFSM4DCP_SETUPPROC:   break;
		case USBFSM4DCP_STALL:       break;
		case USBFSM4DCP_DATAOUT:     break;
		default:
			printf("Fatal Error: Isr_Ep0TxDone Unexpected FSMState=%x\n",bD13flags.bits.DCP_state);
			bD13flags.bits.DCP_state = USBFSM4DCP_STALL;
			Hal4D13_StallEP0InControlRead();
			break;
		}
}

void Isr_Ep01Done(void) {
	/* This gets called when the interrupt is determined to have came from
	 * endpoint 1. When this interrupt it fired it means the host has received
	 * a packet from this endpoint.
	 */
	printf("Packet Received Dood.\n");

	//I'm currenly using this to set the ready to send packet to high, but probably I will just check to see if the buffers are full in each iteration.
	bD13flags.bits.DCP_state = USBFSM4DCP_INTR1DONE;

	UCHAR err = Hal4D13_GetErrorCode(EPINDEX4EP01);
	UCHAR stat = Hal4D13_GetEndpointStatusWInteruptClear(EPINDEX4EP01); //Interrupt Clear

	// print endpoint status and error registers
	//printf("Status = %x : Err = %x\n", stat, err);
}


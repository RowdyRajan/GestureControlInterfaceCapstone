
#include <stdio.h>
#include <string.h>
#include "BasicTyp.h"
#include "common.h"
#include "usb.h"
#include "chap_9.h"
#include "Hal4D13.h"
#include "ISO.H"
#include "MAINLOOP.H"


// *************************************************************************
// Public Data
// *************************************************************************

extern D13FLAGS bD13flags;
extern USBCHECK_DEVICE_STATES bUSBCheck_Device_State;
extern CONTROL_XFER ControlData;

UCHAR ALTERNATIVE_SETTING = 0;

// *************************************************************************
// USB Device Descriptor
// *************************************************************************

#define NUM_ENDPOINTS   4

#define NUM_INTERFACE   	2
#define NUM_ALTINTERFACE    2

#define USB_CLASS_CODE_TEST_CLASS_DEVICE                    0xdc
#define USB_SUBCLASS_CODE_TEST_CLASS_D12                    0xA0
#define USB_PROTOCOL_CODE_TEST_CLASS_D12                    0xB0

#define EP0_TX_FIFO_SIZE   64		//16
#define EP0_RX_FIFO_SIZE   64		//16
//#define EP0_PACKET_SIZE    64		//16

#define EP1_TX_FIFO_SIZE   16
#define EP1_RX_FIFO_SIZE   16
#define EP1_PACKET_SIZE    16

#define EP2_TX_FIFO_SIZE   64
#define EP2_RX_FIFO_SIZE   64
#define EP2_PACKET_SIZE    64


//#define CONFIG_DESCRIPTOR_LENGTH    NUM_ALTINTERFACE * sizeof(USB_CONFIGURATION_DESCRIPTOR) + NUM_ALTINTERFACE * sizeof(USB_INTERFACE_DESCRIPTOR) + NUM_ENDPOINTS * sizeof(USB_ENDPOINT_DESCRIPTOR)

//#define CONFIG_DESCRIPTOR_LENGTH    sizeof( USB_CONFIGURATION_DESCRIPTOR)//NUM_ALTINTERFACE * sizeof(USB_CONFIGURATION_DESCRIPTOR) +NUM_ALTINTERFACE * sizeof(USB_INTERFACE_DESCRIPTOR) +4 * NUM_ALTINTERFACE * sizeof(USB_ENDPOINT_DESCRIPTOR) 

USB_DEVICE_DESCRIPTOR DeviceDescr =//<<
{
		18,							//bLength
		USB_DEVICE_DESCRIPTOR_TYPE,	//bDescriptorType
		0x10,						//bcdUSB_L
		0x01,						//bcdUSB_H
		0x00,						//bDeviceClass	-- Either 0x00 (defer to interface class) OR 0x03 HID class (But I don't know about multiple interfaces in this mode)
		0x00,						//bDeviceSubClass
		0x00,						//bDeviceProtocol
		64,							//bMaxPacketSize0
		0xcc, //cc					//idVendor_L -- Vendor ID for PHILIPS. There is an alternate, see CD code
		0x04, //04					//idVendor_H
		0x62, //2B					//idProduct_L -- Product ID for ISP 1362. Alternate 0x1a62
		0x1a, //C5					//idProduct_H
		0x00,						//bcdDevice_L
		0x01,						//bcdDevice_H
		0x02,						//iManufacturer --STR_INDEX_MANUFACTURER
		0x03,						//iProduct --STR_INDEX_PRODUCT
		0x00,						//iSerialNumber --STR_INDEX_SERIALNUMBER
		0x01						//bNumConfigurations
};

USB_CONFIGURATION_DESCRIPTOR_a ConfigDescr_a=//<<
{
		0x09,	//bLength
		USB_CONFIGURATION_DESCRIPTOR_TYPE, 		//bDescriptorType
		0x22, //wTotalLength_L
		0x00, 									//wTotalLength_H, always 00
		0x01, 									//bNumInterfaces
		0x01, 									//bConfigurationValue
		0x00, 									//iConfiguration string index
		0x60, 	//0xC0							//bmAttributes, Want self powered and remote wakeup
		0x00,	//0x01							//MaxPower: Self Powered. This maybe should be zero... I'll check.

		/* Interface Descriptor: HID Keyboard */
		0x09,  //bLength
		USB_INTERFACE_DESCRIPTOR_TYPE,	//descriptor type
		0x00,	// Interface Number
		0x00,	// Alternate Number
		0x01,  // Number of Endpoints
		USB_DEVICE_CLASS_HUMAN_INTERFACE, 		// Interface Class
		0, 										//USB_SUBCLASS_HID_BOOTINTERFACE,			// SubClass : Usually zero unless you're an HID
		0, 										//USB_HID_INTERFACE_PROTOCOL_KEYBOARD,		// Protocol : Also mostly only used in HIDs
		STR_INDEX_INTERFACE,

		/* HID Descriptor: Keyboard */
		0x09, //bLength
		0x21,		// This is of HID descriptor type
		0x01,		// HID spec. version LOW BIT
		0x01,		// HID spec. version HIGH BIT
		0x00,		// No country code
		0x01,		// One additional descriptor
		0x22,		// of type HID Report
		0x2D,		// bDescriptorLength_L
		0x00,		// bDescriptorLength_H

		/* HID Keyboard Endpoint Descriptor: ENDP02 Interrupt IN */
		0x07, 	//bLength
		USB_ENDPOINT_DESCRIPTOR_TYPE,		//bDescriptorType
		0x81,								//bEndpointAddress --ENDP01 IN (I think the 8 means IN and 0 is OUT)
		USB_ENDPOINT_TYPE_INTERRUPT,		//bmAttributes
		0x08,								//wMaxPacketSize -- This is size of input report. Configure Endpoint accordingly
		0x00,								//wMaxPacketSize
		0x14								//bInterval -- Interval for polling in milliseconds
};

USB_CONFIGURATION_DESCRIPTOR ConfigDescr =
{
		sizeof(USB_CONFIGURATION_DESCRIPTOR),	//bLength
		USB_CONFIGURATION_DESCRIPTOR_TYPE, 		//bDescriptorType
		sizeof(USB_CONFIGURATION_DESCRIPTOR_a), //wTotalLength_L
		0x00, 									//wTotalLength_H, always 00
		0x01, 									//bNumInterfaces
		0x01, 									//bConfigurationValue
		0x00, 									//iConfiguration
		0xC0, 									//bmAttributes, Want self powered and remote wakeup
		0x00
};

USB_INTERFACE_DESCRIPTOR InterfaceDescr0 =
{
		sizeof(USB_INTERFACE_DESCRIPTOR),  //bLength
		USB_INTERFACE_DESCRIPTOR_TYPE,	//descriptor type
		0,	// Interface Number
		0,	// Alternate Number
		1,  // Number of Endpoints
		USB_DEVICE_CLASS_HUMAN_INTERFACE, 		// Interface Class
		USB_SUBCLASS_CODE_UNKNOWN, 				//USB_SUBCLASS_HID_BOOTINTERFACE,			// SubClass : Usually zero unless you're an HID
		USB_PROTOCOL_CODE_UNKNOWN, 	//USB_HID_INTERFACE_PROTOCOL_KEYBOARD,		// Protocol : Also mostly only used in HIDs
		STR_INDEX_INTERFACE
};

/*
USB_INTERFACE_DESCRIPTOR InterfaceDescr1 =
{
    sizeof(USB_INTERFACE_DESCRIPTOR),
    USB_INTERFACE_DESCRIPTOR_TYPE,
    0,//bInterfaceNumber
    1,//bAlternateSetting
    NUM_ENDPOINTS,
    USB_CLASS_CODE_UNKNOWN,
    USB_SUBCLASS_CODE_UNKNOWN,
    USB_PROTOCOL_CODE_UNKNOWN,
    0//STR_INDEX_INTERFACE
};
*/
/*
USB_ENDPOINT_DESCRIPTOR EP_Descr[4] =
{

    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        0x03,//EP03, 0x4, OUT
        USB_ENDPOINT_TYPE_BULK,
        SWAP(NONISO_FIFOSIZE_64),
        0
    },


    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        0x84,// EP4, 0x5, IN
        USB_ENDPOINT_TYPE_BULK,
        SWAP(NONISO_FIFOSIZE_64),
        0
    },


    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        0x05,// EP5 ISO OUT
        USB_ENDPOINT_TYPE_ISOCHRONOUS,
        SWAP(ISO_FIFOSIZE_512),
        1
    },


    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        0x86,// EP6 ISO IN
        USB_ENDPOINT_TYPE_ISOCHRONOUS,
        SWAP(ISO_FIFOSIZE_512),
        1
    },
};
*/

unsigned char code REPORT_DESCRIPTOR2[45] =
{
		0x05, 0x01,	// USAGE_PAGE (Generic_Desktop)
		0x09, 0x06, // USAGE (Keyboard)
		0xa1, 0x01, // COLLECTION (Application)
		0x05, 0x07,	// USAGE_PAGE (Keyboard)
		0x19, 0xe0,	// USAGE_MINIMUM (Keyboard LeftControl)
		0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
		0x15, 0x00, // LOGICAL_MINIMUM (0)
		0x25, 0x01, // LOGICAL_MAXIMUM (1)
		0x75, 0x01, // REPORT_SIZE (1)
		0x95, 0x08, // REPORT_COUNT (8)
		0x81, 0x02, // INPUT (Data,Var,Abs)
		0x95, 0x01, // REPORT_COUNT (1)
		0x75, 0x08, // REPORT_SIZE (8)
		0x81, 0x03, // INPUT (Cnst,Var,Abs)
		0x95, 0x06, // REPORT_COUNT (6)
		0x75, 0x08, // REPORT_SIZE (8)
		0x15, 0x00,	// LOGICAL_MINIMUM (0)
		0x25, 0x65, // LOGICAL_MAXIMUM (101)
		0x05, 0x07,	// USAGE_PAGE (Keyboard)
		0x19, 0x00,	// USAGE_MINIMUM (Reserved (no event indicated))
		0x29, 0x65,	// USAGE_MAXIMUM (Keyboard Application)
		0x81, 0x00,	// INPUT (Data,Ary,Abs)
		0xc0		// END_COLLECTION
};

unsigned char code REPORT_DESCRIPTOR[45] =
{
		0x05,	//USAGE_PAGE (Generic_Desktop)
		0x01,
		0x09,	//USAGE (Keyboard)
		0x06,
		0xA1,	//COLLECTION (Application)
		0x01,

		0x05,	//USAGE_PAGE (Keyboard)
		0x07,
		0x19,	//USAGE_MINIMUM (Keyboard LeftControl)
		0xE0,
		0x29,	//USAGE_MAXIMUM (Keyboard Right GUI)		29 E7
		0xE7,
		0x15,	//LOGICAL_MINIMUM (0)						15 00
		0x00,
		0x25,	//LOGICAL_MAXIMUM (1)						25 01
		0x01,
		0x75,	//REPORT_SIZE (1)							75 01
		0x01,
		0x95,	//REPORT_COUNT (8)							95 08
		0x08,
		0x81,	//INPUT (Data,Var,Abs)						81 02
		0x02,
		0x75,	//REPORT_SIZE (8)							75 08
		0x08,
		0x95,	//REPORT_COUNT (1)							95 01
		0x01,
		0x81,	//INPUT (Cnst,Ary,Abs)						81 01
		0x01,
		0x09,	//USAGE (Reserved [no event])				09 00
		0x00,
		0x09,	//USAGE (Keyboard Tab)						09 2B
		0x2B,
		0x09,	//USAGE (Keyboard PageUp)					09 4B
		0x4B,
		0x09,	//USAGE (Keyboard PageDown)					09 4E
		0x4E,
		0x95,	//REPORT_COUNT (1)							95 01
		0x01,
		0x15,	//LOGICAL_MINIMUM (0)						15 00
		0x00,
		0x25,	//LOGICAL_MAXIMUM (3)						25 02
		0x02,
		0x81,	//INPUT (Data,Ary,Abs)						81 00
		0x00,
		0xC0	//END_COLLECTION	C0
};

USB_STRING_LANGUAGE_DESCRIPTOR  strLanguage =
{
  sizeof(USB_STRING_LANGUAGE_DESCRIPTOR),
  USB_STRING_DESCRIPTOR_TYPE,
  0x09,
  0x04
};


USB_STRING_INTERFACE_DESCRIPTOR  strInterface =
{
    sizeof(USB_STRING_INTERFACE_DESCRIPTOR),
    USB_STRING_DESCRIPTOR_TYPE,
    {'I',0,'n',0,'t',0,'e',0,'r',0,'f',0,'a',0,'c',0,'H',0,'I',0,'D',0}
};

USB_STRING_CONFIGURATION_DESCRIPTOR  strConfiguration =
{
    sizeof(USB_STRING_CONFIGURATION_DESCRIPTOR),
    USB_STRING_DESCRIPTOR_TYPE,
    {
    'C',0,
    'o',0,
    'n',0,
    'f',0,
    'i',0,
    'g',0,
    '0',0,
    '0',0
    }
};

USB_STRING_SERIALNUMBER_DESCRIPTOR  strSerialNum =
{
    sizeof(strSerialNum),
    USB_STRING_DESCRIPTOR_TYPE,
   {
    'M',0,
    'y',0,
    'D',0,
    'e',0,

    'v',0,
    'i',0,
    'c',0,
    'e',0,

    'K',0,
    'e',0,
    'y',0,
    'B',0
   }
};

USB_STRING_PRODUCT_DESCRIPTOR  strProduct =
{
    sizeof(USB_STRING_PRODUCT_DESCRIPTOR),
    USB_STRING_DESCRIPTOR_TYPE,
    {
    'P', 0,
    'H', 0,
    'I', 0,
    'L', 0,
    'I', 0,
    'P', 0,
    'S', 0,
    ' ', 0,
    'I', 0,
    'S', 0,
    'P', 0,
    '1', 0,
    '1', 0,
    '8', 0,
    '1', 0,
    ' ', 0,
    'T', 0,
    'e', 0,
    's', 0,
    't', 0,
    ' ', 0,
    ' ', 0,
    ' ', 0
    }
};

USB_STRING_MANUFACTURER_DESCRIPTOR  strManufacturer =
{
    sizeof(USB_STRING_MANUFACTURER_DESCRIPTOR),
    USB_STRING_DESCRIPTOR_TYPE,
    {
    'P', 0,
    'H', 0,
    'I', 0,
    'L', 0,
    'I', 0,
    'P', 0,
    'S', 0,
    ' ', 0,
    'S', 0,
    'e', 0,
    'm', 0,
    'i', 0,
    'c', 0,
    'o', 0,
    'n', 0,
    'd', 0,
    'u', 0,
    'c', 0,
    't', 0,
    'o', 0,
    'r', 0,
    's', 0,
    ' ', 0,
    'A', 0,
    'P', 0,
    'I', 0,
    'C', 0}
};

// *************************************************************************
// USB Protocol Layer
// ************************************************************************



// *************************************************************************
// USB standard device requests
// *************************************************************************

void Chap9_GetStatus(void)
{
    UCHAR   endp, txdat[2];
    UCHAR   c;
    UCHAR   bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;


    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
    {
    switch(bRecipient)
            {
            case USB_RECIPIENT_DEVICE:
                    if(bD13flags.bits.remote_wakeup == 1)
                        txdat[0] = DEVSTS_SELFPOWERED|DEVSTS_REMOTEWAKEUP;   /* remote wakeup and Self-powered */
                    else
                        txdat[0] = DEVSTS_SELFPOWERED;   /* Self-powered*/
                    txdat[1]=0;

                    Chap9_SingleTransmitEP0(txdat, 2);
                    break;

            case USB_RECIPIENT_INTERFACE:

                    Chap9_StallEP0InControlRead();
                    break;

            case USB_RECIPIENT_ENDPOINT:

                    if( ControlData.DeviceRequest.wIndex == 0x00 )
                    {
                        endp = -1;
                        c = Hal4D13_GetEndpointStatusWOInteruptClear(endp + 1);
                        if(c & D13REG_EPSTS_STALL)
                            txdat[0] = ENDPSTS_HALT;   /* Halt */
                        else
                            txdat[0] = 0;

                        txdat[1] = 0;

                        Chap9_SingleTransmitEP0(txdat, 2);
                    }
                    else
                        Chap9_StallEP0InControlRead();

                    break;

            default:
                    Chap9_StallEP0InControlRead();
                    break;
            }
    }


    else
    {
        if(ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wLength == 2 )
        {
            switch(bRecipient)
            {
            case USB_RECIPIENT_DEVICE:
                    if(bD13flags.bits.remote_wakeup == 1)
                        txdat[0] = DEVSTS_SELFPOWERED|DEVSTS_REMOTEWAKEUP;   /* remote wakeup and Self-powered */
                    else
                        txdat[0] = DEVSTS_SELFPOWERED;   /* Self-powered*/
                    txdat[1]=0;

                    Chap9_SingleTransmitEP0(txdat, 2);
                    break;

            case USB_RECIPIENT_INTERFACE:
                    txdat[0]=0;
                    txdat[1]=0;

                    Chap9_SingleTransmitEP0(txdat, 2);
                    break;

            case USB_RECIPIENT_ENDPOINT:

                    if( ControlData.DeviceRequest.wIndex == 0x00 )
                        endp = -1;
                    else
                        endp = (UCHAR)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);

                    c = Hal4D13_GetEndpointStatusWOInteruptClear(endp + 1);
                    if(c & D13REG_EPSTS_STALL)
                        txdat[0] = ENDPSTS_HALT;   /* Halt */
                    else
                        txdat[0] = 0;

                    txdat[1] = 0;

                    Chap9_SingleTransmitEP0(txdat, 2);
                    break;

            default:
                    Chap9_StallEP0InControlRead();
                    break;
            }
        }
    }

}

void Chap9_ClearFeature(void)
{
    UCHAR   endp;
    UCHAR   bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;
    USHORT  wFeature = ControlData.DeviceRequest.wValue;

    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
    {
        if( ControlData.DeviceRequest.wLength == 0 )
        {
            switch(bRecipient)
            {
            case USB_RECIPIENT_DEVICE:
                        if(wFeature == USB_FEATURE_REMOTE_WAKEUP)
                        {
                        bD13flags.bits.remote_wakeup = 0;
                        Chap9_SingleTransmitEP0(0, 0);
                        }
                        else
                            Chap9_StallEP0InControlWrite();

                        break;



            case USB_RECIPIENT_ENDPOINT:

                        if(wFeature == USB_FEATURE_ENDPOINT_STALL)
                        {
                            if( ControlData.DeviceRequest.wIndex == 0x00 )
                            {
                                endp = 0;
                                Hal4D13_SetEndpointStatus(endp, 0);

                                Chap9_SingleTransmitEP0(0, 0);
                            }
                            else
                            {
                                Chap9_StallEP0InControlWrite();
                                break;
                            }

                        }

                        else
                            Chap9_StallEP0InControlWrite();

                        break;


            default:
                        Chap9_StallEP0InControlWrite();
                        break;
            }
      }
    }

    else
    {

        if( ControlData.DeviceRequest.wLength == 0 )
        {
            switch(bRecipient)
            {
            case USB_RECIPIENT_DEVICE:
                if(wFeature == USB_FEATURE_REMOTE_WAKEUP)
                {
                    bD13flags.bits.remote_wakeup = 0;
                    Chap9_SingleTransmitEP0(0, 0);
                }
                else
                {
                    Chap9_StallEP0InControlWrite();
                }

                break;

                case USB_RECIPIENT_ENDPOINT:

                if(wFeature == USB_FEATURE_ENDPOINT_STALL)
                {
                    if( ControlData.DeviceRequest.wIndex == 0x00 )
                            endp = -1;
                    else
                        endp = (UCHAR)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);

                    Hal4D13_SetEndpointStatus(endp+1, 0);
                    Hal4D13_SetEndpointStatus(endp+1, 0);

                    Chap9_SingleTransmitEP0(0, 0);
                }
                else
                {
                    Chap9_StallEP0InControlWrite();
                }
                break;

            default:
                    Chap9_StallEP0InControlWrite();
                    break;
            }
      }
    }

}

void Chap9_SetFeature(void)
{
  UCHAR   endp = 0;
    UCHAR   bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;
    USHORT  wFeature = ControlData.DeviceRequest.wValue;
  

    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
    {
        switch(bRecipient)
        {
        case USB_RECIPIENT_DEVICE:
            if(wFeature == USB_FEATURE_REMOTE_WAKEUP)
            {
                bD13flags.bits.remote_wakeup = 1;
                Chap9_SingleTransmitEP0(0, 0);
            }
            else
            {
                Chap9_StallEP0InControlWrite();
            }
            break;

        case USB_RECIPIENT_ENDPOINT:
            if( wFeature == USB_FEATURE_ENDPOINT_STALL)
            {
                if( ControlData.DeviceRequest.wIndex == 0x00 )
                    endp = -1;

                Hal4D13_SetEndpointStatus(endp+1, D13REG_EPSTS_STALL);
                Chap9_SingleTransmitEP0(0, 0);

            }
            else
                Chap9_StallEP0InControlWrite();

            break;
        default:
            Chap9_StallEP0InControlWrite();
            break;
        }
    }


    else
    {
        if( ControlData.DeviceRequest.wLength == 0 )
        {
            switch(bRecipient)
            {
            case USB_RECIPIENT_DEVICE:
                if(wFeature == USB_FEATURE_REMOTE_WAKEUP)
                {
                    bD13flags.bits.remote_wakeup = 1;
                    Chap9_SingleTransmitEP0(0, 0);
                }
                else
                {
                    Chap9_StallEP0InControlWrite();
                }
                break;
            case USB_RECIPIENT_ENDPOINT:
                if( wFeature == USB_FEATURE_ENDPOINT_STALL)
                {
                    if( ControlData.DeviceRequest.wIndex == 0x00 )
                        endp = -1;
                    else
                        endp = (UCHAR)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);

                    Hal4D13_SetEndpointStatus(endp+1, D13REG_EPSTS_STALL);

                    Chap9_SingleTransmitEP0(0, 0);

                }
                else
                {
                    Hal4D13_SetEndpointStatus(endp+1, 0);
                    Chap9_SingleTransmitEP0(0, 0);
                }
                break;
            default:
                Chap9_StallEP0InControlWrite();
                break;
            }
        }
    }



}

void Chap9_SetAddress(void)
{

    UCHAR   j;

    {
        RaiseIRQL();
        disable();
        if(!ControlData.Abort)
        {
            if(bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE)
            {
                if((ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK) != 0 )
                {
                    bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 0;
                    Hal4D13_SetAddressEnable((UCHAR)(ControlData.DeviceRequest.wValue &
                                            DEVICE_ADDRESS_MASK), 1);
                    bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 1;
                    Chap9_SingleTransmitEP0(0, 0);

                }

                else
                    bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
            }

            else if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
            {
                if((ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK) == 0 )
                {
                    Hal4D13_SetAddressEnable((UCHAR)(ControlData.DeviceRequest.wValue &
                                            DEVICE_ADDRESS_MASK), 1);
                    bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
                    bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
                    Chap9_SingleTransmitEP0(0, 0);

                }

                else
                {
                    bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 0;
                    Hal4D13_SetAddressEnable((UCHAR)(ControlData.DeviceRequest.wValue &
                                            DEVICE_ADDRESS_MASK), 1);
                    bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 1;
                    Chap9_SingleTransmitEP0(0, 0);

                }

            }

            else
            {

                bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
                bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
                bUSBCheck_Device_State.State_bits.DEVICE_DEFAULT_STATE = 1;
                j = DEV_BEHAVIOUR_NOT_SPECIFIED;
                Chap9_SingleTransmitEP0(&j, 0);
            }

        }
        LowerIRQL();
        enable();

    }
    printf("Addr %x\n",Hal4D13_GetAddress());
}


void Chap9_GetDescriptor(void)
{
    UCHAR   bDescriptor =      MSB(ControlData.DeviceRequest.wValue);
    UCHAR   bDescriptorIndex = LSB(ControlData.DeviceRequest.wValue);
    UCHAR	bmRequestType = ControlData.DeviceRequest.bmRequestType;
    USHORT	wIndex = ControlData.DeviceRequest.wIndex;
    USHORT	wLen = ControlData.DeviceRequest.wLength;
    /*if(bmRequestType == 0x82) {
    printf("bmRequestType = %x\n", bmRequestType);
    printf("bDescri = %d : ",bDescriptor);
    printf("Index = %d\n",bDescriptorIndex);
    printf("Interface = %d\n", wIndex);
    printf("Length = %x\n", wLen);
    }*/
//  printf("USB_DEVICE_DESCRIPTORsize %d\n",sizeof(USB_DEVICE_DESCRIPTOR));
//    printf("CONFIGURATIONsize %ld\n", sizeof(USB_CONFIGURATION_DESCRIPTOR_a));
    switch(bDescriptor)
    {
    //sDevice sConfiguration
    //sizeof(struct CONFIGURATION)
//sizeof( struct DEVICE)

    case USB_DEVICE_DESCRIPTOR_TYPE:{//1

        Chap9_BurstTransmitEP0( (PUCHAR)&DeviceDescr , sizeof(USB_DEVICE_DESCRIPTOR));
//      Chap9_BurstTransmitEP0((PUCHAR)&sDevice,sizeof(struct DEVICE));
        break;}
    case USB_CONFIGURATION_DESCRIPTOR_TYPE:{//2
//      Chap9_BurstTransmitEP0((PUCHAR)&ConfigDescr, CONFIG_DESCRIPTOR_LENGTH);
    	//printf("CONFIGURATIONsize %x\n", sizeof(USB_CONFIGURATION_DESCRIPTOR_a));
      
            //printf("\nConfigDescr     \n");  
    	Chap9_BurstTransmitEP0((PUCHAR)&ConfigDescr_a, sizeof(USB_CONFIGURATION_DESCRIPTOR_a));//sizeof(USB_CONFIGURATION_DESCRIPTOR));
       //Chap9_SetConfiguration();
        
      //  config_endpoint();
   //       printf("\nInterfaceDescr0  ");    Chap9_BurstTransmitEP0((PUCHAR)&InterfaceDescr0, sizeof(USB_INTERFACE_DESCRIPTOR));
//          printf("\nEP1_TXDesc       ");     Chap9_BurstTransmitEP0((PUCHAR)&EP1_TXDescr, sizeof(USB_ENDPOINT_DESCRIPTOR));
  //        printf("\nEP1_RXDescr      ");    Chap9_BurstTransmitEP0((PUCHAR)&EP1_RXDescr, sizeof(USB_ENDPOINT_DESCRIPTOR));
    //      printf("\nEP2_TXDescr      ");    Chap9_BurstTransmitEP0((PUCHAR)&EP2_TXDescr, sizeof(USB_ENDPOINT_DESCRIPTOR));
      //    printf("\nEP2_RXDescr      ");    Chap9_BurstTransmitEP0((PUCHAR)&EP2_RXDescr, sizeof(USB_ENDPOINT_DESCRIPTOR));
        break;}
    case USB_STRING_DESCRIPTOR_TYPE://3
        switch(bDescriptorIndex)
        {
        case STR_INDEX_LANGUAGE:
            printf("\n str1     \n");
            Chap9_BurstTransmitEP0((PUCHAR)&strLanguage, sizeof(USB_STRING_LANGUAGE_DESCRIPTOR));
            break;
        case STR_INDEX_MANUFACTURER:
            printf("\n str2     \n");Chap9_BurstTransmitEP0((PUCHAR)&strManufacturer, sizeof(USB_STRING_MANUFACTURER_DESCRIPTOR));
            break;
        case STR_INDEX_PRODUCT:
            printf("\n str3     \n");Chap9_BurstTransmitEP0((PUCHAR)&strProduct, sizeof(USB_STRING_PRODUCT_DESCRIPTOR));
            break;
        case STR_INDEX_SERIALNUMBER:
            printf("\n str4     \n");Chap9_BurstTransmitEP0((PUCHAR)&strSerialNum, sizeof(USB_STRING_SERIALNUMBER_DESCRIPTOR));
            break;
        case STR_INDEX_CONFIGURATION:
            printf("\n str5     \n");Chap9_BurstTransmitEP0((PUCHAR)&strConfiguration, sizeof(USB_STRING_CONFIGURATION_DESCRIPTOR) );
            break;
        case STR_INDEX_INTERFACE:
            printf("\n str6     \n");Chap9_BurstTransmitEP0((PUCHAR)&strInterface, sizeof(USB_STRING_INTERFACE_DESCRIPTOR) );
            break;
        default:
            printf("\n str7     \n");Chap9_StallEP0InControlRead();
            break;
        }
        break;
    case USB_INTERFACE_DESCRIPTOR_TYPE:
    	printf("Interface Req\n");
    	break;
    case USB_ENDPOINT_DESCRIPTOR_TYPE:
    case USB_POWER_DESCRIPTOR_TYPE:
    case USB_HID_DESCRIPTOR_TYPE:
    	if(bmRequestType == 0x81) {
    		Chap9_BurstTransmitEP0((PUCHAR)&REPORT_DESCRIPTOR2, sizeof(REPORT_DESCRIPTOR2) );
    	}
    	break;
    default:
        Chap9_StallEP0InControlRead();
        break;
    }
}

void Chap9_GetConfiguration(void)
{
    UCHAR   c = bD13flags.bits.configuration;
    UCHAR   j;

    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
    {
        j = 0;
        Chap9_SingleTransmitEP0(&j, 1);
    }

    else
    {
        if (ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wIndex == 0 && ControlData.DeviceRequest.wLength == 1)
        {

        Chap9_SingleTransmitEP0(&c, 1);
        }
    }

}

void Chap9_SetConfiguration(void)
{
  
    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
    {
        if(ControlData.DeviceRequest.wIndex == 0 && ControlData.DeviceRequest.wLength == 0)
        {
            if (ControlData.DeviceRequest.wValue == 0)
            {
                /* put device in unconfigured state */
                bD13flags.bits.configuration = 0;
                bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 1;
                Chap9_SingleTransmitEP0(0, 0);
            }
            else if (ControlData.DeviceRequest.wValue == 1)
            {
                /* Configure device */
                bD13flags.bits.configuration = 1;
                bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
                bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 1;
                Chap9_SingleTransmitEP0(0, 0);
            }

            else
                Chap9_StallEP0InControlWrite();
        }
    }

    else
    {
        if (ControlData.DeviceRequest.wValue == 0)
        {
            /* put device in unconfigured state */
            bD13flags.bits.configuration = 0;
            bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 1;
            bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 0;
            Chap9_SingleTransmitEP0(0, 0);

        }

        else if (ControlData.DeviceRequest.wValue == 1)
        {
            /* Configure device */
            bD13flags.bits.configuration = 1;
            bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE = 0;
            bUSBCheck_Device_State.State_bits.DEVICE_CONFIGURATION_STATE = 1;
            Chap9_SingleTransmitEP0(0, 0);
        }
        else
            Chap9_StallEP0InControlWrite();
    }

}

void Chap9_GetInterface(void)
{

    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
        Chap9_StallEP0InControlRead();


    else
    {

        if ((ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wIndex == 0 && ControlData.DeviceRequest.wLength == 1))
            Chap9_SingleTransmitEP0(&ALTERNATIVE_SETTING, 1);      //txdat
        else
            Chap9_StallEP0InControlRead();
    }

}

void Chap9_SetInterface(void)
{
    UCHAR   dir = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_DIR_MASK;

    if(dir)
        Chap9_StallEP0InControlRead();


    if(bUSBCheck_Device_State.State_bits.DEVICE_ADDRESS_STATE)
        Chap9_StallEP0InControlRead();



    else
    {

        if (ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wIndex == 0)
        {
            ALTERNATIVE_SETTING = 0;
            Chap9_SingleTransmitEP0(0,0);
        }

        else if (ControlData.DeviceRequest.wValue == 1 && ControlData.DeviceRequest.wIndex == 0)
        {
            ALTERNATIVE_SETTING = 1;
            Chap9_SingleTransmitEP0(0,0);
        }
        else
        {
            Chap9_StallEP0InControlWrite();
        }
    }

}

// *************************************************************************
// Chap9 support functions
// *************************************************************************

void Chap9_StallEP0(void)
{
    UCHAR dir;
    dir = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_DIR_MASK;
    if(dir)
    {
        Hal4D13_StallEP0InControlRead();
    }
    else
    {
        Hal4D13_StallEP0InControlWrite();
    }

    RaiseIRQL();
    if(!ControlData.Abort)
    {
        bD13flags.bits.DCP_state = USBFSM4DCP_STALL;

    }
    LowerIRQL();
}
void Chap9_StallEP0InControlWrite(void)
{
    Hal4D13_StallEP0InControlWrite();

    RaiseIRQL();
    if(!ControlData.Abort)
    {
        bD13flags.bits.DCP_state = USBFSM4DCP_STALL;

    }
    LowerIRQL();
}

void Chap9_StallEP0InControlRead(void)
{
    Hal4D13_StallEP0InControlRead();
    RaiseIRQL();
    if(!ControlData.Abort)
    {
        bD13flags.bits.DCP_state = USBFSM4DCP_STALL;

    }
    LowerIRQL();
}

void Chap9_SingleTransmitEP0(PUCHAR buf, USHORT len)
{

    Hal4D13_SingleTransmitEP0(buf, len);

    RaiseIRQL();
    if(!ControlData.Abort)
    {
        ControlData.wLength = ControlData.wCount = len;
        bD13flags.bits.DCP_state = USBFSM4DCP_HANDSHAKE;

    }
    LowerIRQL();
}

void Chap9_BurstTransmitEP0(PUCHAR pData, USHORT len)
{
    ControlData.wCount = 0;
    if(ControlData.wLength > len)
        ControlData.wLength = len;

    ControlData.Addr.pData = pData;

        if( ControlData.wLength >= EP0_PACKET_SIZE)
        {
            Hal4D13_WriteEndpoint(EPINDEX4EP0_CONTROL_IN, pData, EP0_PACKET_SIZE);

            RaiseIRQL();
            if(!ControlData.Abort)
            {
                ControlData.wCount += EP0_PACKET_SIZE;
                bD13flags.bits.DCP_state = USBFSM4DCP_DATAIN;

            }
            LowerIRQL();
        }
        else
        {
            Hal4D13_WriteEndpoint(EPINDEX4EP0_CONTROL_IN, pData, ControlData.wLength);

            RaiseIRQL();
            if(!ControlData.Abort)
            {
                ControlData.wCount += ControlData.wLength;
                bD13flags.bits.DCP_state = USBFSM4DCP_HANDSHAKE;

            }
            LowerIRQL();
        }
}



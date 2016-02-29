/*Code created with help of tutorial https://github.com/gregkh/usb_tutorial*/

//This driver will detect if the DE2 is plugged in and will print in the case it is. There is currenly also an experimental packet sending implemented

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

//Defining the ids for the DE2
#define VENDOR_ID	0x04CC
#define PRODUCT_ID	0x0021

//random init value to send. Has to be the same
//as the one expected by DE2
#define INIT 0x56
MODULE_AUTHOR("Group 2");
MODULE_DESCRIPTION("De2 driver");
MODULE_LICENSE("GPL");

/* table of devices that work with this driver */
static struct usb_device_id usb_tables[] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ },
};

MODULE_DEVICE_TABLE(usb, usb_tables);

/*
Struct for communication
*/
struct de2 {
	struct usb_device* udev;
	int command;
};

struct output_packet {
	u8	cmd;
	u8	params[7];
} __attribute__ ((packed));

//Sends a command to the de2 device
static int send_cmd(struct de2* de2usb, u8 cmd){
	struct output_packet* packet;
	int return_value;

	packet = kmalloc(sizeof(*packet), GRP_KERNEL);
	if(!packet){
		return -ENOMEM;
	}
	packet->cmd = cmd;
	//Send a packet. Values used defined udeer standard	
	return_value = usb_control_msg(de2->udev, usb_sndctrlpipe(de2->udev,0),
									0x09, 0x21, 0x0200,0x0000, packet, sizeof(*packet),10000);

	//If it returns a packet was a packet size 
	if(return_value == sizeof(*packet)){
		return_value = 0;
	}

	kfree(packet);
	return return_value;
} 
//Function that usb core calls when detects a usb device device
static int de2_probe(struct usb_interface* interface, const struct usb_device_id* id){
	//Create the usb device in memory 
	struct usb_device* udev = interface_to_usbdev(interface);
	struct de2 *usbDe2;

	usbDe2 = kzalloc(sizeof(*usbDe2), GFP_KERNEL);
	if (usbDe2 == NULL) {
		dev_err(&interface->dev, "Out of memory\n");
		retuusbDe2rn -ENOMEM;
	}
	
	usbDe2->udev = usb_get_dev(udev);
	usb_set_intfdata(interface, usbDe2);
	
	//Send a init command
	send_cmd(usbDe2, INIT);
	dev_info(&interface->dev, "DE2 board is now attached\n");
	return 0;
}

//Will free memory and print a message on disconect
static void de2_disconnect(struct usb_interface *interface){
	struct de2 *usbDe2;
	//Freeing memory on disconect
	usbDe2 = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(usbDe2->udev);

	kfree(usbDe2);

	dev_info(&interface->dev, "DE2 disconnected\n");
}

//Creating a proper structure to load in to usb core
static struct usb_driver de2_driver = {
	.name =		"de2",
	.probe =	de2_probe,
	.disconnect =	de2_disconnect,
	.id_table =	usb_tables,
};

//Loading driver
static int __init de2_init(void)
{
	return usb_register(&de2_driver);
}

//Unregisetering driver
static void __exit de2_exit(void)
{
	usb_deregister(&de2_driver);
}

module_init(de2_init);
module_exit(de2_exit);



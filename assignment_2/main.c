#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/usb.h>	
#include<linux/timer.h>	
#include<linux/slab.h>

/********************************************MACROS************8*************************************/
#define HP_PENDRIVE_VID 0x03f0					//HP pendrive Id's 
#define HP_PENDRIVE_PID 0x5a07

#define SANDISK_PENDRIVE_VID 0x0781				//Sandisk pendrive Id's
#define SANDISK_PENDRIVE_PID 0x55a9

#define KINGSTON_PENDRIVE_VID 0x0951						//Kingston pendrive Id's
#define KINGSTON_PENDRIVE_PID 0x1446

#define MOSERBAER_PENDRIVE_VID 0x0781					//Moserbaer pendrive Id's
#define MOSERBAER_PENDRIVE_PID 0x5a37

#define READ_CAPACITY_LENGTH 0x08
#define be_to_int32(buf) (((buf)[0]<<24)|((buf)[1]<<16)|((buf)[2]<<8)|(buf)[3])

#define RETRY_MAX    			5
#define LIBUSB_ERROR_PIPE      -9
#define SUCCESS		 	        0
#define log(fmt,...) ({ printk(KERN_INFO "%s: ",__func__); \printk(fmt, ##__VA_ARGS__); \})
/*****************************************************************************************************/

enum endpoint_directions {
	USB_ENDP_IN = 0x80,
	USB_ENDP_OUT = 0x00
};

/***************************************STRUCTURES****************************************************/
struct usbdev_private		//private structure which is defined for our driver
{
	struct usb_device *udev;  
	unsigned char class;
	unsigned char subclass;
	unsigned char protocol;
	unsigned char ep_in;
	unsigned char ep_out;
};

struct command_block_wrapper {
	uint8_t dCBWSignature[4];
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
};

struct command_status_wrapper {
	uint8_t dCSWSignature[4];
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
};
/*****************************************************************************************************/

static uint8_t cdb_length[256] = {
//	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};

/*********************************************FUNCTIONS**************************************************/
int send_command_block(struct usb_device*,uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t, uint8_t*);
int get_command_status(struct usb_device*, uint8_t,uint8_t);
int read_capacity(struct usb_device *, uint8_t, uint8_t);
/********************************************************************************************************/

struct usbdev_private *p_usbdev_info;

static void usbdev_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "USBDEV Device Removed\n");
	return;
}

static struct usb_device_id usbdev_table [] = {		//device id table
	{USB_DEVICE(HP_PENDRIVE_VID, HP_PENDRIVE_PID)},
	{USB_DEVICE(SANDISK_PENDRIVE_VID, SANDISK_PENDRIVE_PID)},
	{USB_DEVICE(KINGSTON_PENDRIVE_VID, KINGSTON_PENDRIVE_PID)},
	{USB_DEVICE(MOSERBAER_PENDRIVE_VID, MOSERBAER_PENDRIVE_PID)},
	{}						//terminating entry
};

/*********************************************Capacity_Read_Function**************************************************/
int read_capacity(struct usb_device *device, uint8_t ep_in, uint8_t ep_out)
{
	int  size=0;
	uint8_t lun = 0;
	uint8_t expected_tag;
	long  max_lba, block_size;
	uint8_t cdb[16]={0};	// SCSI Command Descriptor Block
	uint8_t *buffer=NULL;
	long device_size=99;
	
	if ( !(buffer = (uint8_t *)kmalloc(sizeof(uint8_t)*64, GFP_KERNEL)) ) {
		printk(KERN_INFO "Error in allocating memory for rcv buffer\n");
		return -1;
	}
	
	cdb[0] = 0x25;			// Read Capacity command

	if(send_command_block(device, ep_out, lun, cdb, USB_ENDP_IN, READ_CAPACITY_LENGTH, &expected_tag) != 0){
		printk(KERN_INFO "Command Block sending error\n");
		return -1;
	}
	
	if(usb_bulk_msg(device, usb_rcvbulkpipe(device,ep_in), (unsigned char*)buffer, READ_CAPACITY_LENGTH, &size, 1000) !=0)
		{
		printk(KERN_INFO "Reading endpoint cdb error\n");
		return -1;
		}
	
	printk(KERN_INFO "received %d bytes\n", size);
	max_lba = be_to_int32(buffer);
	block_size = be_to_int32(buffer+4);
	device_size = ((max_lba+1))*block_size/(1024*1024*1024);
	printk(KERN_INFO "Pendrive size is: %ldGB\n", device_size);
	kfree(buffer);

	if(get_command_status(device, ep_in, expected_tag)==-1){
	printk(KERN_INFO "Command Status Block receiving error\n");
		return -1;
	}
	return 0;
}
/*************************************************************************************************************************/

/***************************************Get_Command_Status_Function*******************************************************/
int get_command_status(struct usb_device *device, uint8_t endpoint, uint8_t expected_tag)
{
	int i, r, size;
	struct command_status_wrapper *csw;

	if( !(csw = (struct command_status_wrapper*)kmalloc(sizeof(struct command_status_wrapper), GFP_KERNEL)) ) {
		printk(KERN_INFO "Error in memory allocation for csw\n");
		return -1;
	}

	i = 0;
	do {
		r = usb_bulk_msg(device, usb_rcvbulkpipe(device,endpoint), (unsigned char*)csw, 13, &size, 1000);
		if (r == LIBUSB_ERROR_PIPE) {
			usb_clear_halt(device, usb_sndbulkpipe(device,endpoint));
		}
		i++;
	} while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));

	printk(KERN_INFO "CSW received, status: %d\n", csw->bCSWStatus);
	kfree(csw);
	
	return r;
}
/***********************************************************************************************************************/

/********************************************Send_Command_Block_Function************************************************/
int send_command_block(struct usb_device *device, uint8_t endpoint, uint8_t lun,uint8_t *cdb, uint8_t direction, uint8_t data_length, uint8_t*ret_tag)
{
	 uint8_t tag = 100;
	 uint8_t cdb_len;
	 int i=0, r, size=0;
	 struct command_block_wrapper *cbw=NULL;

	cbw = (struct command_block_wrapper*)kmalloc(sizeof(struct command_block_wrapper), GFP_KERNEL);
	if ( cbw == NULL ) {
		printk(KERN_INFO "Error in memory allocation for cbw\n");
		return -1;
	}

	//check validity of command array
	if (cdb == NULL) {
		return -1;
	}

	//check validity of endpoint
	if (endpoint & USB_ENDP_IN) {
		printk(KERN_INFO "send_mass_storage_command: cannot send command on IN endpoint\n");
		return -1;
	}

	*ret_tag = tag;
	cdb_len = cdb_length[cdb[0]];
	printk(KERN_INFO "cdb_len: %d\n",cdb_len);

	memset(cbw, 0, sizeof(struct command_block_wrapper));
	cbw->dCBWSignature[0] = 'U';
	cbw->dCBWSignature[1] = 'S';
	cbw->dCBWSignature[2] = 'B';
	cbw->dCBWSignature[3] = 'C';
	cbw->dCBWTag = tag++;
	cbw->dCBWDataTransferLength = data_length;
	cbw->bmCBWFlags = direction;
	cbw->bCBWLUN = lun;
	// Subclass is 1 or 6 => cdb_len
	cbw->bCBWCBLength = cdb_len;
	memcpy(cbw->CBWCB, cdb, cdb_len);
	usb_reset_device(device); 

	i = 0;

	do {
		// The transfer length must always be exactly 31 bytes.
		r = usb_bulk_msg(device, usb_sndbulkpipe(device,endpoint), (unsigned char*)cbw, 31, &size, 1000);
		if (r == LIBUSB_ERROR_PIPE) {
			usb_clear_halt(device, usb_sndbulkpipe(device,endpoint));
		}
		i++;
	} while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));
		
	printk(KERN_INFO "Sending command return value = %d\n",r);
	if (r != SUCCESS) {
		printk(KERN_INFO "send_mass_storage_command error\n");
		return -1;
	}

	printk(KERN_INFO "sent successfully %d CDB bytes\n", cdb_len);
	kfree(cbw);
	return 0;
}
/**************************************************************************************************************/

/********************************************Probe_Function****************************************************/
static int usbdev_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int i;
	unsigned char epAddr, epAttr;
	uint8_t ep_in, ep_out; 	
	
	struct usb_endpoint_descriptor *ep_desc;

	struct usb_device *device;	

	device = interface_to_usbdev(interface);			//get device structure
	if ( device == NULL ) {
		printk(KERN_INFO "Device is not feteched\n");
		return -1;
	}
	
	ep_in = ep_out = 0;
	
	if(id->idVendor == SANDISK_PENDRIVE_VID)
	{
		printk(KERN_INFO "Sandisk Pendrive Plugged in\n");
	}

	else if(id->idVendor == HP_PENDRIVE_VID)
	{
		printk(KERN_INFO "HP Pendrive Plugged in\n");
	}

	else if(id->idVendor == KINGSTON_PENDRIVE_VID)
	{
		printk(KERN_INFO "Kingston Pendrive Plugged in\n");
	}

	else if(id->idVendor == MOSERBAER_PENDRIVE_VID)
	{
		printk(KERN_INFO "Moserbaer Pendrive Plugged in\n");
	}

	else
	{
		printk(KERN_INFO "Unknown device plugged in\n");
	}

	//if_desc = interface->cur_altsetting;
	printk(KERN_INFO "VID: %x\n", id->idVendor);
	printk(KERN_INFO "PID: %x\n", id->idProduct);
	printk(KERN_INFO "No. of Altsettings: %d\n", interface->num_altsetting);
	printk(KERN_INFO "No. of Endpoints: %d\n", interface->cur_altsetting->desc.bNumEndpoints);

	for(i = 0; i < interface->cur_altsetting->desc.bNumEndpoints;i++)
	{
		ep_desc = &interface->cur_altsetting->endpoint[i].desc; //get endpoint descriptor of current endpoint from interface descriptor
		epAddr = ep_desc->bEndpointAddress;	//fetching direction	
		epAttr = ep_desc->bmAttributes;		//fetching type of endpoint		
	
		if((epAttr & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)  //checking for endpoint descriptor
		{ 
			if(epAddr & 0x80)
			{
				if(!ep_in)
					ep_in = ep_desc->bEndpointAddress;
				printk(KERN_INFO "IN ENDPOINT address: %d\n",ep_in);
			}		
				
			else
			{
				if(!ep_out)
					ep_out = ep_desc->bEndpointAddress;
				printk(KERN_INFO "OUT ENDPOINT address: %d\n",ep_out);
			}		
		}
	}

	printk(KERN_INFO "USB DEVICE CLASS: %x", interface->cur_altsetting->desc.bInterfaceClass);
	printk(KERN_INFO "USB DEVICE SUB CLASS: %x", interface->cur_altsetting->desc.bInterfaceSubClass);
	printk(KERN_INFO "USB DEVICE PROTOCOL: %x", interface->cur_altsetting->desc.bInterfaceProtocol);

	if ( read_capacity(device,ep_in, ep_out) !=0 ) {
		printk(KERN_INFO "read capacity error\n");
		return -1;
	}	

	return 0;
}

/*operation structure*/
static struct usb_driver usbdev_driver = {
	name: "usbdev",				//name of the device
	probe: usbdev_probe,			//whenever device is plugged in
	disconnect: usbdev_disconnect,		//when we remove a device
	id_table: usbdev_table,			//list of devices served by this driver
};

int device_driver_init(void)
{
	printk(KERN_INFO "USB Capacity Read Driver Inserted\n");
	usb_register(&usbdev_driver);
	return 0;
}

void device_driver_exit(void)
{
	usb_deregister(&usbdev_driver);
	printk(KERN_NOTICE "USB Capacity Read Driver Removed\n");
}
module_init(device_driver_init);
module_exit(device_driver_exit);
MODULE_DESCRIPTION("USB module");
MODULE_AUTHOR("SIDDHARTH JHA <h20190535@goa.bits-pilani.ac.in>");
MODULE_LICENSE("GPL");





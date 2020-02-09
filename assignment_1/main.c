#include<linux/module.h>
#include<linux/init.h>
#include<linux/types.h>
#include<linux/kdev_t.h>        // For MAJOR and MINOR
#include<linux/fs.h>            // For "alloc_chrdev_region()"
#include<linux/cdev.h>			// For cdev
#include<linux/device.h>		// For udev related functions
#include<linux/uaccess.h> // For put_user and get_user macros
#include<linux/random.h>		// For get_random_bytes()
#include"ioctl.h"				// For custom ioctl commands that we have defined


#define SUCCESS 0
//#define DEVICE_NAME "adc8"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIDDHARTH JHA");
MODULE_DESCRIPTION("Character device driver for 10bit 8 channel ADC ");




/*************  Declaration of functions used *********************/
static int __init mychar_init(void);
static void __exit mychar_exit(void);
static ssize_t my_read(struct file *, char __user *, size_t, loff_t *);
static int my_open(struct inode *, struct file *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
static int my_close(struct inode*, struct file*);
/******************************************************************/

/*************************** global variables and data structures ***********************************/

/* file operation structure */
struct file_operations fops = {
	.owner= THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.release = my_close,
	.unlocked_ioctl = my_ioctl
};

struct cdev c_dev;														// my cdev structure for device
//dev_t device_num = 0;													// device number( major + minor )
dev_t dev_no;
struct class *cls;														// class structure pointer for device creation in userspace /dev directory
unsigned short adc_reading = 0;											// reading from the channel of ADC
int allignment = 1;														// Allignment of ADC data. 1 for storing bits in lower bits and 2 for storing in higher bits
int adc_channel = 0;													// ADC channel number

/*****************************************************************************************************/


/*********** setting module's init and exit functions **********/
module_init(mychar_init);
module_exit(mychar_exit);
/***************************************************************/


/******************************* Module initialization function *************************************/
static int __init mychar_init(void)
{
	printk(KERN_INFO "Driver inserted successfully\n");

	/* Allocating major number and minor number dynamically for our driver */
	if ( alloc_chrdev_region(&dev_no,0,1,"BITS_PILANI") !=0 ) {
		printk(KERN_ALERT "Cannot obtain major and minor num for device\n");
		return -1;
	}


	// creating udev device for userspace access //
	if ((cls = class_create(THIS_MODULE, "chardv")) == NULL) {
		printk(KERN_ERR "class create error\n");
		unregister_chrdev_region(dev_no, 1);
		return -1;
	}
	if (device_create(cls, NULL, dev_no, NULL, "8_ADC") == NULL) {
		printk(KERN_ERR "device_create error\n");
		class_destroy(cls);
		unregister_chrdev_region(dev_no, 1);
		return -1;
	}

	printk(KERN_INFO "Successfully created your device\n");


	// Initializing cdev structure //
	cdev_init( &c_dev, &fops);

	// Adding device to kernel devices list //
	if( cdev_add( &c_dev, dev_no, 1) == -1) {
		printk(KERN_ERR "Cannot add your device to the kernel\n");
		device_destroy(cls, dev_no);  
		class_destroy(cls);
		unregister_chrdev_region(dev_no, 1);
		return -1;
	}


	printk(KERN_INFO "MAJOR NUM: %d\n", MAJOR(dev_no) );
	printk(KERN_INFO "MINOR NUM: %d\n", MINOR(dev_no) );

	return SUCCESS;

}
/******************************************************************************************************/


/************************** File operation functions *************************************************/

// Open
static int my_open(struct inode *f_inode, struct file *f)
{
	printk(KERN_INFO " %s started !\n", __func__ );
	return 0;
}

// Read
ssize_t my_read(struct file *f, char __user *buff, size_t count, loff_t *offset)
{
	// Function to generate random number
	unsigned short random = 0;
	unsigned short align_num = 0;
	int ret;
	printk(KERN_INFO "%s started ! \n", __func__);

	get_random_bytes(&random, sizeof(unsigned short));
	random %= 1024;
	align_num = random << 6;
	switch(adc_channel) {

		case 0:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 1:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}

		case 2:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 3:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 4:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 5:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 6:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		case 7:
			if ( allignment == 1 ) {
				printk( KERN_INFO "Sending value: %hu\n", random);
				if(copy_to_user(buff,random,sizeof(random)))
				printk(KERN_INFO "some data is missing\n");
	
			}
			else {
				printk( KERN_INFO "Sending value: %hu\n", align_num);
				if(copy_to_user(buff,align_num,sizeof(align_num)))
				printk(KERN_INFO "some data is missing\n");
			}
			break;

		default:
			printk(KERN_ERR "Invalid channel selected\n");
	
	}

	return 4;
}

// Close
static int my_close(struct inode *i, struct file *fp)
{

	printk(KERN_INFO "Close function started\n");	
	return SUCCESS;
}

// Ioctl
static long my_ioctl(struct file *fp , unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO " %s started !\n", __func__ );
	switch(cmd) {
		case SET_CHANNEL:
				get_user(adc_channel, (int __user*)arg);
				printk(KERN_INFO "ADC channel set to %d\n", adc_channel);
				break;

		case SET_ALLIGNMENT:
				get_user(allignment, (int __user*)arg);
				printk(KERN_INFO "allignment set to %d\n", allignment);
				break;

		case GET_ALLIGNMENT:
				put_user(allignment, (int __user*)arg);
				printk(KERN_INFO "sent %d to userapp\n", allignment);
				break;
		
		case GET_CHANNEL:
				put_user(adc_channel, (int __user*)arg);
				printk(KERN_INFO "sent %d to userapp\n", adc_channel);
				break;
		default:
				printk(KERN_ERR "Invalid ioctl command\n");
				return -ENOTTY;
	
	}
	return SUCCESS;
}

/**********************************************************************************************************/



/***************************** Module Exit function ***************************************************/
static void __exit mychar_exit(void)
{
	/* removing our device from kernel */
	cdev_del( &c_dev);
	/* destroy device */
	device_destroy(cls, dev_no);
	/* destroying device class */
	class_destroy(cls);
	/* unregistering or giving back the major number and minor numbers to the kernel */
	unregister_chrdev_region(dev_no, 1);
	printk(KERN_INFO "Driver removed successfully\n");
}
/******************************************************************************************************/


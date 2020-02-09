#ifndef _IOCTL_H
#define _IOCTL_H

#include<linux/ioctl.h>
#define MAGIC_NUM 'K'
#define SET_CHANNEL _IOW(MAGIC_NUM,0,int)					// This command is to select the channel of the ADC
#define SET_ALLIGNMENT _IOW(MAGIC_NUM,1,int)					// This command is to select allignment of 10-bit data in 16-bit register 
#define GET_CHANNEL _IOR(MAGIC_NUM,2,int)
#define GET_ALLIGNMENT _IOR(MAGIC_NUM,4,int)

#endif



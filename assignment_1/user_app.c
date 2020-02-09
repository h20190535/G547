#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include"ioctl.h"

int main()
{
	char choice[8] = {'\0'};
	unsigned short data;
	int fd,channel,allignment;

	fd = open("/dev/8_ADC", O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		if( ioctl(fd,GET_ALLIGNMENT,&allignment) ) {
			perror("Ioctl userspace");
			exit(EXIT_FAILURE);
		}
//		printf("got %d from kernel space\n", allign);
		if( ioctl(fd,GET_CHANNEL,&channel) ) {
			perror("Ioctl userspace");
			exit(EXIT_FAILURE);
		}

		printf("press 0 to exit or 1 to continue");
		fflush(stdout);
		fgets(choice,8,stdin);

		if ( *choice == '0' && (strlen(choice) == 2) ) {
			printf("\nExiting program .....\n\n");
			break;
		}

		else{
			printf("Enter ADC channel number: ");
			scanf("%d", &channel);
			while( getchar() != '\n');
			if( ioctl(fd,SET_CHANNEL,&channel) ) {
				perror("ioctl");
				exit(EXIT_FAILURE);
			}
	
			printf("Enter ADC data allignment ( 1 for lower bytes and 2 for higher bytes ): ");
			scanf("%d", &allignment);
			while( getchar() != '\n');
			if( ioctl(fd,SET_CHANNEL,&allignment) ) {
				perror("ioctl");
				exit(EXIT_FAILURE);	
			}

			read(fd,&data,2);
			printf("data: %d\n", data);
			printf("Current ADC channel set to: \"%d\"\n", channel);
			printf(" Allignment set to: \"%d\"\n", allignment);
		}
	}
		
	return 0;
}

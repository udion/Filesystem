#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "emufs.h"

#define EMULATED_DISK_SIZE 6

char smalltext[] = "START-This is a small text consisting 50 bytes-END";
char largetext[] = "START--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a large text consisting 700 bytes--This is a  large text consisting 700 bytes--This is a large text  consisting 700 bytes--This is a large text consisting 700 bytes--END";
char veryLargetext[] = "START---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes----This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes---This is a very large text consisting 1710 bytes-This is a very large text consisting 1710 bytes---END";
int main(int argc, char *argv[])
{
	int ret;
	char *readBuf;
	if(argc < 2)
	{
		printf("Error: <DISK-NAME> \n");
		exit(1);
	}

	printf("Disk name : %s \n",argv[1]);
	ret=opendevice(argv[1], EMULATED_DISK_SIZE);
	if(ret < 0)
	{
		// Error
		exit(1);
	}
	else if (ret == 1)
	{	
		// File system not found
		create_file_system();
	}
	else if (ret == 2)// File system not found
	{	
		// File system found
	}

	printf("\n");
	fsdump();


	struct file_t *f1 = eopen("file1");
	struct file_t *f2 = eopen("file2");

	
	printf("Writing %ld bytes to file1, offset: %d.\n",strlen(smalltext),f1->offset); 
	ewrite(f1, smalltext, strlen(smalltext));
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	
	printf("Writing %ld bytes to file2, offset: %d.\n",strlen(largetext),f2->offset); 
	ewrite(f2, largetext, strlen(largetext));
	printf("After writing, file2 offset : %d\n\n",  f2->offset);


	printf("Writing %ld bytes to file1, offset: %d.\n",strlen(smalltext),f1->offset); 
	ewrite(f1, smalltext, strlen(smalltext));
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	fsdump();

	printf("Writing %ld bytes to file1, offset: %d.\n",strlen(veryLargetext),f1->offset); 
	ewrite(f1, veryLargetext, strlen(veryLargetext));
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	printf("Truncating all blocks from file2. current offset: %d\n",f2->offset);
	ret=etruncate(f2,10);
	printf("%d blocks truncated from file2, offset: %d\n\n",ret, f2->offset);


	printf("Writing %ld bytes to file1, offset: %d.\n",strlen(veryLargetext),f1->offset); 
	ewrite(f1, veryLargetext, strlen(veryLargetext));
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	fsdump();

	if (closedevice()<0)
		exit(1);
	printf("Device Closed \n");

	return 0;
}
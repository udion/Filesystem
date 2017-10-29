#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "emufs.h"

#define EMULATED_DISK_SIZE 42

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

	printf("Writing %ld bytes to file2, offset: %d.\n",strlen(veryLargetext),f2->offset); 
	ewrite(f2, veryLargetext, strlen(veryLargetext));
	printf("After writing, file2 offset : %d\n\n",  f2->offset);

	printf("Writing %ld bytes to file1, offset: %d.\n",strlen(veryLargetext),f1->offset); 
	ewrite(f1, veryLargetext, strlen(veryLargetext));
	printf("After writing, file1 offset : %d\n\n",  f1->offset);

	printf("Writing %ld bytes to file2, offset: %d.\n",strlen(largetext),f2->offset); 
	ewrite(f2, largetext, strlen(largetext));
	printf("After writing, file2 offset : %d\n\n",  f2->offset);

	fsdump();
	eclose(f1);
	eclose(f2);
	if(closedevice())
		printf("Device Closed\n");
	else
		exit(1);

	printf("\n...................Reopening device................ \n"); 
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

	printf("Here \n");

	printf("\n");
	fsdump();

	struct file_t *fileA = eopen("file1");
	struct file_t *fileB = eopen("file2");

	printf("Reading %ld bytes from file1, offset: %d.\n",strlen(smalltext),fileA->offset); 
	readBuf=(char *)malloc(strlen(smalltext)*sizeof(char)+1);
	readBuf[0]='\0';
	eread(fileA,readBuf,strlen(smalltext));
	readBuf[strlen(smalltext)]='\0';
	printf("Data = [%s] \n",readBuf);
	printf("After reading, file1 offset : %d\n\n",  fileA->offset);
	free(readBuf);

	printf("Reading %ld bytes from file1, offset: %d.\n",strlen(veryLargetext),fileA->offset); 
	readBuf=(char *)malloc(strlen(veryLargetext)*sizeof(char)+1);
	readBuf[0]='\0';
	eread(fileA,readBuf,strlen(veryLargetext));
	readBuf[strlen(veryLargetext)]='\0';
	printf("Data = [%s] \n",readBuf);
	printf("After reading, file1 offset : %d\n\n",  fileA->offset);
	free(readBuf);

	fsdump();
	eclose(fileA);
	eclose(fileB);

	printf("Reached A\n");
	if(closedevice())
		printf("Device Closed\n");
	
	return 0;
}
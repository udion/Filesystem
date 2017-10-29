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

char smalltext[] = "This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--This is a small text consisting 512 bytes--------------------------------------END";
char largetext[] = "This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---This is a large text consisting 1024 bytes---Reached the end of 1024 bytes--END";
char veryLargetext[] = "This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes----This is a very large text consisting of 1536 bytes-----Reached the end-----END";

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



	struct file_t *fileWrite = eopen("file1");
	struct file_t *fileRead = eopen("file1");

	//	Writes using 'fileWrite' file handler
	printf("Writing %ld bytes to file1, write-offset: %d.\n",strlen(smalltext),fileWrite->offset); 
	ewrite(fileWrite, smalltext, strlen(smalltext));
	printf("After writing,  write-offset : %d\n\n",  fileWrite->offset);
	
	printf("Writing %ld bytes to file1, write-offset: %d.\n",strlen(largetext),fileWrite->offset); 
	ewrite(fileWrite, largetext, strlen(largetext));
	printf("After writing,  write-offset : %d\n\n",  fileWrite->offset);

	fsdump();


	// Read the data using another file hanlder
	printf("Reading %ld bytes from file1, read-offset: %d.\n",strlen(smalltext),fileRead->offset); 
	readBuf=(char *)malloc(strlen(smalltext)*sizeof(char)+1);
	readBuf[0]='\0';
	eread(fileRead,readBuf,strlen(smalltext));
	readBuf[strlen(smalltext)]='\0';
	printf("Data = [%s] \n",readBuf);
	printf("After reading, read-offset: %d\n\n",  fileRead->offset);
	free(readBuf);

	// Read the data using another file hanlder
	printf("Reading %ld bytes from file1, read-offset: %d.\n",strlen(largetext),fileRead->offset); 
	readBuf=(char *)malloc(strlen(largetext)*sizeof(char)+1);
	readBuf[0]='\0';
	eread(fileRead,readBuf,strlen(largetext));
	readBuf[strlen(largetext)]='\0';
	printf("Data = [%s] \n",readBuf);
	printf("After reading, read-offset: %d\n\n",  fileRead->offset);
	free(readBuf);

	fsdump();

	if (closedevice()<0)
		exit(1);
	printf("Device Closed \n");

	return 0;
}
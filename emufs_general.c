#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "emufs.h"
#include <time.h>

/*
	----------------DEVICE------------------
*/
int EMULATED_DISK_FD=-1;

int opendevice(char *device_name, int size)
{

	/*
		* It opens the the emulated deive.
		* If emulated device file is not exit
			* Create emulated disk file.
			* Creates superblocks and writes to disk.
			* file system type on newly created superblock will be null or '0'
		* It returns 2, if a file sytem exits on the deivce. Other wise return 1.
		* In case of any error, it returns -1.
	*/

	if(size > MAX_BLOCKS || size<2)
	{
		printf("Invalid Disk Size\n");
		return -1;
	}

	FILE *fp;
	int fd;
	fp = fopen(device_name, "r");
	struct superblock_t *superblock;
	
	if(!fp)
	{
		//	Creating the device
		printf("Creating disk. \n");
		
		superblock = (struct superblock_t*)malloc(sizeof(struct superblock_t));
		strcpy(superblock->name,device_name);
		strcpy(superblock->fstype,"0");
		superblock->disk_size=size;

		fp = fopen(device_name, "w+");
		if(!fp)
		{
			printf("Error : Unable to create device. \n");
			free(superblock);
			return -1;
		}
		fd = fileno(fp);
		
		// Make size of the disk as the total size
        fseek(fp, size*BLOCKSIZE, SEEK_SET);
        fputc('\0', fp);

		fseek(fp, 0, SEEK_SET);
		char tempBuf[BLOCKSIZE];
		memcpy(tempBuf, superblock, sizeof(struct superblock_t));
		write(fd, tempBuf, BLOCKSIZE);

		printf("New disk created\n");
		EMULATED_DISK_FD = fd;	// Set the current device as the file descriptor of the disk file
		free(superblock);
		return 1;		// File system does not exist.

	}
	else
	{
		fclose(fp);
		fd = open(device_name, O_RDWR);
		printf("Disk exists. \n");
		EMULATED_DISK_FD = fd;

		char tempBuf[BLOCKSIZE];
		superblock = (struct superblock_t*)malloc(sizeof(struct superblock_t));

		read(EMULATED_DISK_FD, tempBuf,BLOCKSIZE);
		memcpy(superblock,tempBuf,sizeof(struct superblock_t));
		if(strcmp(superblock->fstype,"0")==0)
		{
			// File system does not exist.
			printf("File system not found on this disk \n");
			free(superblock);
			return 1;
		}
		else
		{
			printf("File system on the disk : %s \n",superblock->fstype);
			free(superblock);
			return 2;
		}
	}
	
	
}

int writedevice(int block, char * buf)
{
	
	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	int offset=block * BLOCKSIZE;
	lseek(EMULATED_DISK_FD, offset, SEEK_SET);
	int ret=write(EMULATED_DISK_FD, buf, BLOCKSIZE);
	if(ret!=BLOCKSIZE)
	{
		printf("Error: Disk write error. \n");
		return -1;
	}
	return 1;
}

int readdevice(int block, char * buf)
{
	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	int offset=block * BLOCKSIZE;
	lseek(EMULATED_DISK_FD, offset, SEEK_SET);
	int ret=read(EMULATED_DISK_FD, buf, BLOCKSIZE);
	if(ret!=BLOCKSIZE)
	{
		printf("Error: Disk read error. \n");
		return -1;
	}
	return 1;
}

int closedevice()
{
	if(EMULATED_DISK_FD < 0)
	{
		printf("No Device is open\n");
		return -1;
	}
	close(EMULATED_DISK_FD);
	EMULATED_DISK_FD = -1;

	return 1;
}

void fsdump()
{
	char buff[BLOCKSIZE];
	readdevice(1, buff);
	struct metadata_t *md = (struct metadata_t*)malloc(sizeof(struct metadata_t));
	memcpy(md, buff, sizeof(struct metadata_t));

	printf("NAME 	SIZE 	[BLOCKS] 	LAST MODIFIED\n");
	for(int i=0; i<MAX_FILES; i++){
		struct inode_t *n = &md->inodes[i]; 
		if(n->status == USED){
			printf("%s 	%d 	[%d, %d, %d, %d] 	%s", n->name, n->file_size, n->blocks[0], n->blocks[1],
			 n->blocks[2], n->blocks[3], asctime(localtime(&n->modtime)));
		}
	}
	printf("\n\n");
}


/*
	----------------HELPER FUNCTIONS------------------
*/

struct superblock_t* readSuperblock()
{
	/*
		* Read 0th block from the device into a blocksize buffer
		* Create superblock_t variable and fill it using reader buffer
		* Return the superblock_t variable
	*/
	char buff[BLOCKSIZE];
	readdevice(0, buff);
	struct superblock_t *sb = (struct superblock_t*)malloc(sizeof(struct superblock_t));
	memcpy(sb, buff, sizeof(struct superblock_t));
	return sb;
}

int writeSuperblock(struct superblock_t *superblock)
{
	/*
		* Read the 0th block from device into a buffer
		* Write the superblock into the buffer
		* Write back the buffer into block 0
	*/
	char buff[BLOCKSIZE];
	readdevice(0, buff);
	memcpy(buff, superblock, sizeof(struct superblock_t));
	writedevice(0, buff);
}

struct metadata_t* readMetadata()
{
	// Same as readSuperBlock(), but it is stored on block 1
	char buff[BLOCKSIZE];
	readdevice(1, buff);
	struct metadata_t *md = (struct metadata_t*)malloc(sizeof(struct metadata_t));
	memcpy(md, buff, sizeof(struct metadata_t));
	return md;
}

int writeMetadata(struct metadata_t *metadata)
{
	// Same as writeSuperblock(), but it is stored on block 1
	char buff[BLOCKSIZE];
	readdevice(1, buff);
	memcpy(buff, metadata, sizeof(struct metadata_t));
	writedevice(1, buff);
}

void writedbg(int block, int offset, int blockoffset, int remaining){
	printf("dbg: in ewrite, block=%d, infile offset=%d, block offset=%d, remaining=%d\n"
		, block, offset, blockoffset, remaining);
}

/*
	----------------FILE SYSTEM API------------------
*/

int create_file_system()
{
	/*
	   	* Read the superblock.
	    * Set file system type on superblock as 'emufs'
		* Clear the bitmaps.  values on the bitmap will be either '0', or '1', or'x'. 
		* Create metadata block in disk
		* Write superblock and metadata block back to disk.
	*/
	struct superblock_t *sb = readSuperblock();
	strcpy(sb->fstype,"emufs");
	for(int i=0; i<MAX_BLOCKS; i++){
		if(i==0 || i==1){
			sb->bitmap[i] = '1';
		}
		else if(i > sb->disk_size){
			sb->bitmap[i] = 'x';
		}
		else{
			sb->bitmap[i] = '0';
		}
	}

	struct metadata_t *md = (struct metadata_t *)malloc(sizeof(struct metadata_t));
	//filling it up
	for(int i=0; i<MAX_FILES; i++){
		md->inodes[i].status = UNUSED;
		strcpy(md->inodes[i].name, "");
		md->inodes[i].file_size = 0;
		md->inodes[i].blocks[0] = -1;
		md->inodes[i].blocks[1] = -1;
		md->inodes[i].blocks[2] = -1;
		md->inodes[i].blocks[3] = -1;
		time(&md->inodes[i].modtime);
	}
	writeSuperblock(sb);
	writeMetadata(md);
}

struct file_t* eopen(char * filename)
{
	/* 
		* If file exist, get the inode number. inode number is the index of inode in the metadata.
		* If file does not exist, 
			* find free inode.
			* allocate the free inode as USED
			* if free id not found, print the error and return -1
		* Create the file hander (struct file_t)
		* Initialize offset in the file hander
		* Return file handler.
	*/
	struct file_t* f = (struct file_t*)malloc(sizeof(struct file_t));
	struct metadata_t *md = readMetadata();
	struct superblock_t *sb = readSuperblock();
	
	for(int i=0; i<MAX_FILES; i++){
		if(strcmp(md->inodes[i].name, filename) == 0){
			printf("dbg: Got the file inode at %d\n", i);
			f->inode_number = i;
			f->offset = 0;
			return f;
		}
	}

	//traverse to find free inode space
	for(int i=0; i<MAX_FILES; i++){
		if(md->inodes[i].status == UNUSED){
			//found the enmpty inode
			md->inodes[i].status = USED;
			strcpy(md->inodes[i].name, filename);
			md->inodes[i].file_size = 0*BLOCKSIZE;
			time(&md->inodes[i].modtime);
			writeMetadata(md);
			
			f->inode_number = i;
			f->offset = 0;
			printf("file open file_inode=%d\n", i);
			return f;
		}
	}

	//no free inodes
	free(f);
	printf("no free inodes!!\n");
	return NULL;
}

int ewrite(struct file_t* file, char* data, int size)
{
	// You do not need to implement partial writes in case file exceeds 4 blocks
	// or no free block is available in the disk.
	struct metadata_t *md = readMetadata();
	struct superblock_t *sb = readSuperblock();
	int mynode = file->inode_number;
	int offset = file->offset;
	//gathering data to be used for processing
	int start_block = offset/BLOCKSIZE; //0,1,2,3
	int start_block_left = (start_block+1)*BLOCKSIZE - offset;
	int block_offset = offset%BLOCKSIZE;
	//the free space will always be at the end of the block in the final block
	//overwrites are allowed from the current offset
	int remaining = size;
	if((offset + size) > MAX_FILE_SIZE*BLOCKSIZE){
		printf("ERROR: out of bound write!\n");
		return -1;

	}
	//figuring out all the blocks which will be affected by the writes
	//start_block ... to ... last_block
	int last_block = (offset+size-1)/BLOCKSIZE;//0,1,2,3
	//assign blocks if not already present
	//because of eseek handling bounds, uptill startblock-1 all asiigned
	for(int i=start_block; i<=last_block; i++){
		if(md->inodes[mynode].blocks[i] == -1){
			for(int j=0; j<sb->disk_size; j++){
				if(sb->bitmap[j] == '0'){
					sb->bitmap[j] = '1';
					writeSuperblock(sb);
					md->inodes[mynode].blocks[i] = j;
					time(&md->inodes[mynode].modtime);
					writeMetadata(md);
					break;
				}
			}
		}
		continue;
	}
	//now I can start writing in the blocks, and it will be in a continuous
	//manner from start_block to last_block, 
	//last_block need not be filled completely
	int writedatafrom = 0;
	for(int i=start_block; i<=last_block; i++){
		if(i == start_block){
			if(remaining <= start_block_left){
				//writedbg(i, offset, block_offset, remaining);
				char subbuff[remaining];
				memcpy(subbuff, &data[writedatafrom], remaining);
				//modify the current block
				char curr_block_data[BLOCKSIZE];
				readdevice(md->inodes[mynode].blocks[i], curr_block_data);
				memcpy(&curr_block_data[block_offset], subbuff, remaining);
				writedevice(md->inodes[mynode].blocks[i], curr_block_data);
				writedatafrom += remaining;
				offset += remaining-1;
				remaining = 0;
				block_offset = offset%BLOCKSIZE;
			}
			else{
				//writedbg(i, offset, block_offset, remaining);
				//printf("start blk left %d\n", start_block_left);
				char subbuff[start_block_left];
				memcpy(subbuff, &data[writedatafrom], start_block_left);
				//modify the current block
				char curr_block_data[BLOCKSIZE];
				readdevice(md->inodes[mynode].blocks[i], curr_block_data);
				memcpy(&curr_block_data[block_offset], subbuff, start_block_left);
				writedevice(md->inodes[mynode].blocks[i], curr_block_data);
				writedatafrom += start_block_left;
				offset += start_block_left;
				remaining -= start_block_left;
				block_offset = offset%BLOCKSIZE;
			}
		}
		else if(i == last_block){
			if(remaining == 0){
				md->inodes[mynode].file_size += size;
				writeMetadata(md);
				file->offset = offset;
				return size;
			}
			else{
				//writedbg(i, offset, block_offset, remaining);
				char subbuff[remaining];
				memcpy(subbuff, &data[writedatafrom], remaining);
				//modify the current block
				char curr_block_data[BLOCKSIZE];
				readdevice(md->inodes[mynode].blocks[i], curr_block_data);
				memcpy(&curr_block_data[block_offset], subbuff, remaining);
				writedevice(md->inodes[mynode].blocks[i], curr_block_data);
				writedatafrom += remaining;
				offset += remaining-1;
				remaining = 0;
				block_offset = offset%BLOCKSIZE;
			}
		}
		else{
			if(remaining == 0){
				md->inodes[mynode].file_size += size;
				writeMetadata(md);
				file->offset = offset;
				return size;
			}
			else{
				//writedbg(i, offset, block_offset, remaining);
				char subbuff[BLOCKSIZE];
				memcpy(subbuff, &data[writedatafrom], BLOCKSIZE);
				//modify the current block
				char curr_block_data[BLOCKSIZE];
				readdevice(md->inodes[mynode].blocks[i], curr_block_data);
				memcpy(&curr_block_data[block_offset], subbuff, BLOCKSIZE);
				writedevice(md->inodes[mynode].blocks[i], curr_block_data);
				writedatafrom += BLOCKSIZE;
				offset += BLOCKSIZE;
				remaining = 0;
				block_offset = offset%BLOCKSIZE;
			}
		}

		if(remaining==0){
			//done writing
			//printf("dbg inside remaining 0\n");
			md->inodes[mynode].file_size += size;
			writeMetadata(md);
			file->offset = offset;
			return size;
		}
	}
}

int eread(struct file_t* file, char* data, int size)
{
	struct metadata_t *md = readMetadata();
	int mynode = file->inode_number;
	int offset = file->offset;
	//gathering data to be used for processing
	int start_block = offset/BLOCKSIZE;
	int start_block_left = (start_block+1)*BLOCKSIZE - offset;
	int block_offset = offset%BLOCKSIZE;
	
	//let's see how much this creature wants to read
	if((offset+size) > MAX_FILE_SIZE*BLOCKSIZE){
		printf("ERROR: out of bound reading\n");
		return -1;
	}
	//TODO error handling
	//printf("dbg in eread!\n");
	// int blocknum_int = size/BLOCKSIZE;
	// int blocknum_frac = size%BLOCKSIZE;
	int remaining = size;
	//printf("file_inode idx=%d, num of blocks to read=%d\n", mynode, endblocknum);
	for(int i=start_block; i<MAX_FILE_SIZE; i++){
		char temp[BLOCKSIZE];
		readdevice(md->inodes[mynode].blocks[i], temp);

		if(i == start_block){
			//printf("dbg remaining read=%d, start_block_left=%d\n", remaining, start_block_left);
			if(remaining <= start_block_left){
				char dtemp[remaining];
				memcpy(dtemp, &temp[block_offset], remaining);
				strcat(data, dtemp);
				offset += remaining;
				block_offset = offset%BLOCKSIZE;
				remaining = 0;
			}
			else{
				char dtemp[start_block_left];
				memcpy(dtemp, &temp[block_offset], start_block_left);
				strcat(data, dtemp);
				offset += start_block_left;
				block_offset = offset%BLOCKSIZE;
				remaining -= start_block_left;
			}
		}
		if(remaining){
			if(remaining <= BLOCKSIZE-block_offset){
				char dtemp[remaining];
				memcpy(dtemp, &temp[block_offset], remaining);
				strcat(data, dtemp);
				offset += remaining;
				block_offset = offset%BLOCKSIZE;
				remaining = 0;
			}
			else{
				char dtemp[BLOCKSIZE-block_offset];
				memcpy(dtemp, &temp[block_offset], BLOCKSIZE-block_offset);
				strcat(data, dtemp);
				remaining -= BLOCKSIZE-block_offset;
				offset += BLOCKSIZE-block_offset;
				block_offset = offset%BLOCKSIZE;
			}
		}
		else{
			return size;
		}
	}
}

void eclose(struct file_t* file)
{
	// free the memory allocated for the file handler 'file'
	struct metadata_t *md = readMetadata();
	// md->inodes[file->inode_number].status=UNUSED;
	// strcpy(md->inodes[file->inode_number].name, "");
	// md->inodes[file->inode_number].file_size = 0;
	// md->inodes[file->inode_number].blocks[0] = -1;
	// md->inodes[file->inode_number].blocks[1] = -1;
	// md->inodes[file->inode_number].blocks[2] = -1;
	// md->inodes[file->inode_number].blocks[3] = -1;
	time(&md->inodes[file->inode_number].modtime);
	writeMetadata(md);
	free(file);
}

int eseek(struct file_t *file, int offset)
{
	// Chnage the offset in file hanlder 'file'
	struct metadata_t *md = readMetadata();
	int fsz = md->inodes[file->inode_number].file_size;
	if(fsz){
		if(offset > 3*BLOCKSIZE || offset>=fsz){
			printf("ERROR: seeking out of bounds!\n");
			return -1;
		}
		file->offset = offset;
	}
	else{
		if(offset != 0){
			printf("ERROR: file size zero, seeking out of bounds!\n");
		return -1;
		}
		file->offset = offset;
	}
	return 0;
}

int etruncate(struct file_t *file, int n)
{
	/*
		* Truncate last n blocks allocated a file referred by the file hanlder 'file'
		* If n is greater than the blocks allocated, remove all blocks.
		* It should return the actual number of blocks truncated.

		* Read the metadata & superblock from disk.
		* Remove the last n blocks allocated in inode of a file
		* Add the those truncated blocks into free list(Update bitmap)
		* Write metadata and superblock back to disk
	*/
	struct metadata_t *md = readMetadata();
	int mynode = file->inode_number;
	int removed = 0;
	for(int i=1; i<=MAX_FILE_SIZE; i++){
		if(md->inodes[mynode].blocks[MAX_FILE_SIZE-i] == -1){
			continue;
		}
		else{
			if(removed < n){
				md->inodes[mynode].blocks[MAX_FILE_SIZE-i] = -1;
				md->inodes[mynode].file_size = (MAX_FILE_SIZE-i)*BLOCKSIZE;
				file->offset = (MAX_FILE_SIZE-i)*BLOCKSIZE-1;
				if(i == 4)
					file->offset = 0;
				writeMetadata(md);
				removed++;
			}
			else{
				break;
			}
		}
	}
	return removed;//COMPLETE IT
}
#include <stdio.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define RESEVED_SECTOR_NUM (2)

int main(int argc, char**argv){
  
    if (argc <3) {
	printf("argv error\n");
	printf("cmd <deveice> <imagefile>\n");
	return -1;
    }

    unsigned long blknum = 0;
    int bd = open(argv[1],O_RDWR);
    if (bd < 0) {
	printf("open device failed : %s\n",strerror(errno));
	return -1;
    }

    ioctl(bd,BLKGETSIZE,&blknum);
    printf("device %s block num is %lu\n",argv[1],blknum);

    int imagefile = open (argv[2],O_RDONLY);
    if (imagefile < 0) {
	printf("open image file failed : %s\n",strerror(errno));
	return -1;
    }

    struct stat s;
    fstat(imagefile, &s);
    printf("file size is : %d\n", s.st_size);
    printf("file block num is: %d\n",s.st_blocks);
    char *buffer = (char *)malloc(s.st_size);
    if (buffer == NULL) {
	printf("alloc memory failed");
	close(imagefile);
	return -1;
    }
    read(imagefile, buffer, s.st_size);
    close(imagefile);

    printf("writing to sdcard\n");

    unsigned long startsector = blknum - RESEVED_SECTOR_NUM - s.st_blocks;
    lseek(bd,startsector*512,SEEK_SET);

    ssize_t wsize = write(bd, buffer, s.st_size);
    if(wsize!=s.st_size){
	printf("write only %u",wsize);
    }
    close(bd);
    free(buffer);
    return 0;
}

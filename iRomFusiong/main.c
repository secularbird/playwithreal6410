/*********************************************************************
 * Filename:      main.c
 *                
 * Copyright (C) 2011,  yaozh
 * Version:       
 * Author:        zhuangyao <secularbird.eagle@gmail.com>
 * Created at:    Mon Jul  4 09:46:36 2011
 *                
 * Description:   
 *                
 ********************************************************************/
#define _LARGEFILE64_SOURCE
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

#ifndef _FOR_HCSD_
#define RESEVED_SECTOR_NUM (2)
#else
#define RESEVED_SECTOR_NUM (1026)
#endif

int main(int argc, char**argv){
  
    if (argc <3) {
	printf("argv error\n");
	printf("cmd <deveice> <imagefile>\n");
	return -1;
    }

    unsigned long blknum = 0;
    int bd = open(argv[1],O_RDWR|O_LARGEFILE);
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
    printf("file size is : %ld\n", s.st_size);
    printf("file block num is: %ld\n",s.st_blocks);
    char *buffer = (char *)malloc(s.st_size);
    if (buffer == NULL) {
	printf("alloc memory failed");
	close(imagefile);
	return -1;
    }
    read(imagefile, buffer, s.st_size);
    close(imagefile);

    printf("writing to sdcard\n");

    off64_t startsector = blknum - RESEVED_SECTOR_NUM - s.st_blocks;
    printf("start sector is %lld\n address is %llx\n", startsector, startsector*512);
    
    off64_t offset;
    offset = lseek64(bd,startsector*512,SEEK_SET);

    if(offset != startsector*512){
	printf("out of range %lld\n",offset);
	return -1;
    }

    ssize_t wsize = write(bd, buffer, s.st_size);
    if(wsize!=s.st_size){
	printf("write only %u",wsize);
    }
    printf("end sector is %lld\n address is %llx\n", 
	   startsector+wsize/512, startsector*512+wsize);
    close(bd);
    free(buffer);
    return 0;
}

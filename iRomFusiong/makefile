######################################################################
## Filename:      makefile
##                
## Copyright (C) 2011,  yaozh
## Version:       
## Author:        zhuangyao <secularbird.eagle@gmail.com>
## Created at:    Mon Jul  4 09:48:19 2011
##                
## Description:   
##                
######################################################################


TARGETNAME1:=irom

TARGETNAME2:=iromhc

CXXFLAGS := -m32 -g

all: $(TARGETNAME1) $(TARGETNAME2)

$(TARGETNAME1):main.c
	gcc $(CXXFLAGS) main.c -o $@

$(TARGETNAME2):main.c
	gcc $(CXXFLAGS) -D_FOR_HCSD_ main.c -o $@

.PHONY:clean
clean:
	rm $(TARGETNAME1)
	rm $(TARGETNAME2)


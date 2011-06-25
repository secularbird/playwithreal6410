#
# secbulk - Samsung USB bulk-transfer Linux kernel driver
#
# Copyright (C) 2011 Michel Stempin <michel.stempin@wanadoo.fr>
# Heavily based on existing GPL source code that can be found in
# various Google Code projects and on several Chineese blog sites.
#
# $Id: Makefile 214 2011-03-09 23:36:14Z michel.stempin@wanadoo.fr $
#

KERNEL_VERSION	:= `uname -r`
KERNEL_DIR	:= /lib/modules/$(KERNEL_VERSION)/build
INSTALL_MOD_DIR	:= kernel/drivers/usb/misc

DIST_FILES := AUTHORS COPYING COPYRIGHT INSTALL Makefile NEWS README secbulk.c
DIST_VERSION := `grep MODULE_VERSION secbulk.c | cut -d '"' -f 2`

PWD		:= $(shell pwd)

obj-m := secbulk.o
secbulk-m := secbulk.o

all: secbulk

secbulk:
	@echo "Building Samsung USB bulk transfer driver..."
	@(cd $(KERNEL_DIR) && make -C $(KERNEL_DIR) SUBDIRS=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) modules)

install:
	@echo "Installing Samsung USB bulk transfer driver..."
	@(cd $(KERNEL_DIR) && make -C $(KERNEL_DIR) SUBDIRS=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install)

clean:
	@echo "Cleaning Samsung USB bulk transfer driver..."
	@-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order
	@-rm -rf .tmp_versions

dist:
	@echo "Creating Samsung USB bulk transfer driver distribution package..."
	@-rm -rf secbulk-$(DIST_VERSION)
	@mkdir secbulk-$(DIST_VERSION)
	@ cp -a $(DIST_FILES) secbulk-$(DIST_VERSION)/
	@tar -zcf secbulk-$(DIST_VERSION).tar.gz secbulk-$(DIST_VERSION)
	@-rm -rf secbulk-$(DIST_VERSION)

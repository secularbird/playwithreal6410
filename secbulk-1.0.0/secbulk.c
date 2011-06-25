/*
 * secbulk.c -- Samsung USB bulk-transfer device
 *
 * Copyright (C) 2011 Michel Stempin <michel.stempin@wanadoo.fr>
 * All rights reserved.
 *
 * $Id: secbulk.c 214 2011-03-09 23:36:14Z michel.stempin@wanadoo.fr $
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define SECBULK_MAJOR	102
#define SECBULK_MINOR	0
#define DRIVER_NAME	"secbulk"


MODULE_DESCRIPTION("Samsung USB bulk-transfer device");
MODULE_AUTHOR("Michel Stempin <michel.stempin@wanadoo.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

#define BULKOUT_BUFFER_SIZE	512

struct secbulk_dev
{
	struct usb_device *udev;
	struct mutex io_mutex;
	char *bulkout_buffer;
	__u8 bulk_out_endpointAddr;
};

static struct usb_class_driver secbulk_class;

static struct usb_device_id secbulk_table[]= {
	{USB_DEVICE(0x04E8, 0x1234)},
	{}
};

module_param_named(vendor, secbulk_table[0].idVendor, ushort, S_IRUGO);
MODULE_PARM_DESC(vendor, " USB Vendor ID");

module_param_named(product, secbulk_table[0].idProduct, ushort, S_IRUGO);
MODULE_PARM_DESC(product, " USB Product ID");

static struct usb_driver secbulk_driver;
static void secbulk_disconnect(struct usb_interface *interface)
{
	struct secbulk_dev *dev = NULL;

	printk(KERN_INFO "secbulk: device %04X:%04X disconnected!\n", secbulk_table[0].idVendor, secbulk_table[0].idProduct);
	dev = usb_get_intfdata(interface);
	if (dev != NULL) {
		kfree(dev);
  }
	usb_deregister_dev(interface, &secbulk_class);
	return;
}

static ssize_t secbulk_read(struct file *file, char __user *buf, size_t len, loff_t *loff)
{
	return -EPERM;
}

static ssize_t secbulk_write(struct file *file, const char __user *buf, size_t len, loff_t *loff)
{
	size_t to_write;
	struct secbulk_dev *dev = file->private_data;
	int ret;
	int actual_length;
	size_t total_writed;

	total_writed = 0;
	while (len > 0) {
		to_write = min(len, (size_t) BULKOUT_BUFFER_SIZE);
    if (copy_from_user(dev->bulkout_buffer, buf+total_writed, to_write)) {
			printk(KERN_ERR "secbulk: copy_from_user failed!\n");
			return -EFAULT;
		}
		ret = usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
				dev->bulkout_buffer,
				to_write,
				&actual_length,
				3 * HZ);
		if (ret || actual_length != to_write) {
			printk(KERN_ERR "secbulk: cannot write bulk message to device %04X:%04X: %d\n", secbulk_table[0].idVendor, secbulk_table[0].idProduct, ret);
			return -EFAULT;
		}
		len -= to_write;
		total_writed += to_write;
	}
	return total_writed;
}

static int secbulk_open(struct inode *node, struct file *file)
{
	struct usb_interface *interface;
	struct secbulk_dev *dev;

	interface = usb_find_interface(&secbulk_driver, iminor(node));
	if (!interface) {
		return -ENODEV;
  }
	dev = usb_get_intfdata(interface);
	dev->bulkout_buffer = kzalloc(BULKOUT_BUFFER_SIZE, GFP_KERNEL);
	if (!(dev->bulkout_buffer)) {
		return -ENOMEM;
  }
	if (!mutex_trylock(&dev->io_mutex)) {
		return -EBUSY;
  }
	file->private_data = dev;
	return 0;
}

static int secbulk_release(struct inode *node, struct file *file)
{
	struct secbulk_dev *dev;

	dev = (struct secbulk_dev*) (file->private_data);
	kfree(dev->bulkout_buffer);
	mutex_unlock(&dev->io_mutex);
	return 0;
}

static struct file_operations secbulk_fops = {
	.owner = THIS_MODULE,
	.read = secbulk_read,
	.write = secbulk_write,
	.open = secbulk_open,
	.release = secbulk_release
};

static struct usb_class_driver secbulk_class = {
	.name = "secbulk%d",
	.fops = &secbulk_fops,
	.minor_base = 100
};

static int secbulk_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int ret;
	struct secbulk_dev *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;

	printk(KERN_INFO "secbulk: probing for device %04X:%04X...\n", secbulk_table[0].idVendor, secbulk_table[0].idProduct);
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		ret = -ENOMEM;
		goto error;
	}
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &(iface_desc->endpoint[i].desc);
		if (!dev->bulk_out_endpointAddr
		&& usb_endpoint_is_bulk_out(endpoint)) {
			printk(KERN_INFO "secbulk: device %04X:%04X found!\n", secbulk_table[0].idVendor, secbulk_table[0].idProduct);
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
			break;
		}
	}
	if (!(dev->bulk_out_endpointAddr)) {
		ret = -EBUSY;
		goto error;
	}
	ret = usb_register_dev(interface, &secbulk_class);
	if (ret) {
		printk(KERN_ERR "secbulk: cannot register USB device: %d\n", ret);
		return ret;
	}
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	usb_set_intfdata(interface, dev);
	mutex_init(&dev->io_mutex);
	return 0;

error:
	if (!dev) {
		kfree(dev);
  }
	return ret;
}

static struct usb_driver secbulk_driver= {
	.name = "secbulk",
	.probe = secbulk_probe,
	.disconnect = secbulk_disconnect,
	.id_table = secbulk_table,
	.supports_autosuspend = 0,
};

static int __init secbulk_init(void)
{
	int result;

	printk(KERN_INFO "secbulk: driver loaded\n");
	result = usb_register(&secbulk_driver);
	if(result) {
    printk(KERN_ERR "secbulk: cannot register as USB driver: %d", result);
		return result;
	}
	return 0;
}
module_init(secbulk_init);

static void __exit secbulk_exit(void)
{
	usb_deregister(&secbulk_driver);
	printk(KERN_INFO "secbulk: driver unloaded\n");
}
module_exit(secbulk_exit);

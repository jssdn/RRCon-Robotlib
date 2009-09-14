/*
 *	Bit-bang i2c driver using xilinx gpio core.
 * 
 *      Stephen Neuendorffer <stephen.neuendorffer@xilinx.com>
 *      Copyright 2007, Xilinx Inc., All Rights Reserved
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/xilinx_devices.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#include <linux/device.h>
#else
#include <linux/platform_device.h>
#endif
//#include <xgpio_l.h>

#include <asm/io.h>
#include <asm/irq.h>

#include "xilinx_common/xbasic_types.h"

#define DRIVER_NAME             "xilinx_gpio_iic"

static int xilinx_gpio_iic_probe(struct device *device);
static int xilinx_gpio_iic_remove(struct device *device);

struct device_driver xilinx_gpio_iic_driver = {
	.bus = &platform_bus_type,
	.name = DRIVER_NAME,
	.probe = xilinx_gpio_iic_probe,
	.remove = xilinx_gpio_iic_remove,
};

/* Our private per device data. */
struct xilinx_gpio_iic_drvdata {
	struct i2c_adapter adap;	/* The Linux I2C core data  */
	struct i2c_algo_bit_data bit_data; /* Bit bang device driver hooks */
	int index;		/* index taken from platform_device */
	struct completion complete;	/* for waiting for interrupts */
        struct resource *regs_res;
        u32 regs;
        u32 gpio_state;
};

#define DATA_BIT 0x1
#define CLOCK_BIT 0x2

void xilinx_iic_gpio_setsda(void *data, int state) {
	struct xilinx_gpio_iic_drvdata *drvdata = data;
        if(state) {
                // Set bit to input.
                drvdata->gpio_state |= DATA_BIT;
        } else {
                // Set bit to output.
                drvdata->gpio_state &= ~DATA_BIT;
        }
        // Tristate REG.
        out_be32((void *)(drvdata->regs + 0x4), drvdata->gpio_state);
}
void xilinx_iic_gpio_setscl(void *data, int state) {
	struct xilinx_gpio_iic_drvdata *drvdata = data;
        if(state) {
                // Set bit to input.
                drvdata->gpio_state |= CLOCK_BIT;
        } else {
                // Set bit to output.
                drvdata->gpio_state &= ~CLOCK_BIT;
        }
        // Tristate REG.
        out_be32((void *)(drvdata->regs + 0x4), drvdata->gpio_state);
}
int xilinx_iic_gpio_getsda(void *data) {
	struct xilinx_gpio_iic_drvdata *drvdata = data;
        // Data REG.
        return in_be32((void *)(drvdata->regs + 0x8)) & DATA_BIT; 
}
int xilinx_iic_gpio_getscl(void *data) {
	struct xilinx_gpio_iic_drvdata *drvdata = data;
        // Data REG.
        u32 result = in_be32((void *)(drvdata->regs + 0x8)) & CLOCK_BIT;
        return (result >> 1);
}


static int xilinx_gpio_iic_remove(struct device *dev)
{
	struct xilinx_gpio_iic_drvdata *drvdata;

	drvdata = dev_get_drvdata(dev);

        i2c_del_adapter(&drvdata->adap);
        iounmap((void *)drvdata->regs_res->start);
        release_mem_region(drvdata->regs_res->start, 
                        drvdata->regs_res->end - drvdata->regs_res->start + 1);

	kfree(drvdata);

	return 0;
}

static int xilinx_gpio_iic_probe(struct device *dev)
{
        struct platform_device *pdev;
	struct xilinx_gpio_iic_drvdata *drvdata;
        int error;
        int retval;
        int reg_count;

        if(!dev) {
                return -EINVAL;
        }

        pdev = to_platform_device(dev);

	/* Allocate the dev and zero it out. */
	drvdata = kzalloc(sizeof(struct xilinx_gpio_iic_drvdata), GFP_KERNEL);

	if (!drvdata) {
		dev_err(dev, "Cannot allocate struct xilinx_gpio_iic_drvdata\n");
		retval = -ENOMEM;
		goto out;
	}
	dev_set_drvdata(dev, drvdata);

        drvdata->bit_data.setsda = xilinx_iic_gpio_setsda;
        drvdata->bit_data.setscl = xilinx_iic_gpio_setscl;
        drvdata->bit_data.getsda = xilinx_iic_gpio_getsda;
        drvdata->bit_data.getscl = xilinx_iic_gpio_getscl;
        drvdata->gpio_state = 3;
        drvdata->bit_data.data = drvdata;
        drvdata->bit_data.udelay = 50; // 10KHz
        drvdata->bit_data.timeout = 1000;

	drvdata->regs_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!drvdata->regs_res) {
		retval = -ENODEV;
		goto cleanup_drvdata;
	}
        reg_count = drvdata->regs_res->end - drvdata->regs_res->start + 1;

	/* initialize fields to satisfy i2c  */
	strcpy(drvdata->adap.name, dev->bus_id);
	drvdata->index = pdev->id;

	if (!request_mem_region(drvdata->regs_res->start, reg_count, DRIVER_NAME)) {
		printk(KERN_ERR "Couldn't lock memory region at %p\n",
			(void *)drvdata->regs_res->start);
		retval = -EBUSY;
		goto cleanup_drvdata;
	}

	/* Change the addresses to be virtual; save the old ones to restore. */
	drvdata->regs = (u32) ioremap(drvdata->regs_res->start, 
                        drvdata->regs_res->end - drvdata->regs_res->start + 1);

	drvdata->adap.algo_data = &(drvdata->bit_data);
	error = i2c_bit_add_bus(&drvdata->adap);

        // When configured as outputs, drive low.
        out_be32((void *)(drvdata->regs), 0);

        // Issue a stop sequence, getting the lines
        // back to high impedance.
        xilinx_iic_gpio_setsda(drvdata, 0);
        xilinx_iic_gpio_setscl(drvdata, 1);
        udelay(drvdata->bit_data.udelay);
        xilinx_iic_gpio_setsda(drvdata, 1);

	if (error) {
		dev_err(dev, "could not add i2c adapter\n");
                retval = error;
		goto cleanup_add_bus;
	}
	
	printk("%s #%d at 0x%08X mapped to 0x%08X\n",
                        drvdata->adap.name, drvdata->index,
                        drvdata->regs_res->start, drvdata->regs);

        return 0;

 cleanup_add_bus:
        iounmap(drvdata->regs_res->start);
        release_mem_region(drvdata->regs_res->start, reg_count);
 cleanup_drvdata:
        kfree(drvdata);
 out:
	return retval;
}

static int __init xilinx_gpio_iic_init(void)
{
	return driver_register(&xilinx_gpio_iic_driver);
}

static void __exit xilinx_gpio_iic_cleanup(void)
{
	driver_unregister(&xilinx_gpio_iic_driver);
}

module_init(xilinx_gpio_iic_init);
module_exit(xilinx_gpio_iic_cleanup);

MODULE_AUTHOR("Stephen Neuendorffer <stephen.neuendorffer@xilinx.com>");
MODULE_DESCRIPTION("Xilinx GPIO IIC driver");
MODULE_LICENSE("GPL");

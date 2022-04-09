#include <linux/module.h>
#include <linux/fs.h> // for alloc_chrdev_region
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include "platform.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "%s : " fmt,  __func__

/* Device private data structure */
struct pcdev_private_data
{
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;
};

/* Driver private data structure */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_num_base;
    struct class *class_pcd;
    struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
    return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t * f_pos)
{
    return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* File operations for the driver */
struct file_operations pcd_fops = {
    .open = pcd_open,
    .read = pcd_read,
    .write = pcd_write,
    .llseek = pcd_lseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};

/* gets called when the device is removed from the system */
int pcd_platform_driver_remove(struct platform_device *pdev)
{
    pr_info("A device is removed\n");
    return 0;
}

/* gets called when matched platform device is found */
int pcd_platform_driver_probe(struct platform_device *pdev)
{
    int ret;
    
    struct pcdev_private data *dev_data;

    struct pcdev_platform_data *pdata;

    /* 1. Get the platform data */
    // pdata = pdev->dev.platform_data;
    pdata = (struct pcdev_platform_data*)dev_get_platdata(&pdev->dev);
    if (!pdata)
    {
        pr_err("No platform data available\n");
        ret = -EINVAL;
        goto out;
    }

    /* 2. Dynamically allocate memory for the device private data */

    /* 3. Dynamically allocate memory for the device buffer using size 
    information from the platform data */

    /* 4. Get the device number */

    /* 5. Do cdev init and cdev add */

    /* 6. Create device file for the detected platform device */

    /* 7. Error handling */

    pr_info("A device is detected\n");
    return 0;

out:
    pr_err("Device probe failed\n");
    return ret;
}

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .driver = {
        .name = "pseudo-char-device"
    }
};

static int __init pcd_platform_driver_init(void)
{
    int ret;

    /* 1. Dynamically allocate a device number for MAX_DEVICES */
    ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
    if (ret < 0)
    {
        pr_err("Allooc chrdev failed\n");
        return ret;
    }

    /* 2. Create device class under /sys/class */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if (IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
        return ret;
    }

    /* 3. Register a platform driver */
    platform_driver_register(&pcd_platform_driver);

    pr_info("pcd platform driver loaded\n");
    return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
    /* 1. Unregister the platform driver */
    platform_driver_unregister(&pcd_platform_driver);

    /* 2. Class destroy */
    class_destroy(pcdrv_data.class_pcd);

    /* 3. Unregister device numbers for MAX_DEVICES */
    unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
    pr_info("pcd platform driver unloaded\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pawel Drozdz");
MODULE_DESCRIPTION("A pseudo character platform driver handles n devices");
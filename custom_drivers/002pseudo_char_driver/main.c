#include <linux/module.h>
#include <linux/fs.h> // for alloc_chrdev_region
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define DEV_MEM_SIZE 512

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "%s:" fmt,  __func__

/* pseudo device's memory */
char device_buffer[DEV_MEM_SIZE];

dev_t device_number;

struct cdev pcd_cdev;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
    pr_info("lseek requested\n");
    pr_info("Current file position = %lld\n", filp->f_pos);

    loff_t tmp;

    switch(whence)
    {
        case SEEK_SET:
            if (offset > DEV_MEM_SIZE || offset < 0)
            {
                return -EINVAL;
            }
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            tmp = filp->f_pos + offset;
            if (tmp > DEV_MEM_SIZE || tmp < 0)
            {
                return -EINVAL;
            }
            filp->f_pos += offset;
            break;
        case SEEK_END:
            tmp = DEV_MEM_SIZE + offset;
            if (tmp > DEV_MEM_SIZE || tmp < 0)
            {
                return -EINVAL;
            }
            filp->f_pos = DEV_MEM_SIZE + offset;
            break;
        default:
            return -EINVAL;
    }

    pr_info("New value of file position = %lld\n", filp->f_pos);
    return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t * f_pos)
{
    pr_info("read requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /* Examin the count */
    if ((*f_pos + count) > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - *f_pos;
    }

    /* Copy data from kernel space into user space */
    if (copy_to_user(buff, &device_buffer[*f_pos], count))
    {
        return -EFAULT;
    }

    /* Update current file position */
    *f_pos += count;

    pr_info("Number of bytes succesfully read = %zu\n", count);
    pr_info("Updated file position = %lld\n", *f_pos);

    /* Return number of bytes which have been succesfully read */
    return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Write requested for %zu bytes\n", count);
    pr_info("Current file position = %lld\n", *f_pos);

    /* Examin the count */
    if ((*f_pos + count) > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - *f_pos;
    }

    if (!count)
    {
        pr_err("No space left on the device");
        return -ENOMEM;
    }

    /* Copy data from user space into kernel space */
    if (copy_from_user(&device_buffer[*f_pos], buff, count))
    {
        return -EFAULT;
    }

    /* Update current file position */
    *f_pos += count;

    pr_info("Number of bytes succesfully written = %zu\n", count);
    pr_info("Updated file position = %lld\n", *f_pos);

    /* Return number of bytes which have been succesfully read */
    return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    /* We do not implement this function because this driver is pseudo-char-driver */
    pr_info("open was succesfull\n");
    return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
    /* We do not implement this function because this driver is pseudo-char-driver */
    pr_info("release was succesfull\n");
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

struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_driver_init(void)
{
    /*1. Dynamically allocate a device number */
    alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");

    pr_info("%s : Device number <major>.<minor> = %d.%d\n", __func__, MAJOR(device_number), MINOR(device_number));

    /* 2. Initialize a cdev structure */
    cdev_init(&pcd_cdev, &pcd_fops);

    /* 3. Register a device (cdev structure) with VFS */
    pcd_cdev.owner = THIS_MODULE;
    cdev_add(&pcd_cdev, device_number, 1);

    /*4. Create device class under /sys/class */
    class_pcd = class_create(THIS_MODULE, "ocd_class");

    /*5. Populate the sysfs with device information */
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");

    pr_info("Module init was succesfull\n");

    return 0;
}

static void __exit pcd_driver_cleanup(void)
{
    /* The order of cleanup functions have to be in reverse order
       to the initialization */
    device_destroy(class_pcd, device_number);
    class_destroy(class_pcd);
    cdev_del(&pcd_cdev);
    unregister_chrdev_region(device_number, 1);

    pr_info("Module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pawel Drozdz");
MODULE_DESCRIPTION("A pseudo character driver");
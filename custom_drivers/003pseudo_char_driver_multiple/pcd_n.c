#include <linux/module.h>
#include <linux/fs.h> // for alloc_chrdev_region
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "%s:" fmt,  __func__

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

/* pseudo device's memory */
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/* Device private data structure */
struct pcdev_private_data
{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev cdev;
};

/* Driver private data structure */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_number;
    struct class *class_pcd;
    struct device *device_pcd;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data =
{
    .total_devices = NO_OF_DEVICES,
    .pcdev_data = {
        [0] = {
            .buffer = device_buffer_pcdev1,
            .size = MEM_SIZE_MAX_PCDEV1,
            .serial_number = "PCDEV1XYZ123",
            .perm = 0x1 /* RDONLY */
        },
        [1] = {
            .buffer = device_buffer_pcdev2,
            .size = MEM_SIZE_MAX_PCDEV2,
            .serial_number = "PCDEV2XYZ123",
            .perm = 0x10 /* WRDONLY */
        },
        [2] = {
            .buffer = device_buffer_pcdev2,
            .size = MEM_SIZE_MAX_PCDEV2,
            .serial_number = "PCDEV2XYZ123",
            .perm = 0x11 /* RDWR */
        },
        [3] = {
            .buffer = device_buffer_pcdev3,
            .size = MEM_SIZE_MAX_PCDEV3,
            .serial_number = "PCDEV3XYZ123",
            .perm = 0x11 /* RDWR */
        }
    }
};

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
#if 0
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
#endif
    return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t * f_pos)
{
#if 0
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
#endif
    return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
#if 0
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
#endif
    return -ENOMEM;
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

static int __init pcd_driver_init(void)
{
    int ret;

    /* Dynamically allocate a device number */
    ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices");
    if (ret < 0)
    {
        pr_err("Alloc chrdev failed\n");
        goto out;
    }

    /* Create device class under /sys/class */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if (IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto unreg_chrdev;
    }

    int i = 0;
    for (i = 0; i < NO_OF_DEVICES; ++i)
    {
        pr_info("%s : Device number <major>.<minor> = %d.%d\n", __func__, MAJOR(pcdrv_data.device_number + 1), MINOR(pcdrv_data.device_number + 1));

        /* Initialize a cdev structure */
        cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

        /* Register a device (cdev structure) with VFS */
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1);
        if (ret < 0)
        {
            pr_err("Device add failed\n");
            goto cdev_del;
        }

        /* Populate the sysfs with device information */
        pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, "pcdev-%u", i);
        if (IS_ERR(pcdrv_data.device_pcd))
        {
            pr_err("Device creation failed\n");
            ret = PTR_ERR(pcdrv_data.device_pcd);
            goto class_del;
        }
    }

    pr_info("Module init was succesfull\n");

    return 0;

cdev_del:
class_del:
    for (; i >= 0; --i)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);

unreg_chrdev:
    unregister_chrdev_region(pcdrv_data.device_number, 1);

out:
    pr_err("Module insertion failed\n");
    return ret;
}

static void __exit pcd_driver_cleanup(void)
{
    int i;
    for (i = 0; i < NO_OF_DEVICES; ++i)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number, 1);

    pr_info("Module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pawel Drozdz");
MODULE_DESCRIPTION("A pseudo character driver handles n devices");
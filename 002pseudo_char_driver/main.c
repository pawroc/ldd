#include <linux/module.h>

#define DEV_MEM_SIZE 512

char device_buffer[DEV_MEM_SIZE]

static int __init pcd_init(void)
{
    return 0;
}

static void __exit pcd_driver_cleanup(coid)
{

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);
#include <linux/modul.h>

static int __init helloworld_init()
{
    pr_info("Hello world\n");
    return 0;
}

static void __exit helloworld_cleanup()
{
    pr_info("Good bye world\n");
}

module_init(helloworld_init);
module_exit(helloworld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pawel Drozdz")
MODULE_DESCRIPTION("A simple hello world kernel module");
MODULE_INFO(board, "Beaglebine black")
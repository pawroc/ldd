# ldd
Linux drivers course materials

## Host OS dependencies

- build-essential
- lzop
- u-boot-tools
- net-tools
- bison
- flex
- libssl-dev
- libncurses5-dev
- libncursesw5-dev
- unzip
- chrpath
- xz-utils
- minicom
- wget
- git-core

```bash
sudo apt-get update
sudo apt-get install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath xz-utils minicom wget git-core
```

### Docker

You can use __docker__ with all dependencies.

```bash
# Build image
docker build --rm -t ldd:latest .

# run container
docker run --rm -v <path_to_source_code>:/workspace -it ldd:latest
```

## Linux module building

```bash
# Building module
# make -C <path_to_kernel_source> M=<path_to_module_source> <target>
make -C /lib/modules/5.16.0-12parrot1-amd64/build M=$PWD modules

# Cleaning
make -C /lib/modules/5.16.0-12parrot1-amd64/build M=$PWD clean
```

### Intree module building

1. Download Linux Kernel source
2. Create a new folder in `drivers/<type_of_driver>/<driver_name>`, e.g. `drivers/char/my_c_dev/`
3. Copy source code into the directory
4. If you want your module to be visible in the `menuconfig` you should deliver also `Kconfig` file in the driver directory
5. Add the local Kconfig entry to upper level Kconfig, e.g. `source "drivers/char/my_c_dev/Kconfig`
6. Create local Makefile
7. Create `obj-$(config_item) += <module_name.o>`
    Because we do not know what config was chosen (m|n|y), we can use config name preceeded with CONFIG keyword, e.g. `obj-$(CONFIG_DRIVER_NAME_IN_MENUCONFIG) += main.o`
8. Add the local level Makefile into the upper level Makefile `obj-<config [y|m|n]> += <driver_path>`, e.g. `obj-y += my_c_dev`. __Note__: when config used `y`, then the folder will be always enabled (not the module itself, but the folder).

__Kconfig example__:
```
menu "Menu entry header description"
config DRIVER_NAME_IN_MENUCONFIG
    tristate "Short description of the module"
    default [n|m|y]
endmenu
```

## Tools

- `udevadm`

## Kernel APIs for drivers

- `alloc_chrdev_region()` - create device number
- `unregister_chrdev_region()`
- `cdev_init()`, `cdev_add()` - make a char device registration with the VFS
- `cdev_del()`
- `class_create()`, `device_create()` - create device files
- `class_destroy()`, `device_destroy()`

### printk priorities

The default priorities can be setup in `menuconfig` before kernel compilation.
In runtime it can be setup by overwriting `/proc/sys/kernel/printk` file, e.g. we can change the console log level
to 6 by `echo 6 > /proc/sys/kernel/printk`. Message from `printk` will be printed on a console when `printk`
priority is lower than current console log level.

### File operations

These are represented by `struct file`.

#### open

This operation creates `struct file` in the kernel space for each opened file descriptor.
Each call to `open` increments `f_count`.

#### release

This callback is issued when the last `close` is called. It means that the callback
is called only when `f_count` becomes 0.

#### read / write

In the prototype, there is an `__user` macro used. This macro is used to signal that the variable
that it corresponds to is a user space data which kernel shouldn't trust, e.g. pointer - it shouldn't
be dereferenced directly, but `copy_to_user` / `copy_from_user` should be used instead.

When `__user` macro is used, it is detected by `sparse` tool (GCC doesn't detect any errors).

## Beaglebone

### Toolchain

Downloaded either from package manager or from [arm-Developer page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads).

Required version: _arm-linux-gnueabihf_.

## Linux kernel build steps (on BeagleBoard example)

__Warning:__ These steps base on https://github.com/beagleboard/linux branch 4.14

__STEP 1:__
```bash
/*
 *removes all the temporary folder, object files, images generated during the previous build. 
 *This step also deletes the .config file if created previously 
 */
make ARCH=arm distclean
```

__STEP 2:__
```bash
/*creates a .config file by using default config file given by the vendor */

make ARCH=arm bb.org_defconfig
```

__Warning:__ This is valid only for branch 4.14 (and maybe some others) but not for the newer ones.
Newer kernel source code doesn't have the _bb.org_defconfig_ config file, therefore this won't work anymore.

__STEP 3:__
```bash
/*This step is optional. Run this command only if you want to change some kernel settings before compilation */ ​

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```

__STEP 4:__
```bash
/*Kernel source code compilation. This stage creates a kernel image "uImage" also all the device tree source files will be compiled, and dtbs will be generated */ ​

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
```

__STEP 5:__
```bash
/*This step builds and generates in-tree loadable(M) kernel modules(.ko) */

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-  modules  -j4
```

__STEP 6:__
```bash
/* This step installs all the generated .ko files in the default path of the computer (/lib/modules/<kernel_ver>) */​

sudo make ARCH=arm  modules_install
```

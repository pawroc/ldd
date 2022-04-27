# Linux Drivers Model

## General information

Every bus (platform driver) and device is represented in `/sysfs (/sys)` subsystem.
On the lowest level (in the code) they are represented by following structures:

![Components of the device model](pictures/components_of_the_device_model.png)

Note that there are many of already defined device drivers subsystems in Linux like:
- I2C
  - it's represented by `struct i2c_client`
  - module registering done with `module_i2c_driver` macro
- SPI
  - it's represented by `stuct spi_driver`
  - module registring done with `module_spi_driver` macro
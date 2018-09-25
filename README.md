# drvAsynSPI

This module provides EPICS device support for accessing SPI bus.

drvAsynSPI is derived from drvAsynI2C, Copyright 2015 Florian Feldbauer.

Original files of drvAsynI2C were obtained from [here](https://github.com/ffeldbauer/drvAsynI2C).

## Dependencies

   - EPICS base 3.14.12.5 (or newer)
   - asynDriver 4-26 (or newer)
   - spidev

## Installation

 1.  Edit "configure/RELEASE" according to your environment
 2.  Type `make` inside the top directory

## Usage

### Loading the driver
To load the driver use the command inside the IOC

>     drvAsynSPIConfigure( "NAME", "SPIbus", spi_mode, spi_max_speed, autoConnect )

where `NAME` is the name used by asyn to identify the driver,
`SPIbus` is the name of the SPI device on the filesystem,
`spi_mode` is the SPI mode number to configure CPOL and CPHA,
`spi_max_speed` is the SPI maximum frequency,
and `autoConnect` is a flag whether the driver should automatically
connect to the device or not.

### Writing to/Reading from the bus
The driver provides an asynOctet interface for reading from/writing to the bus.
This interfaces uses a binary format.

### Using StreamDevice
Refer to the readme inside the direcotry "streamDevice"

## Known Issues
   - none    

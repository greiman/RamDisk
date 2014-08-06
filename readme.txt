Introduction
------------

This is a very early version of a RamDisk library.  It provides a FAT style
file system on external RAM devices.

Much of the code has not throughly been tested.  It is likely that features
will be added, removed or changed.  The API is still under development

New Features in this Release
----------------------------

Support for MB85RS2MT FRAM has been added.

Support for multiple chips has been added.

Fast small template classes are provided for a single 23LCV1024 or MB85RS2MT
chip.


Hardware
--------

This library was developed using a Microchip 23LCV1024 1 Mbit SPI
serial SRAM with battery backup.

MB85RS2MT FRAM chips are also supported.

You must have working 23LCV1024, 23LC1024, MB85RS2MT hardware, or write your
own RAM device library.  You should be able to use other RAM devices by
writing a library with the RamBaseDevice API.

Html
----

Read the html documentation for more information.

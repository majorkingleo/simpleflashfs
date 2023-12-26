# Simple Flash FS

The filesystem is designed for use on flash datastorage usually used by microcontrollers.

Targets are:
* Page aligned writing
* Journaling (data and metadata)
* Powerloss safe
* Small memory footprint
* Memory footprint of a fixed size

## Configuration

The filesystem has to be configured in SimpleFlashFsConfig.h.

There you have to define flash page sizes, storage adresses, etc...

These values cannot be changed without dataloss.

## Internal Datastructure

* The filesystem is using flash page sizes to store the data. (Pagesize has to be configured)
* This is an inode based filesystem.
* The maximum number of files (inodes) will be defined at formatting time.
* An inode number will never be reused. Therefore it is a 64bit integer value.
* At the end of each inode there is a CRC sum. (So it's selfvalidating)
* To avoid to exceed the maximum write cycles of the flash memory, each write operation on any page 
  will be performed on a new page. So the flash storage is splitted into data pages and inode pages.
  Modifying an inode will result in writing the inode data into a new page, by increasing the inode version number.
  From now on, all old versions of the inode are invalid. They will be cleanuped by a job later.
  
  
### Journaling mechanism

Assume a file consists of an inode and one page of data.
Inodes are stored from page 1 to 9 data, afterwards.

* Page 1: Inode 1, Version 1 => is using Page 10
* Page 10: Data

Now Page 10 will be modified, modified data will be written first to Page 11 
then the inode has to be updated, because page 10 is not used anymore instead
page 11 is in use. The result will be: 

* Page 2: Inode 1, Version 2 => is using Page 11
* Page 11: Data-Modified

Now following data will be now on the flash:

* Page 1: Inode 1, Version 1 => is using Page 10 
* Page 2: Inode 1, Version 2 => is using Page 11
* Page 10: Data
* Page 11: Data-Modified

Page 1 and 10 are invalid, and can be erased by a later job.

If there is any power interuption (powerloss) during writing the filesystem
data and meta data will always be in a defined state.

For example: If during writing of page 2 (inode data) a power loss occours,
the crc sum will be invalid. So the dataset Inode 1, Version 2 will be invalid.
Since the previous dataset, Inode 1, Version 1 is still on disc, this one 
is valid. Data, as metadata.

If during writing of page 11 (data), a power loss occours, the inode (page 2) would not be written,
so still dataset 1,1 is the valid one.

The filesystem will be always consistence. At anytime, at any case.
This can be garanteed, by writing the data always before the meta data (inode data).

### First page

The initial page contains all configuration values from SimpleFlashFsConfig.h
as additional meta information.

* Bytes 13: magick string "SimpleFlashFs"
* Bytes 02: "BE" or "LE" (bigendian, or little endian number formats)
* Bytes 02: Filesystemformat version
* Bytes 04: Pagesize
* Bytes 08: Filesystem size in pages
* Bytes 04: Maximum number of inodes
* Bytes 02: Maximum path len (path + filename)
* Bytes 02: CRC checksum type
...
* last Bytes: CRC checksum (depends on checksum type)

### Inode

* Bytes 08: Inode number
* Bytes 08: Inode version number
* Bytes 02: File name length in bytes
* Bytes XX: File name
* Bytes 08: File attributes
* Bytes 08: File length in bytes
* Bytes 04: Number of Pages uses by this file
* Bytes 04: Page.. 1..N
...  
* Bytes 08: CRC checksum


#### Version number
Each inode has an version number. If multiple inodes with the same inode number exists, 
only the page with a valid crc sum and the highest version number is valid. All older versions
are invalid.

#### File attributes

* 0x0000 Data
* 0x0001 Buffered data
* 0x0002 Data with CRC sum check
* 0x0004 Always sync (flush at every write)

#### Number of Pages

If the number of pages is 0, the data is store inside the inode as long as it fits in the space between number of pages and the CRC checksum.

# Cleanup job

If the cleanup task is called when idle, invalid inodes as pages will be erased.
If a new page has to be allocated, and no free page is available, the cleanup function
is called immediate.

# Startup

On start up all inode pages have to be read from flash. The valid ones have to be kept in ram.

## Fast Startup

If there is a FRAM available, the index of valid inodes can be stored there. So on startup
you don't have to read all inodes.

# Implementation modes

## Warp 1

This is the basic implementation.

* The cleanup function has to be called manually, or will be called autoamtically, if there is no free page available.
* Target is using the smallest memory footprint


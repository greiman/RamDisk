/* Arduino RamDisk Library
 * Copyright (C) 2014 by William Greiman
 *
 * This file is part of the Arduino RamDisk Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino RamDisk Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef RamVolume_h
#define RamVolume_h
/**
 * \file
 * RamVolume class
 */
#include <Arduino.h>
#include <utility/FatStructs.h>
#include <utility/FatApiConstants.h>
#include <RamBaseDevice.h>
//------------------------------------------------------------------------------
/**
 * \typedef fat_t
 *
 * \brief Type for FAT entry
 */
typedef uint16_t fat_t;
//------------------------------------------------------------------------------
/** RamDiskParams version YYYYMMDD */
const uint32_t RAM_DISK_PARAMS_VERSION = 20140427;
/** \class RamDiskParams
 * \brief RamDiskParams file-system parameters stored at location zero.
 */
struct RamDiskParams {
           /** Version of the RamDiskParams structure. */
  uint32_t version;
           /** Number of data clusters in the volume. */
  uint16_t clusterCount;
           /** Start of the directory in 512 byte blocks */
  uint16_t dataStartBlock;
           /** Start of the data in 512 byte blocks */
  uint16_t rootDirStartBlock;
           /** Shift that produces blocksPerCluster.
            *  blocksPerCluster = 1 << clusterSizeShift*/
  uint8_t  clusterSizeShift;
}__attribute__((packed));
//------------------------------------------------------------------------------
/** \class RamVolume
 * \brief RamVolume Ram Disk FAT16 like volume.
 */
class RamVolume {
 public:
  /** \return The number of 512 byte blocks in a cluster */
  uint8_t blocksPerCluster() {return 1 << m_clusterSizeShift;}

  /** Set the current working volume to this volume. */
  void chvol() {RamVolume::m_curVol = this;}

  /** \return The count of clusters in the volume. */
  fat_t clusterCount() {return m_clusterCount;}

  /** \return Size of a cluster in bytes. */
  uint16_t clusterSizeBytes() {return 512 << m_clusterSizeShift;}

  /** \return Left shift to multiply by the number of blocks in a cluster. */
  uint8_t clusterSizeShift() {return m_clusterSizeShift;}

  /** \return Data start block number. */
  uint16_t dataStartBlock() {return m_dataStartBlock;}

  /** \return the Size of the FAT in blocks. */
  uint16_t fatSize() {return m_rootDirStartBlock - FAT_START_BLOCK;}

  /** \return The number of free clusters in the file-system. */
  uint16_t freeClusterCount();

  /**
   * Format the RamDisk volume.
   *
   * \param[in] dev the raw RAM device.
   *
   * \param[in] totalBlocks total number of 512 byte blocks to be used
   *            in the volume.
   *
   * \param[in]  dirBlocks Number of 512 bytes to be allocated to the
   *             directory.  Each block contains 16 directory entries.
   *
   * \param[in]  blocksPerCluster number of blocks in a cluster.
   *             blocksPerCluster must be a power of two less than 128.
   *
   * \return true for success or false for failure.
   */
  bool format(RamBaseDevice* dev, uint32_t totalBlocks = 0,
                     uint8_t dirBlocks = 4, uint8_t blocksPerCluster = 1);
  /**
   * Initialize the RamDisk volume.
   * \param[in] dev the raw RAM device.
   *
   * \return true for success or false for failure.
   */
  bool init(RamBaseDevice* dev);

  /** List directory contents.
   *
   * \param[in] pr Print stream that list will be written to.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   */
  void ls(Print* pr, uint8_t flags = 0);

  /** Print volume information.
   *
   * \param[in] pr Print stream that information will be written to.
   */
  void printInfo(Print* pr);

  /**
   * Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \param[in] fileName The name of the file to be removed.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the file is read only, \a fileName is not
   * found or an I/O error occurred.
   */
  bool remove(const char* fileName);

  /** \return The number of entries in the root directory. */
  uint16_t rootDirEntryCount() {return m_rootDirEntryCount;}

  /** \return The root directory start block number. */
  uint16_t rootDirStartBlock() {return m_rootDirStartBlock;}

 private:
  // Allow RamBaseFile access to RamVolume private data.
  friend class RamBaseFile;
//------------------------------------------------------------------------------
  static const uint32_t FAT_START_BLOCK = 1;      // start of FAT
  bool allocCluster(fat_t* cluster);
  uint32_t clusterAddress(fat_t cluster) {
    uint32_t lba = m_dataStartBlock
                   + ((uint32_t)(cluster - 2) << m_clusterSizeShift);
    return lba << 9;
  }
  uint16_t clusterOffsetMask() {return m_clusterOffsetMask;}
  uint32_t dirAddress(uint16_t index) {
    return  ((uint32_t)m_rootDirStartBlock << 9) + (index << 5);
  }
  uint32_t fatAddress(fat_t cluster) {
    return 512*FAT_START_BLOCK + (cluster << 1);
  }
  bool fatGet(fat_t cluster, fat_t* value);
  bool fatPut(fat_t cluster, fat_t value);
  bool freeChain(fat_t cluster);
  bool read(uint32_t address, void *buf, size_t nbyte) {
    return m_ramDev->read(address, buf, nbyte);
  }
  bool isEOC(fat_t cluster) {return cluster >= 0XFFF8;}
  bool readDir(uint16_t index, dir_t *dir);
  bool write(uint32_t address, const void *buf, size_t nbyte) {
    return m_ramDev->write(address, buf, nbyte);
  }
  bool writeDir(uint16_t index, dir_t *dir);
  static RamVolume* m_curVol;
  //----------------------------------------------------------------------------
  // Volume info
  uint16_t m_allocStartCluster;  // place to start free cluster search
  uint16_t m_dataStartBlock;     // start of data clusters
  uint16_t m_rootDirStartBlock;  // start of root dir
  bool     m_volumeValid;        // true if volume has been initialized
  uint8_t  m_clusterSizeShift;   // Shift to multiply by m_blocksPerCluster
  uint16_t m_clusterOffsetMask;  // Mask to determine offset in cluster
  uint16_t m_rootDirEntryCount;  // Entries in directory
  fat_t    m_clusterCount;       // total clusters in volume
  RamBaseDevice* m_ramDev;       // Raw RAM driver.
};
#endif  // RamVolume_h

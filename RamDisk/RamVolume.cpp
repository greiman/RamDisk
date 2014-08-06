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
#include <stddef.h>
#include <RamDisk.h>
//------------------------------------------------------------------------------
/** Macros for debug. */
#define DBG_FAIL_MACRO  //  Serial.print(__FILE__);Serial.println(__LINE__)
#define TRACE  // Serial.println(__LINE__)
//------------------------------------------------------------------------------
RamVolume* RamVolume::m_curVol = 0;
//------------------------------------------------------------------------------
bool RamVolume::allocCluster(fat_t* cluster) {
  fat_t freeCluster = m_allocStartCluster;
  for (fat_t i = 0; ; i++) {
    // return no free clusters
    if (i >= m_clusterCount) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // Fat has clusterCount + 2 entries
    if (freeCluster > m_clusterCount) freeCluster = 1;
    freeCluster++;
    fat_t value;
    if (!fatGet(freeCluster, &value)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (value == 0) break;
  }
  // mark cluster allocated
  if (!fatPut(freeCluster, FAT16EOC)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (*cluster != 0) {
    // connect chains
    if (!fatPut(*cluster, freeCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // return first cluster number to caller
  *cluster = freeCluster;
  m_allocStartCluster = freeCluster;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool RamVolume::fatGet(fat_t cluster, fat_t* value) {
  if (cluster > (m_clusterCount + 1)) return false;
  return m_ramDev->read(fatAddress(cluster), value, 2);
}
//------------------------------------------------------------------------------
bool RamVolume::fatPut(fat_t cluster, fat_t value) {
  if (cluster < 2 || cluster > (m_clusterCount + 1)) return false;
  return m_ramDev->write(fatAddress(cluster), &value, 2);
}
//------------------------------------------------------------------------------
bool RamVolume::format(RamBaseDevice* dev, uint32_t totalBlocks,
                     uint8_t dirBlocks, uint8_t blocksPerCluster) {
  RamDiskParams params;
  params.version = RAM_DISK_PARAMS_VERSION;
  if (totalBlocks == 0) totalBlocks = dev->sizeBlocks();
  if (dirBlocks == 0 || totalBlocks < (dirBlocks + blocksPerCluster + 2UL)) {
    return false;
  }
  // determine shift that is same as multiply by blocksPerCluster
  uint8_t shift = 0;
  for (uint8_t tmp = 1; blocksPerCluster != tmp; shift++, tmp <<= 1) {
    // Error if blocksPerCluster zero, not power of two, or too large.
    if (shift == 6) return false;
  }
  params.clusterSizeShift = shift;

  // Round to integral number of clusters.
  totalBlocks = (totalBlocks >> shift) << shift;

  uint16_t dataStart;
  uint16_t dirStart;
  uint32_t nc;
  for (dataStart = blocksPerCluster;; dataStart += blocksPerCluster) {
    nc = (totalBlocks - dataStart)/blocksPerCluster;
    uint16_t fatSize = (nc + 2 + 255)/256;
    // Error if too many clusters.
    if (fatSize > 255) return false;
    dirStart = FAT_START_BLOCK + fatSize;
    // Space required before data.
    uint16_t r = dirStart + dirBlocks;
    if (dataStart >= r) break;
  }
  params.rootDirStartBlock = dirStart;
  params.dataStartBlock = dataStart;
  params.clusterCount = nc;

  fat_t fat[2];
  fat[0] = fat[1] = 0;

  // Zero parameter block, FAT, and directory.
  uint32_t addLimit = 512UL*dataStart;
  for (uint32_t add = 0; add < addLimit; add += sizeof(fat)) {
    if (!dev->write(add, fat, sizeof(fat))) return false;
  }
  // Save parameters at address zero.
  if (!dev->write(0, &params, sizeof(params))) return false;
  fat[0] = fat[1] = 0XFFFF;

  // Reserve first two FAT entries. (like real FAT - could use just one).
  if (!dev->write(512*FAT_START_BLOCK, &fat, sizeof(fat))) return false;
  return true;
}
//------------------------------------------------------------------------------
// free a cluster chain
bool RamVolume::freeChain(fat_t cluster) {
  while (1) {
    fat_t next;
    if (!fatGet(cluster, &next)) return false;
    if (!fatPut(cluster, 0)) return false;
    if (cluster <= m_allocStartCluster) m_allocStartCluster = cluster - 1;
    if (isEOC(next)) return true;
    cluster = next;
  }
}
//------------------------------------------------------------------------------
uint16_t RamVolume::freeClusterCount() {
  uint16_t free = 0;
  // Fat has clusterCount + 2 entries.  First two are dummy
  for (fat_t i = 2; i < (m_clusterCount + 2); i++) {
    fat_t f;
    if (!fatGet(i, &f)) return 0;
    if (f == 0) free++;
  }
  return free;
}
//------------------------------------------------------------------------------
bool RamVolume::init(RamBaseDevice* dev) {
  RamDiskParams params;
  if (!dev->read(0, &params, sizeof(params))) {
    DBG_FAIL_MACRO;
    return false;
  }
  if (params.version != RAM_DISK_PARAMS_VERSION
    || params.clusterSizeShift > 6
    || params.rootDirStartBlock >= params.dataStartBlock) {
    DBG_FAIL_MACRO;
    return false;
  }
  m_clusterSizeShift = params.clusterSizeShift;
  m_clusterOffsetMask = (1UL << (9 + m_clusterSizeShift)) - 1;
  m_rootDirStartBlock = params.rootDirStartBlock;
  m_dataStartBlock = params.dataStartBlock;
  m_clusterCount = params.clusterCount;
  m_rootDirEntryCount = 16*(m_dataStartBlock - m_rootDirStartBlock);
  m_ramDev = dev;
  m_volumeValid = true;
  m_curVol = this;
  m_allocStartCluster = 1;
  return true;
}
//------------------------------------------------------------------------------
void RamVolume::ls(Print* pr, uint8_t flags) {
  dir_t d;
  for (uint16_t index = 0; index < rootDirEntryCount(); index++) {
    readDir(index, &d);
    if (d.name[0] == DIR_NAME_FREE) break;
    if (d.name[0] == DIR_NAME_DELETED || !DIR_IS_FILE(&d)) continue;
    // print file name with possible blank fill
    RamBaseFile::printDirName(pr, d, flags & (LS_DATE | LS_SIZE) ? 14 : 0);

    // print modify date/time if requested
    if (flags & LS_DATE) {
       RamBaseFile::printFatDate(pr, d.lastWriteDate);
       pr->write(' ');
       RamBaseFile::printFatTime(pr, d.lastWriteTime);
    }
    // print size if requested
    if (DIR_IS_FILE(&d) && (flags & LS_SIZE)) {
      pr->write(' ');
      pr->print(d.fileSize);
    }
    pr->println();
  }
}
//------------------------------------------------------------------------------
void RamVolume::printInfo(Print* pr) {
  pr->println(F("\nVolume Info:"));
  pr->print(F("FAT Size: "));
  pr->println(fatSize());
  pr->print(F("Dir Start Block: "));
  pr->println(rootDirStartBlock());
  pr->print(F("Dir Entry Count: "));
  pr->println(rootDirEntryCount());
  pr->print(F("Data Start Block: "));
  pr->println(dataStartBlock());
  pr->print(F("Cluster Count: "));
  pr->println(clusterCount());
  pr->print(F("Cluster Size Bytes: "));
  pr->println(clusterSizeBytes());
  pr->println();
}
//------------------------------------------------------------------------------
bool RamVolume::readDir(uint16_t index, dir_t* dir) {
  if (index >= m_rootDirEntryCount) {
    return false;
  }
//  uint32_t addr = (m_rootDirStartBlock << 9) + (index << 5);
  return m_ramDev->read(dirAddress(index), dir, sizeof(dir_t));
}
//------------------------------------------------------------------------------
bool RamVolume::remove(const char* fileName) {
  RamBaseFile file;
  if (!file.open(this, fileName, O_WRITE)) return false;
  return file.remove();
}
//------------------------------------------------------------------------------
bool RamVolume::writeDir(uint16_t index, dir_t* dir) {
  if (index >= m_rootDirEntryCount) {
    return false;
  }
//  uint32_t addr = (m_rootDirStartBlock << 9) + (index << 5);
  return m_ramDev->write(dirAddress(index), dir, sizeof(dir_t));
}

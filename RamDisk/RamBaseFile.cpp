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
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <RamVolume.h>
#include <RamBaseFile.h>
//------------------------------------------------------------------------------
/** Macros for debug. */
#define DBG_FAIL_MACRO   // Serial.print(__FILE__);Serial.println(__LINE__)
#define TRACE  // Serial.println(__LINE__)
//------------------------------------------------------------------------------
// callback function for date/time
void (*RamBaseFile::m_dateTime)(uint16_t* date, uint16_t* time) = NULL;
//------------------------------------------------------------------------------
// format 8.3 name for directory entry
static bool make83Name(const char* str, uint8_t* name) {
  uint8_t c;
  uint8_t n = 7;  // max index for part before dot
  uint8_t i = 0;
  // blank fill name and extension
  while (i < 11) name[i++] = ' ';
  i = 0;
  while ((c = *str++) != '\0') {
    if (c == '.') {
      if (n == 10) return false;  // only one dot allowed
      n = 10;  // max index for full 8.3 name
      i = 8;   // place for extension
    } else {
      // illegal FAT characters
      PGM_P p = PSTR("|<>^+=?/[];,*\"\\");
      uint8_t b;
      while ((b = pgm_read_byte(p++))) if (b == c) return false;
      // check length and only allow ASCII printable characters
      if (i > n || c < 0X21 || c > 0X7E) return false;
      // only upper case allowed in 8.3 names - convert lower to upper
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }
  // must have a file name, extension is optional
  return name[0] != ' ';
}
//==============================================================================
// RamBaseFile member functions
//------------------------------------------------------------------------------
// add a cluster to a file
bool RamBaseFile::addCluster() {
  if (!m_vol->allocCluster(&m_curCluster)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // if first cluster of file link to directory entry
  if (m_firstCluster == 0) {
    m_firstCluster = m_curCluster;
    m_flags |= F_FILE_DIR_DIRTY;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/**
 *  Close a file and write directory information.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include no file is open or an I/O error.
 */
bool RamBaseFile::close() {
  bool rtn = sync();
  m_flags = 0;
  return rtn;
}
//------------------------------------------------------------------------------
/**
 * Return a files directory entry
 *
 * \param[out] dir Location for return of the files directory entry.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool RamBaseFile::dirEntry(dir_t* dir) {
  if (!isOpen()) return false;
  if (!sync()) return false;
  return m_vol->readDir(m_dirEntryIndex, dir);
}
//------------------------------------------------------------------------------
/**
 * Get a string from a file.
 *
 * fgets() reads bytes from a file into the array pointed to by \a str, until
 * \a num - 1 bytes are read, or a delimiter is read and transferred to \a str,
 * or end-of-file is encountered. The string is then terminated
 * with a null byte.
 *
 * \param[out] str Pointer to the array where the string is stored.
 * \param[in] num Maximum number of characters to be read
 * (including the final null byte). Usually the length
 * of the array \a str is used.
 * \param[in] delim Optional set of delimiters. The default is "\n".
 *
 * \return For success fgets() returns the length of the string in \a str.
 * If no data is read, fgets() returns zero for EOF or -1 if an error occurred.
 **/
int16_t RamBaseFile::fgets(char* str, int16_t num, char* delim) {
  char ch;
  int16_t n = 0;
  int16_t r = -1;
  while ((n + 1) < num && (r = read(&ch, 1)) == 1) {
    str[n++] = ch;
    if (!delim) {
      if (ch == '\n') break;
    } else {
      if (strchr(delim, ch)) break;
    }
  }
  if (r < 0) {
    // read error
    return -1;
  }
  str[n] = '\0';
  return n;
}
//------------------------------------------------------------------------------
/**
 * Open a file by file name.
 *
 * \note The file must be in the root directory and must have a DOS
 * 8.3 name.
 *
 * \param[in] vol volume that contains the file.
 *
 * \param[in] fileName A valid 8.3 DOS name for a file in the root directory.
 *
 * \param[in] oflag Values for \a oflag are constructed by a bitwise-inclusive
 *  OR of flags from the following list
 *
 * O_READ - Open for reading.
 *
 * O_RDONLY - Same as O_READ.
 *
 * O_WRITE - Open for writing.
 *
 * O_WRONLY - Same as O_WRITE.
 *
 * O_RDWR - Open for reading and writing.
 *
 * O_APPEND - If set, the file offset shall be set to the end of the
 * file prior to each write.
 *
 * O_CREAT - If the file exists, this flag has no effect except as noted
 * under O_EXCL below. Otherwise, the file shall be created
 *
 * O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
 *
 * O_SYNC - Call sync() after each write.  This flag should not be used with
 * write(uint8_t), write_P(PGM_P), writeln_P(PGM_P), or the Arduino Print class.
 * These functions do character a time writes so sync() will be called
 * after each byte.
 *
 * O_TRUNC - If the file exists and is a regular file, and the file is
 * successfully opened and is not read only, its length shall be truncated to 0.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include the FAT volume has not been initialized,
 * a file is already open, \a fileName is invalid, the file does not exist,
 * is a directory, or can't be opened in the access mode specified by oflag.
 */
bool RamBaseFile::open(RamVolume* vol, const char* fileName, uint8_t oflag) {
  dir_t dir;
  uint8_t dname[11];   // name formated for dir entry
  int16_t empty = -1;  // index of empty slot

  if (!vol || isOpen()) return false;
  m_vol = vol;
  // error if invalid name
  if (!make83Name(fileName, dname)) return false;

  for (uint16_t index = 0; index < m_vol->rootDirEntryCount(); index++) {
    if (!m_vol->readDir(index, &dir)) return false;

    if (dir.name[0] == DIR_NAME_FREE || dir.name[0] == DIR_NAME_DELETED) {
      // remember first empty slot
      if (empty < 0) empty = index;
      // done if no entries follow
      if (dir.name[0] == DIR_NAME_FREE) break;
    } else if (!memcmp(dname, dir.name, 11)) {
      // don't open existing file if O_CREAT and O_EXCL
      if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) return false;
      // open existing file
      m_dirEntryIndex = index;
      return openDir(&dir, oflag);
    }
  }
  // error if directory is full
  if (empty < 0) return false;

  // only create file if O_CREAT and O_WRITE
  if ((oflag & (O_CREAT | O_WRITE)) != (O_CREAT | O_WRITE)) return false;

  // initialize as empty file
  memset(&dir, 0, sizeof(dir_t));
  memcpy(dir.name, dname, 11);
  m_dirEntryIndex = empty;

  // set timestamps
  if (m_dateTime) {
    // call user function
    m_dateTime(&dir.creationDate, &dir.creationTime);
  } else {
    // use default date/time
    dir.creationDate = FAT_DEFAULT_DATE;
    dir.creationTime = FAT_DEFAULT_TIME;
  }
  dir.lastAccessDate = dir.creationDate;
  dir.lastWriteDate = dir.creationDate;
  dir.lastWriteTime = dir.creationTime;

  // Write entry
  if (!m_vol->writeDir(m_dirEntryIndex, &dir)) return false;

  // open entry
  return openDir(&dir, oflag);
}
//------------------------------------------------------------------------------
/**
 * Open a file by file index.
 *
 * \param[in] vol volume that contains the file.
 *
 * \param[in] index The root directory index of the file to be opened.
 *
 * \param[in] oflag  See \link RamBaseFile::open(const char*, uint8_t)\endlink.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include the FAT volume has not been initialized,
 * a file is already open, \a index is invalid or is not the index of a
 * file or the file cannot be opened in the access mode specified by oflag.
 */
bool RamBaseFile::open(RamVolume* vol, uint16_t index, uint8_t oflag) {
  dir_t dir;
  if (!vol || isOpen()) return false;

  m_vol = vol;

  if (!m_vol->readDir(index, &dir)) return false;

  // error if unused entry
  if (dir.name[0] == DIR_NAME_FREE || dir.name[0] == DIR_NAME_DELETED) {
    return false;
  }
  m_dirEntryIndex = index;
  return openDir(&dir, oflag);
}
//------------------------------------------------------------------------------
bool RamBaseFile::openDir(dir_t* dir, uint8_t oflag) {
  if ((oflag & O_TRUNC) && !(oflag & O_WRITE)) {
    return false;
  }

  // error if long name, volume label or subdirectory
  if ((dir->attributes & (DIR_ATT_VOLUME_ID | DIR_ATT_DIRECTORY)) != 0) {
    return false;
  }
  // don't allow write or truncate if read-only
  if ((dir->attributes & DIR_ATT_READ_ONLY)
      && (oflag & (O_WRITE | O_TRUNC))) {
      return false;
  }

  m_curCluster = 0;
  m_curPosition = 0;
  m_fileSize = dir->fileSize;
  m_firstCluster = dir->firstClusterLow;
  m_flags = oflag & (O_ACCMODE | O_SYNC | O_APPEND);

  if (oflag & O_TRUNC ) return truncate(0);
  if (oflag & O_AT_END) return seekEnd();
  return true;
}
//------------------------------------------------------------------------------
/** %Print the name field of a directory entry in 8.3 format.
 *
 * \param[in] pr Print stream that name will be written to.
 * \param[in] dir The directory structure containing the name.
 * \param[in] width Blank fill name if length is less than \a width.
 */
void RamBaseFile::printDirName(Print* pr, const dir_t& dir, uint8_t width) {
  uint8_t w = 0;
  for (uint8_t i = 0; i < 11; i++) {
    if (dir.name[i] == ' ') continue;
    if (i == 8) {
      pr->write('.');
      w++;
    }
    pr->write(dir.name[i]);
    w++;
  }
  if (DIR_IS_SUBDIR(&dir)) {
    pr->write('/');
    w++;
  }
  while (w < width) {
    pr->write(' ');
    w++;
  }
}
//------------------------------------------------------------------------------
/** %Print a directory date field.
 *
 *  Format is yyyy-mm-dd.
 *
 * \param[in] pr Print stream that field will be written to.
 * \param[in] fatDate The date field from a directory entry.
 */
void RamBaseFile::printFatDate(Print* pr, uint16_t fatDate) {
  pr->print(FAT_YEAR(fatDate));
  pr->write('-');
  printTwoDigits(pr, FAT_MONTH(fatDate));
  pr->write('-');
  printTwoDigits(pr, FAT_DAY(fatDate));
}
//------------------------------------------------------------------------------
/** %Print a directory time field.
 *
 * Format is hh:mm:ss.
 *
 * \param[in] pr Print stream that field will be written to.
 * \param[in] fatTime The time field from a directory entry.
 */
void RamBaseFile::printFatTime(Print* pr, uint16_t fatTime) {
  printTwoDigits(pr, FAT_HOUR(fatTime));
  pr->write(':');
  printTwoDigits(pr, FAT_MINUTE(fatTime));
  pr->write(':');
  printTwoDigits(pr, FAT_SECOND(fatTime));
}

//------------------------------------------------------------------------------
/** %Print a value as two digits.
 *
 * \param[in] pr Print stream that value will be written to.
 * \param[in] v Value to be printed, 0 <= \a v <= 99
 */
void RamBaseFile::printTwoDigits(Print* pr, uint8_t v) {
  char str[3];
  str[0] = '0' + v/10;
  str[1] = '0' + v % 10;
  str[2] = 0;
  pr->print(str);
}
//------------------------------------------------------------------------------
/**
 * Read the next byte from a file.
 *
 * \return For success read returns the next byte in the file as an int.
 * If an error occurs or end of file is reached -1 is returned.
 */
int16_t RamBaseFile::read() {
  uint8_t b;
  return read(&b, 1) == 1 ? b : -1;
}
//------------------------------------------------------------------------------
/**
 * Read data from a file at starting at the current file position.
 *
 * \param[out] buf Pointer to the location that will receive the data.
 *
 * \param[in] nbyte Maximum number of bytes to read.
 *
 * \return For success read returns the number of bytes read.
 * A value less than \a nbyte, including zero, may be returned
 * if end of file is reached.
 * If an error occurs, read returns -1.  Possible errors include
 * read called before a file has been opened, the file has not been opened in
 * read mode, a corrupt file system, or an I/O error.
 */
int RamBaseFile::read(void* buf, size_t nbyte) {
  // convert void pointer to uin8_t pointer
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);

  // error if not open for read
  if (!(m_flags & O_READ)) {
    return -1;
  }
  // don't read beyond end of file
  if ((m_curPosition + nbyte) > m_fileSize) nbyte = m_fileSize - m_curPosition;

  // bytes left to read in loop
  size_t nToRead = nbyte;
  while (nToRead > 0) {
    uint16_t clusterOffset = m_curPosition & m_vol->clusterOffsetMask();
    if (clusterOffset == 0) {
      // start next cluster
      if (m_curCluster == 0) {
        m_curCluster = m_firstCluster;
      } else {
        if (!m_vol->fatGet(m_curCluster, &m_curCluster)) {
          return -1;
        }
      }
      // return error if bad cluster chain
      if (m_curCluster < 2 || isEOC(m_curCluster)) {
        return -1;
      }
    }
    // Max space in cluster.
    uint16_t n = m_vol->clusterSizeBytes() - clusterOffset;

    // Lesser of space in cluster and amount to read.
    if (n > nToRead) n = nToRead;

    uint32_t addr = m_vol->clusterAddress(m_curCluster) + clusterOffset;
    if (!m_vol->read(addr, dst, n)) {
      return -1;
    }

    m_curPosition += n;
    dst += n;
    nToRead -= n;
  }
  return nbyte;
}
//------------------------------------------------------------------------------
/**
 * Remove a file.  The directory entry and all data for the file are deleted.
 *
 * \note This function should not be used to delete the 8.3 version of a
 * file that has a long name. For example if a file has the long name
 * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include the file is not open for write
 * or an I/O error occurred.
 */
bool RamBaseFile::remove() {
  // error if file is not open for write
  if (!(m_flags & O_WRITE)) return false;
  if (m_firstCluster) {
    if (!m_vol->freeChain(m_firstCluster)) return false;
  }
  dir_t dir;
  if (!m_vol->readDir(m_dirEntryIndex, &dir)) return false;
  dir.name[0] = DIR_NAME_DELETED;
  m_flags = 0;
  return m_vol->writeDir(m_dirEntryIndex, &dir);
}
//------------------------------------------------------------------------------
/**
 * Sets the file's read/write position.
 *
 * \param[in] pos The new position in bytes from the beginning of the file.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool RamBaseFile::seekSet(uint32_t pos) {
  // error if file not open or seek past end of file
  if (!isOpen() || pos > m_fileSize) return false;
  if (pos == 0) {
    // set position to start of file
    m_curCluster = 0;
    m_curPosition = 0;
    return true;
  }
  fat_t n = ((pos - 1) >> 9) >> m_vol->clusterSizeShift();
  if (pos < m_curPosition || m_curPosition == 0) {
    // must follow chain from first cluster
    m_curCluster = m_firstCluster;
  } else {
    // advance from curPosition
    n -= ((m_curPosition - 1) >> 9) >> m_vol->clusterSizeShift();
  }
  while (n--) {
    if (!m_vol->fatGet(m_curCluster, &m_curCluster)) return false;
  }
  m_curPosition = pos;
  return true;
}
//------------------------------------------------------------------------------
/**
 *  The sync() call causes all modified data and directory fields
 *  to be written to the storage device.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include a call to sync() before a file has been
 * opened or an I/O error.
 */
bool RamBaseFile::sync() {
  if (m_flags & F_FILE_DIR_DIRTY) {
    dir_t dir;
    // cache directory entry
    if (!m_vol->readDir(m_dirEntryIndex, &dir)) return false;

    // update file size and first cluster
    dir.fileSize = m_fileSize;
    dir.firstClusterLow = m_firstCluster;

    // set modify time if user supplied a callback date/time function
    if (m_dateTime) {
      m_dateTime(&dir.lastWriteDate, &dir.lastWriteTime);
      dir.lastAccessDate = dir.lastWriteDate;
    }
    m_flags &= ~F_FILE_DIR_DIRTY;
    return m_vol->writeDir(m_dirEntryIndex, &dir);
  }
  return true;
}
//------------------------------------------------------------------------------
/**
 * The timestamp() call sets a file's timestamps in its directory entry.
 *
 * \param[in] flags Values for \a flags are constructed by a bitwise-inclusive
 * OR of flags from the following list
 *
 * T_ACCESS - Set the file's last access date.
 *
 * T_CREATE - Set the file's creation date and time.
 *
 * T_WRITE - Set the file's last write/modification date and time.
 *
 * \param[in] year Valid range 1980 - 2107 inclusive.
 *
 * \param[in] month Valid range 1 - 12 inclusive.
 *
 * \param[in] day Valid range 1 - 31 inclusive.
 *
 * \param[in] hour Valid range 0 - 23 inclusive.
 *
 * \param[in] minute Valid range 0 - 59 inclusive.
 *
 * \param[in] second Valid range 0 - 59 inclusive
 *
 * \note It is possible to set an invalid date since there is no check for
 * the number of days in a month.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool RamBaseFile::timestamp(uint8_t flags, uint16_t year, uint8_t month,
         uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  if (!isOpen()
    || year < 1980
    || year > 2107
    || month < 1
    || month > 12
    || day < 1
    || day > 31
    || hour > 23
    || minute > 59
    || second > 59) {
      return false;
  }
  dir_t dir;
  if (!m_vol->readDir(m_dirEntryIndex, &dir)) return false;

  uint16_t dirDate = FAT_DATE(year, month, day);
  uint16_t dirTime = FAT_TIME(hour, minute, second);
  if (flags & T_ACCESS) {
    dir.lastAccessDate = dirDate;
  }
  if (flags & T_CREATE) {
    dir.creationDate = dirDate;
    dir.creationTime = dirTime;
    // seems to be units of 1/100 second not 1/10 as Microsoft standard states
    dir.creationTimeTenths = second & 1 ? 100 : 0;
  }
  if (flags & T_WRITE) {
    dir.lastWriteDate = dirDate;
    dir.lastWriteTime = dirTime;
  }
  return m_vol->writeDir(m_dirEntryIndex, &dir);
}
//------------------------------------------------------------------------------
/**
 * Truncate a file to a specified length.  The current file position
 * will be maintained if it is less than or equal to \a length otherwise
 * it will be set to end of file.
 *
 * \param[in] length The desired length for the file.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include file is read only, file is a directory,
 * \a length is greater than the current file size or an I/O error occurs.
 */
bool RamBaseFile::truncate(uint32_t length) {
  // error if file is not open for write
  if (!(m_flags & O_WRITE)) return false;

  if (length > m_fileSize) return false;

  // fileSize and length are zero - nothing to do
  if (m_fileSize == 0) return true;
  uint32_t newPos = m_curPosition > length ? length : m_curPosition;
  if (length == 0) {
    // free all clusters
    if (!m_vol->freeChain(m_firstCluster)) return false;
    m_curCluster = m_firstCluster = 0;
  } else {
    fat_t toFree;
    if (!seekSet(length)) return false;
    if (!m_vol->fatGet(m_curCluster, &toFree)) return false;
    if (!isEOC(toFree)) {
      // free extra clusters
      if (!m_vol->fatPut(m_curCluster, FAT16EOC)) return false;
      if (!m_vol->freeChain(toFree)) return false;
    }
  }
  m_fileSize = length;
  m_flags |= F_FILE_DIR_DIRTY;
  if (!sync()) return false;
  return seekSet(newPos);
}
//------------------------------------------------------------------------------
/**
 * Write data at the current position of an open file.
 *
 * \note Data is moved to the cache but may not be written to the
 * storage device until sync() is called.
 *
 * \param[in] buf Pointer to the location of the data to be written.
 *
 * \param[in] nbyte Number of bytes to write.
 *
 * \return For success write() returns the number of bytes written, always
 * \a nbyte.  If an error occurs, write() returns -1.  Possible errors include
 * write() is called before a file has been opened, the file has not been opened
 * for write, device is full, a corrupt file system or an I/O error.
 *
 */
int RamBaseFile::write(const void* buf, size_t nbyte) {
  size_t nToWrite = nbyte;
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);

  // error if file is not open for write
  if (!(m_flags & O_WRITE)) goto writeErrorReturn;

  // go to end of file if O_APPEND
  if ((m_flags & O_APPEND) && m_curPosition != m_fileSize) {
    if (!seekEnd()) goto writeErrorReturn;
  }
  while (nToWrite > 0) {
    uint16_t clusterOffset = m_curPosition & m_vol->clusterOffsetMask();
    if (clusterOffset == 0) {
      // start of new cluster
      if (m_curCluster != 0) {
        fat_t next;
        if (!m_vol->fatGet(m_curCluster, &next)) goto writeErrorReturn;
        if (isEOC(next)) {
          // add cluster if at end of chain
          if (!addCluster()) goto writeErrorReturn;
        } else {
          m_curCluster = next;
        }
      } else {
        if (m_firstCluster == 0) {
          // allocate first cluster of file
          if (!addCluster()) goto writeErrorReturn;
        } else {
          m_curCluster = m_firstCluster;
        }
      }
    }
    // Max space in cluster.
    uint16_t n = m_vol->clusterSizeBytes() - clusterOffset;

    // Lesser of space in cluster and amount to write.
    if (n > nToWrite) n = nToWrite;

    uint32_t addr = m_vol->clusterAddress(m_curCluster) + clusterOffset;
    if (!m_vol->write(addr, src, n)) goto writeErrorReturn;

    m_curPosition += n;
    nToWrite -= n;
    src += n;
  }
  if (m_curPosition > m_fileSize) {
    // update fileSize and insure sync will update dir entry
    m_fileSize = m_curPosition;
    m_flags |= F_FILE_DIR_DIRTY;
  } else if (m_dateTime && nbyte) {
    // insure sync will update modified date and time
    m_flags |= F_FILE_DIR_DIRTY;
  }

  if (m_flags & O_SYNC) {
    if (!sync()) goto writeErrorReturn;
  }
  return nbyte;

 writeErrorReturn:
  writeError = true;
  return -1;
}
//------------------------------------------------------------------------------
/**
 * Write a byte to a file. Required by the Arduino Print class.
 *
 * \param[in] b byte to be written.
 *
 * Use RamBaseFile::writeError to check for errors.
 * \return one for success else zero.
 */
size_t RamBaseFile::write(uint8_t b) {
  return write(&b, 1) == 1 ? 1 : 0;
}
//------------------------------------------------------------------------------
/**
 * Write a string to a file. Used by the Arduino Print class.
 *
 * \param[in] str string to be written.
 *
 * Use RamBaseFile::writeError to check for errors.
 * \return number of bytes written or -1 if an error occurs.
 */
int16_t RamBaseFile::write(const char* str) {
  return write(str, strlen(str));
}
//------------------------------------------------------------------------------
/**
 * Write a PROGMEM string to a file.
 *
 * \param[in] str PROGMEM string to be written.
 *
 * Use RamBaseFile::writeError to check for errors.
 */
void RamBaseFile::write_P(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) write(c);
}
//------------------------------------------------------------------------------
/**
 * Write a PROGMEM string followed by CR/LF to a file.
 *
 * \param[in] str PROGMEM string to be written.
 *
 * Use RamBaseFile::writeError to check for errors.
 */
void RamBaseFile::writeln_P(PGM_P str) {
  write_P(str);
  write('\r');
  write('\n');
}

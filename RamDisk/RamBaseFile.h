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
#ifndef RamBaseFile_h
#define RamBaseFile_h
/**
 * \file
 * RamBaseFile class
 */
#include <Arduino.h>
#include <utility/FatStructs.h>
#include <utility/FatApiConstants.h>
#include <RamVolume.h>
//------------------------------------------------------------------------------
/** \class RamBaseFile
 * \brief RamBaseFile implements a minimal Arduino RamDisk Library
 *
 * RamBaseFile does not support subdirectories or long file names.
 */
class RamBaseFile {
 public:
  /*
   * Public functions
   */
  /** create with file closed */
  RamBaseFile() : m_flags(0) {}
  /** \return The current cluster number. */
  fat_t curCluster() const {return m_curCluster;}
  bool close();
  /** \return The current file position. */
  uint32_t curPosition() const {return m_curPosition;}
  /**
   * Set the date/time callback function
   *
   * \param[in] dateTime The user's callback function.  The callback
   * function is of the form:
   *
   * \code
   * void dateTime(uint16_t* date, uint16_t* time) {
   *   uint16_t year;
   *   uint8_t month, day, hour, minute, second;
   *
   *   // User gets date and time from GPS or real-time clock here
   *
   *   // return date using FAT_DATE macro to format fields
   *   *date = FAT_DATE(year, month, day);
   *
   *   // return time using FAT_TIME macro to format fields
   *   *time = FAT_TIME(hour, minute, second);
   * }
   * \endcode
   *
   * Sets the function that is called when a file is created or when
   * a file's directory entry is modified by sync(). All timestamps,
   * access, creation, and modify, are set when a file is created.
   * sync() maintains the last access date and last modify date/time.
   *
   * See the timestamp() function.
   */
  static void dateTimeCallback(
    void (*dateTime)(uint16_t* date, uint16_t* time)) {
    m_dateTime = dateTime;
  }
  /**
   * Cancel the date/time callback function.
   */
  static void dateTimeCallbackCancel() {m_dateTime = NULL;}
  bool dirEntry(dir_t* dir);
  int16_t fgets(char* str, int16_t num, char* delim = 0);
  /** \return The file's size in bytes. */
  uint32_t fileSize() const {return m_fileSize;}
  /**
   * Checks the file's open/closed status for this instance of RamDisk.
   * \return The value true if a file is open otherwise false;
   */
  bool isOpen() const {return (m_flags & O_ACCMODE) != 0;}
  bool open(RamVolume* vol, const char* fileName, uint8_t oflag);
  /**
   * Open a file by file name.
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
   * O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file
   * exists.
   *
   * O_SYNC - Call sync() after each write.  This flag should not be used with
   * write(uint8_t), write_P(PGM_P), writeln_P(PGM_P), or the Arduino Print
   * class.
   *
   * These functions do character a time writes so sync() will be called
   * after each byte.
   *
   * O_TRUNC - If the file exists and is a regular file, and the file is
   * successfully opened and is not read only, its length shall be truncated
   * to 0.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the FAT volume has not been initialized,
   * a file is already open, \a fileName is invalid, the file does not exist,
   * is a directory, or can't be opened in the access mode specified by oflag.
   */
  bool open(const char* fileName, uint8_t oflag) {
    return open(RamVolume::m_curVol, fileName, oflag);
  }
  bool open(RamVolume* vol, uint16_t entry, uint8_t oflag);
  /**
   * Open a file by file index.
   *
   * \param[in] entry The root directory index of the file to be opened.
   *
   * \param[in] oflag  See
   * \link RamBaseFile::open(const char*, uint8_t)\endlink.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the FAT volume has not been initialized,
   * a file is already open, \a index is invalid or is not the index of a
   * file or the file cannot be opened in the access mode specified by oflag.
   */
  bool open(uint16_t entry, uint8_t oflag) {
    return open(RamVolume::m_curVol, entry, oflag);
  }
  static void printDirName(Print* pr, const dir_t& dir, uint8_t width);
  static void printFatDate(Print* pr, uint16_t fatDate);
  static void printFatTime(Print* pr, uint16_t fatTime);
  static void printTwoDigits(Print* pr, uint8_t v);
  int16_t read();
  int read(void* buf, size_t nbyte);

  bool remove();

  /** Sets the file's current position to zero. */
  void rewind() {m_curPosition = m_curCluster = 0;}
  /**
   * Seek to current position plus \a pos bytes. See RamDisk::seekSet().
   *
   * \param[in] pos offset from current position.
   *
   * \return true for success or false for failure.
   */
  bool seekCur(int32_t pos) {return seekSet(m_curPosition + pos);}
  /** Set the files position to end-of-file + \a offset. See seekSet().
   * \param[in] offset The new position in bytes from end-of-file.
   * \return true for success or false for failure.
   */
  bool seekEnd(int32_t offset = 0) {return seekSet(m_fileSize + offset);}
  bool seekSet(uint32_t pos);
  bool sync();
  bool timestamp(uint8_t flag, uint16_t year, uint8_t month, uint8_t day,
          uint8_t hour, uint8_t minute, uint8_t second);
  bool truncate(uint32_t size);
  /** RamDisk::writeError is set to true if an error occurs during a write().
   * Set RamDisk::writeError to false before calling print() and/or write()
   * and check for true after calls to write() and/or print().
   */
  bool writeError;
  int write(const void *buf, size_t nbyte);
  size_t write(uint8_t b);
  int16_t write(const char* str);
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
  //----------------------------------------------------------------------------
  friend class RamVolume;
  //----------------------------------------------------------------------------
 private:
  // define fields in m_flags
  static uint8_t const F_OFLAG = O_ACCMODE | O_APPEND | O_SYNC;
  static uint8_t const F_FILE_DIR_DIRTY = 0X80;  // require sync directory entry
  // callback function for date/time
  static void (*m_dateTime)(uint16_t* date, uint16_t* time);

  uint32_t m_curPosition;   // current byte offset
  uint32_t m_fileSize;      // fileSize
  uint8_t m_flags;          // see above for bit definitions
  int16_t m_dirEntryIndex;  // index of directory entry for open file
  fat_t m_curCluster;       // current cluster
  fat_t m_firstCluster;     // first cluster of file
  RamVolume* m_vol;         // volume for this file

  // end of chain test
  bool isEOC(fat_t cluster) {return cluster >= 0XFFF8;}
  // allocate a cluster to a file
  bool addCluster();
  bool openDir(dir_t* dir, uint8_t oflag);
};
#endif  // RamBaseFile_h

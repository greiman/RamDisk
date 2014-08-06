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
#ifndef RamFile_h
#define RamFile_h
/**
 * \file
 * RamFile class
 */
#include <RamBaseFile.h>
//------------------------------------------------------------------------------
/**
 * \class RamFile
 * \brief RamBaseFile with Print.
 */
class RamFile : public RamBaseFile, public Print {
 public:
  RamFile() {}
  int write(const char* str);
  int write(const void* buf, size_t nbyte);
  size_t write(uint8_t b);
//------------------------------------------------------------------------------
  /**
   * Write data at the current position of an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] size Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a size.  If an error occurs, write() returns -1.  Possible errors include
   * write() is called before a file has been opened, the file has not been opened
   * for write, device is full, a corrupt file system or an I/O error.
   *
   */
  size_t write(const uint8_t *buf, size_t size) {
    return RamBaseFile::write(buf, size);}
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
};
#endif  // RamFile_h

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
#ifndef RamBaseDevice_h
#define RamBaseDevice_h
/**
 * \file
 * RamBaseDevice class
 */
#define __need_size_t
#include <stddef.h>
#include <stdint.h>
//------------------------------------------------------------------------------
/** \class RamBaseDevice
 * \brief RamBaseDevice virtual base class for derived RAM classes.
 */
class RamBaseDevice {
 public:
  /**
   * Read data from RAM.
   *
   * \param[in] address Location in RAM to be read.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] nbyte Number of bytes to read.
   *
   * \return true for success or false for failure.
   */
  virtual bool read(uint32_t address, void *buf, size_t nbyte) = 0;
  /**
   * \return Total number of 512 byte blocks in the RAM device.
   */
  virtual uint32_t sizeBlocks() = 0;
  /**
   * Write data to RAM.
   *
   * \param[in] address Location in RAM to be written.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] nbyte Number of bytes to write.
   *
   * \return true for success or false for failure.
   */
  virtual bool write(uint32_t address, const void *buf, size_t nbyte) = 0;
};
#endif  // RamBaseDevice_h

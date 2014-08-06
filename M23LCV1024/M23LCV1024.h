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

 * You should have received a copy of the GNU General Public License
 * along with the Arduino RamDisk Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef M23LCV1024_h
#define M23LCV1024_h
#include <Arduino.h>
#include <RamBaseDevice.h>
/**
 * \file
 * 23LCV1024 Arduino Library
 */

/** Set SRAM_USE_SPI_LIB non-zero to use the Arduino SPI library */
#define SRAM_USE_SPI_LIB !defined(__AVR__)

#ifdef __AVR__
#include <T23LCV1024.h>
#endif  // __AVR__

/** Maximum number of chips supported. */
const uint8_t MAX_M23LCV1024_COUNT = 8;

/**
 * \class M23LCV1024
 * \brief 23LCV1024 raw I/O library for multiple chips.
 */
class M23LCV1024 : public RamBaseDevice {
 public:
  /** Constructor */
  M23LCV1024() : m_chipCount(0) {}
  /** Initialize 23LCV1024 chips.
   * \param[in] csPin array of chip select pin numbers.
   * \param[in] chipCount number of chips to use.
   * \return false if too many chips else true.
   */
  bool begin(uint8_t* csPin, uint8_t chipCount);

  /** Initialize one 23LCV1024.
   * \param[in] csPin RAM chip select pin.
   * \return true.
   */
  bool begin(uint8_t csPin) {return begin(&csPin, 1);}

  /** Read a block from the 23LCV1024.
   * \param[in] address start location in the 23LCV1024.
   * \param[out] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return true unless address is out of range.
   */
  bool read(uint32_t address, void *buf, size_t nbyte);

  uint32_t sizeBlocks() {return 256*m_chipCount;}

  /** Write a block to the 23LCV1024.
   * \param[in] address start location in the 23LCV1024.
   * \param[in] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return true unless address is out of range.
   */
  bool write(uint32_t address, const void *buf, size_t nbyte);

 private:
  static const uint8_t READ_DATA = 0X03;
  static const uint8_t WRITE_DATA = 0X02;
  static const uint8_t WRITE_MODE = 0X01;
  static const uint8_t SEQ_MODE = 0X40;

  bool sendCmdAddress(uint8_t cmd, uint32_t address);

#if SRAM_USE_SPI_LIB
  void csLow() {digitalWrite(m_csPin[m_curChip], LOW);}
  void csHigh() {digitalWrite(m_csPin[m_curChip], HIGH);}
  uint8_t m_csPin[MAX_M23LCV1024_COUNT];
#else  // SRAM_USE_SPI_LIB
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void csLow() {
    cli();
    *m_csPort[m_curChip] &= ~m_csBit[m_curChip];
    sei();
  }
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void csHigh() {
    cli();
    *m_csPort[m_curChip] |= m_csBit[m_curChip];
    sei();
  }
  uint8_t m_csBit[MAX_M23LCV1024_COUNT];
  volatile uint8_t* m_csPort[MAX_M23LCV1024_COUNT];
  //----------------------------------------------------------------------------
#endif  // SRAM_USE_SPI_LIB

  uint8_t m_chipCount;
  uint8_t m_curChip;
};
#endif  // M23LCV1024_h

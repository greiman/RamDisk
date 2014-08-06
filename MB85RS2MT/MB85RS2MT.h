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
#ifndef MB85RS2MT_h
#define MB85RS2MT_h
/**
 * \file
 * MB85RS2MT Arduino Library
 */
#include <Arduino.h>
#include <RamBaseDevice.h>
#ifdef __AVR__
#include <T_MB85RS2MT.h>
#endif  // __AVR__
//------------------------------------------------------------------------------
/** Set FRAM_USE_SPI_LIB non-zero to use the Arduino SPI library */
#define FRAM_USE_SPI_LIB !defined(__AVR__)

/** Maximum number of chips supported. */
const uint8_t MAX_MB85RS2MT_COUNT = 8;

/**
 * \class MB85RS2MT
 * \brief MB85RS2MT raw I/O library for a multiple chips.
 */
class MB85RS2MT : public RamBaseDevice {
 public:
  /** Constructor */
  MB85RS2MT() : m_chipCount(0) {}
  /** Initialize MB85RS2MT chips.
   * \param[in] csPin array of chip select pin numbers.
   * \param[in] chipCount number of chips to use.
   * \return false if too many chips else true.
   */
  bool begin(uint8_t* csPin, uint8_t chipCount);

  /** Initialize one MB85RS2MT.
   * \param[in] csPin RAM chip select pin.
   * \return true.
   */
  bool begin(uint8_t csPin) {return begin(&csPin, 1);}

  /** Read a block from the MB85RS2MT.
   * \param[in] address start location in the MB85RS2MT.
   * \param[out] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return true unless address is out of range.
   */
  bool read(uint32_t address, void *buf, size_t nbyte);

  uint32_t sizeBlocks() {return 512*m_chipCount;}

  /** Write a block to the MB85RS2MT.
   * \param[in] address start location in the MB85RS2MT.
   * \param[in] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return true unless address is out of range.
   */
  bool write(uint32_t address, const void *buf, size_t nbyte);

 private:
  //----------------------------------------------------------------------------
  static const uint8_t MB85RS_READ  = 0X03;
  static const uint8_t MB85RS_WREN  = 0X06;
  static const uint8_t MB85RS_WRITE = 0X02;
  static const uint8_t MB85RS_RDSR  = 0X05;
  static const uint8_t MB85RS_WRSR  = 0X01;

  bool sendCmdAddress(uint8_t cmd, uint32_t address);
#if FRAM_USE_SPI_LIB
  void csLow() {digitalWrite(m_csPin[m_curChip], LOW);}
  void csHigh() {digitalWrite(m_csPin[m_curChip], HIGH);}
  uint8_t m_csPin[MAX_MB85RS2MT_COUNT];
#else  // FRAM_USE_SPI_LIB
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
  uint8_t m_csBit[MAX_MB85RS2MT_COUNT];
  volatile uint8_t* m_csPort[MAX_MB85RS2MT_COUNT];
  //----------------------------------------------------------------------------
#endif  // FRAM_USE_SPI_LIB
  uint8_t m_chipCount;
  uint8_t m_curChip;
};
#endif  // MB85RS2MT_h

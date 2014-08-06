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
#ifndef T_MB85RS2MT_h
#define T_MB85RS2MT_h
/**
 * \file
 * MB85RS2MT Arduino Library
 */
#include <Arduino.h>
#include <utility/DigitalPin.h>
#include <RamBaseDevice.h>
/**
 * \class T_MB85RS2MT
 * \brief MB85RS2MT template class for a single FRAM chip.
 */
template<uint8_t ChipSelectPin>
class T_MB85RS2MT : public RamBaseDevice {

 public:
  //----------------------------------------------------------------------------
  /** Initialize the MB85RS2MT. */
  void begin() {
    spiBegin();
  }
  //----------------------------------------------------------------------------
  /** Read a block from the MB85RS2MT.
   * \param[in] address start location in the MB85RS2MT.
   * \param[out] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return Always returns true.
   */
//  inline __attribute__((always_inline))
  bool read(uint32_t address, void *buf, size_t nbyte) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
    sendCmdAddress(MB85RS_READ, address);
    spiReceive(dst, nbyte);
    cspin.high();
    return true;
  }
  //----------------------------------------------------------------------------
  /** Read FRAM status register.
   *
   * \return Value read from the status register.
   */
  uint8_t readStatus() {
    spiInit();
    cspin.low();
    spiSend(MB85RS_RDSR);
    uint8_t rtn = spiReceive();
    cspin.high();
    return rtn;
  }
  //----------------------------------------------------------------------------
  uint32_t sizeBlocks() {return 512;}
  //----------------------------------------------------------------------------
  /** Write FRAM status register.
   *
   * \param[in] value data to be written.
   */
  void writeStatus(uint8_t value) {
    spiInit();
    cspin.low();
    spiSend(MB85RS_WREN);
    cspin.high();
    cspin.low();
    spiSend(MB85RS_WRSR);
    spiSend(value);
    cspin.high();
  }
  //----------------------------------------------------------------------------
  /** Write a block to the MB85RS2MT.
   * \param[in] address start location in the MB85RS2MT.
   * \param[in] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return Always returns true.
   */
//  inline __attribute__((always_inline))
  bool write(uint32_t address, const void *buf, size_t nbyte) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
    sendCmdAddress(MB85RS_WRITE, address);
    spiSend(src, nbyte);
    cspin.high();
    return true;
  }

 private:
  //----------------------------------------------------------------------------
  static const uint8_t MB85RS_READ  = 0X03;
  static const uint8_t MB85RS_WREN  = 0X06;
  static const uint8_t MB85RS_WRITE = 0X02;
  static const uint8_t MB85RS_RDSR  = 0X05;
  static const uint8_t MB85RS_WRSR  = 0X01;
  //----------------------------------------------------------------------------
  DigitalPin<ChipSelectPin> cspin;
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void sendCmdAddress(uint8_t cmd, uint32_t address) {
    spiInit();
    cspin.low();
    if (cmd == MB85RS_WRITE) {
      spiSend(MB85RS_WREN);
      cspin.high();
      cspin.low();
    }
    spiSend(cmd);
    spiSend(address >> 16);
    spiSend(address >> 8);
    spiSend(address);
  }
  //----------------------------------------------------------------------------
  void spiBegin() {
     // set SS high - may be chip select for another SPI device
    fastDigitalWrite(SS, HIGH);

    // SS must be in output mode even it is not chip select
    fastPinMode(SS, OUTPUT);
    fastPinMode(MISO, INPUT);
    fastPinMode(MOSI, OUTPUT);
    fastPinMode(SCK, OUTPUT);
    cspin.mode(OUTPUT);
    cspin.high();
  }
  //----------------------------------------------------------------------------
  void spiInit() {
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = 1 << SPI2X;
  }
  //----------------------------------------------------------------------------
  uint8_t spiReceive() {
    SPDR = 0XFF;
    while (!(SPSR & (1 << SPIF))) {}
    return SPDR;
  }
  //----------------------------------------------------------------------------
  void spiReceive(uint8_t* buf, size_t n) {
    if (n-- == 0) return;
    SPDR = 0XFF;
    for (size_t i = 0; i < n; i++) {
      while (!(SPSR & (1 << SPIF))) {}
      uint8_t b = SPDR;
      SPDR = 0XFF;
      buf[i] = b;
    }
    while (!(SPSR & (1 << SPIF))) {}
    buf[n] = SPDR;
  }
  //----------------------------------------------------------------------------
  void spiSend(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF))) {}
  }
  //----------------------------------------------------------------------------
  void spiSend(const uint8_t* buf , size_t n) {
    if (n == 0) return;
    SPDR = buf[0];
    if (n > 1) {
      uint8_t b = buf[1];
      size_t i = 2;
      while (1) {
        while (!(SPSR & (1 << SPIF))) {}
        SPDR = b;
        if (i == n) break;
        b = buf[i++];
      }
    }
    while (!(SPSR & (1 << SPIF))) {}
  }
};
#endif  // T_MB85RS2MT_h

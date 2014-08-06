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
#ifndef T23LCV1024_h
#define T23LCV1024_h
/**
 * \file
 * 23LCV1024 Arduino Library
 */
#include <Arduino.h>
#include <utility/DigitalPin.h>
#include <RamBaseDevice.h>
/**
 * \class T23LCV1024
 * \brief 23LCV1024 template class for a single chip.
 */
template<uint8_t ChipSelectPin>
class T23LCV1024 : public RamBaseDevice {
 public:
  //----------------------------------------------------------------------------
  /** Initialize the 23LCV1024. */
  void begin() {
    // set SS high - may be chip select for another SPI device
    fastDigitalWrite(SS, HIGH);

    // SS must be in output mode even it is not chip select
    fastPinMode(SS, OUTPUT);
    fastPinMode(MISO, INPUT);
    fastPinMode(MOSI, OUTPUT);
    fastPinMode(SCK, OUTPUT);
    cspin.mode(OUTPUT);
    // Set sequential mode.
    spiInit();
    cspin.low();
    spiSend(WRITE_MODE);
    spiSend(SEQ_MODE);
    cspin.high();
  }
  //----------------------------------------------------------------------------
  /** Read a block from the 23LCV1024.
   * \param[in] address start location in the 23LCV1024.
   * \param[out] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return Always returns true.
   */
//  inline __attribute__((always_inline))
  bool read(uint32_t address, void *buf, size_t nbyte) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
    sendCmdAddress(READ_DATA, address);
    spiReceive(dst, nbyte);
    cspin.high();
    return true;
  }
  //----------------------------------------------------------------------------
  uint32_t sizeBlocks() {return 256;}
  //----------------------------------------------------------------------------
  /** Write a block to the 23LCV1024.
   * \param[in] address start location in the 23LCV1024.
   * \param[in] buf location in Arduino SRAM for the transfer.
   * \param[in] nbyte number of bytes to transfer.
   * \return Always returns true.
   */
//  inline __attribute__((always_inline))
  bool write(uint32_t address, const void *buf, size_t nbyte) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
    sendCmdAddress(WRITE_DATA, address);
    spiSend(src, nbyte);
    cspin.high();
    return true;
  }

 private:
  //----------------------------------------------------------------------------
  static const uint8_t READ_DATA = 0X03;
  static const uint8_t WRITE_DATA = 0X02;
  static const uint8_t WRITE_MODE = 0X01;
  static const uint8_t SEQ_MODE = 0X40;
  //----------------------------------------------------------------------------
  DigitalPin<ChipSelectPin> cspin;
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void sendCmdAddress(uint8_t cmd, uint32_t address) {
    spiInit();
    cspin.low();
    spiSend(cmd);
    spiSend(address >> 16);
    spiSend(address >> 8);
    spiSend(address);
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
#endif  // T23LCV1024_h

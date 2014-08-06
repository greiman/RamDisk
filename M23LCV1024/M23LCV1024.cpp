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
#include <M23LCV1024.h>
//==============================================================================
#if SRAM_USE_SPI_LIB
#include <SPI.h>
//------------------------------------------------------------------------------
inline void spiInit() {
  SPI.setDataMode(SPI_MODE0);
#ifdef __AVR__
  SPI.setClockDivider(SPI_CLOCK_DIV2);
#else  // __AVR__
  const uint8_t ARM_SPI_DIVIDER = 3;  ///////////////////////////////////////
  SPI.setClockDivider(ARM_SPI_DIVIDER);
#endif  // __AVR__
}
//------------------------------------------------------------------------------
inline __attribute__((always_inline))
uint8_t spiReceive() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
static void spiReceive(uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    buf[i] = SPI.transfer(0XFF);
  }
}
//------------------------------------------------------------------------------
inline __attribute__((always_inline))
void spiSend(uint8_t data) {
  SPI.transfer(data);
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf , size_t n) {
  for (size_t i = 0; i < n; i++) {
    SPI.transfer(buf[i]);
  }
}
//==============================================================================
#else  // SRAM_USE_SPI_LIB
#include <DigitalPin.h>
//------------------------------------------------------------------------------
inline void spiInit() {
  SPCR = (1 << SPE) | (1 << MSTR);
  SPSR = 1 << SPI2X;
}
//------------------------------------------------------------------------------
inline __attribute__((always_inline))
uint8_t spiReceive() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF))) {}
  return SPDR;
}
//------------------------------------------------------------------------------
static void spiReceive(uint8_t* buf, size_t n) {
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
//------------------------------------------------------------------------------
inline __attribute__((always_inline))
void spiSend(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF))) {}
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf , size_t n) {
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
#endif  // SRAM_USE_SPI_LIB
//==============================================================================
bool M23LCV1024::begin(uint8_t* csPin, uint8_t chipCount) {
  if (chipCount > MAX_M23LCV1024_COUNT) return false;
  m_chipCount = chipCount;

#if SRAM_USE_SPI_LIB
  SPI.begin();
  for (uint8_t i = 0; i < chipCount; i++) {
    m_csPin[i] = csPin[i];
    pinMode(csPin[i], OUTPUT);
    digitalWrite(csPin[i], HIGH);
  }
#else   // SRAM_USE_SPI_LIB
  // set SS high - may be chip select for another SPI device
  digitalWrite(SS, HIGH);
  // SS must be in output mode even it is not chip select
  pinMode(SS, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  // Make sure all chips have CS high.
  for (uint8_t i = 0; i < chipCount; i++) {
    pinMode(csPin[i], OUTPUT);
    digitalWrite(csPin[i], HIGH);
    m_csBit[i] = digitalPinToBitMask(csPin[i]);
    m_csPort[i] = portOutputRegister(digitalPinToPort(csPin[i]));
  }
#endif  // SRAM_USE_SPI_LIB
  // Set mode for all chips.
  for (int m_curChip = 0; m_curChip < m_chipCount; m_curChip++) {
    // Set sequential mode.
    csLow();
    spiInit();
    spiSend(WRITE_MODE);
    spiSend(SEQ_MODE);
    csHigh();
  }
}
//------------------------------------------------------------------------------
bool M23LCV1024::read(uint32_t address, void *buf, size_t nbyte) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  if (!sendCmdAddress(READ_DATA, address)) return false;
  spiReceive(dst, nbyte);
  csHigh();
  return true;
}
//------------------------------------------------------------------------------
bool M23LCV1024::sendCmdAddress(uint8_t cmd, uint32_t address) {
  m_curChip = address >> 17;
  if (m_curChip >= m_chipCount) return false;
  csLow();
  spiInit();
  spiSend(cmd);
  spiSend(address >> 16);
  spiSend(address >> 8);
  spiSend(address);
  return true;
}
//------------------------------------------------------------------------------
bool M23LCV1024::write(uint32_t address, const void *buf, size_t nbyte) {
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
  if (!sendCmdAddress(WRITE_DATA, address)) return false;
  spiSend(src, nbyte);
  csHigh();
  return true;
}

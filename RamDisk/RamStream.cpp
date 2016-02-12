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
 /**
 * \file
 * RamStream implementation
 */
#include <RamStream.h>
//==============================================================================
// fmtDec is based on divu10() code derived from the book Hackers Delight1
// in the robtillaart Arduino forum post:
// http://forum.arduino.cc/index.php?topic=167414.msg1246851#msg1246851
// Also see: http://www.hackersdelight.org/hdcodetxt/divuc.c.txt
//------------------------------------------------------------------------------
// Format 16-bit unsigned
static char* fmtDec(uint16_t n, char* p) {
  while (n > 9) {
    uint16_t t = n;
    n = (n >> 1) + (n >> 2);
    n = n + (n >> 4);
    n = n + (n >> 8);
    // n = n + (n >> 16);  // no code for 16-bit n
    n = n >> 3;
    uint16_t r = t - (((n << 2) + n) << 1);
    if (r > 9) {
      n++;
      r -= 10;
    }
    *--p = r + '0';
  }
  *--p = n + '0';
  return p;
}
//------------------------------------------------------------------------------
// format 32-bit unsigned
static char* fmtDec(uint32_t n, char* p) {
//  while (n > 0XFFFF) {
  while (n >> 16) {
    uint32_t t = n;
    n = (n >> 1) + (n >> 2);
    n = n + (n >> 4);
    n = n + (n >> 8);
    n = n + (n >> 16);
    n = n >> 3;
    uint8_t r = t - (((n << 2) + n) << 1);
    if (r > 9) {
      n++;
      r -= 10;
    }
    *--p = r + '0';
  }
  return fmtDec((uint16_t)n, p);
}
//------------------------------------------------------------------------------
static char* fmtFloat(float value, uint8_t prec, char* p) {
  char sign = 0;
  if (value < 0) {
    value = -value;
    sign = '-';
  }
  // check for NaN INF OVF
  if (isnan(value)) {
    p = reinterpret_cast<char*>(memcpy_P(p - 3, PSTR("nan"), 3));
  } else if (isinf(value)) {
    p = reinterpret_cast<char*>(memcpy_P(p - 3, PSTR("inf"), 3));
  } else if (value > 4294967040.0) {
    p = reinterpret_cast<char*>(memcpy_P(p - 3, PSTR("ovf"), 3));
  } else {
    if (prec > 9) prec = 9;
    uint32_t s = 1;
    for (uint8_t i = 0; i < prec; i++) {
      // s *= 10;
      s = ((s << 2) + s) << 1;
    }
    // round value
    value += 0.5/s;
    uint32_t whole = value;

    if (prec) {
      char* tmp = p - prec;
      uint32_t fraction = s*(value - whole);
      p = fmtDec(fraction, p);
      while (p > tmp) *--p = '0';
      *--p = '.';
    }
    p = fmtDec(whole, p);
    if (sign) *--p = sign;
  }
  return p;
}
//------------------------------------------------------------------------------
static char* fmtHex(uint32_t n, char* p) {
  do {
    uint8_t h = n & 0XF;
    *--p = h + (h < 10 ? '0' : 'A' - 10);
    n >>= 4;
  } while (n);
  return p;
}
//==============================================================================
//------------------------------------------------------------------------------
int RamStream::fclose() {
  int rtn = 0;
  if (!m_flags) {
    return EOF;
  }
  if (m_flags & F_SWR) {
    if (!flushBuf()) rtn = EOF;
  }
  if (!RamBaseFile::close()) rtn = EOF;
  m_r = 0;
  m_w = 0;
  m_flags = 0;
  return rtn;
}
//------------------------------------------------------------------------------
int RamStream::fflush() {
  if ((m_flags & (F_SWR | F_SRW)) && !(m_flags & F_SRD)) {
    if (flushBuf() && RamBaseFile::sync()) return 0;
  }
  return EOF;
}
//------------------------------------------------------------------------------
char* RamStream::fgets(char* str, int num, size_t* len) {
  char* s = str;
  size_t n;
  if (num-- <= 0) return 0;
  while (num) {
    if ((n = m_r) == 0) {
      if (!fillBuf()) {
        if (s == str) return 0;
        break;
      }
      n = m_r;
    }
    if (n > num) n = num;
    uint8_t* end =  reinterpret_cast<uint8_t*>(memchr(m_p, '\n', n));
    if (end != 0) {
      n = ++end - m_p;
      memcpy(s, m_p, n);
      m_r -= n;
      m_p = end;
      s += n;
      break;
    }
    memcpy(s, m_p, n);
    m_r -= n;
    m_p += n;
    s += n;
    num -= n;
  }
  *s = 0;
  if (len) *len = s - str;
  return str;
}
//------------------------------------------------------------------------------
bool RamStream::fopen(const char* filename, const char* mode) {
  uint8_t oflags;
  switch (*mode++) {
  case 'a':
    m_flags = F_SWR;
    oflags = O_WRITE | O_CREAT | O_APPEND | O_AT_END;
    break;

  case 'r':
    m_flags = F_SRD;
    oflags = O_READ;
    break;

  case 'w':
    m_flags = F_SWR;
    oflags = O_WRITE | O_CREAT | O_TRUNC;
    break;

  default:
    goto fail;
  }
  while (*mode) {
    switch (*mode++) {
    case '+':
      m_flags |= F_SRW;
      oflags |= O_RDWR;
      break;

    case 'b':
      break;

    case 'x':
      oflags |= O_EXCL;
      break;

    default:
      goto fail;
    }
  }
  if ((oflags & O_EXCL) && !(oflags & O_WRITE)) goto fail;
  if (!RamBaseFile::open(filename, oflags)) goto fail;
  m_r = 0;
  m_w = 0;
  m_p = m_buf;
  return true;

 fail:
  m_flags = 0;
  return false;
}
//------------------------------------------------------------------------------
int RamStream::fputs(const char* str) {
  size_t len = strlen(str);
  return fwrite(str, 1, len) == len ? len : EOF;
}
//------------------------------------------------------------------------------
int RamStream::fputs_P(PGM_P str) {
  PGM_P bgn = str;
  for (char c; (c = pgm_read_byte(str)); str++) {
    if (putc(c) < 0) return EOF;
  }
  return str - bgn;
}
//------------------------------------------------------------------------------
size_t RamStream::fread(void* ptr, size_t size, size_t count) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(ptr);
  size_t total = size*count;
  if (total == 0) return 0;
  size_t need = total;
  while (need > m_r) {
    memcpy(dst, m_p, m_r);
    dst += m_r;
    m_p += m_r;
    need -= m_r;
    if (!fillBuf()) {
      return (total - need)/size;
    }
  }
  memcpy(dst, m_p, need);
  m_r -= need;
  m_p += need;
  return count;
}
//------------------------------------------------------------------------------
int RamStream::fseek(int32_t offset, int origin) {
  int32_t pos;
  if (m_flags & F_SWR) {
    if (!flushBuf()) {
      goto fail;
    }
  }
  switch (origin) {
  case SEEK_CUR:
    pos = ftell();
    if (pos < 0) {
      goto fail;
    }
    pos += offset;
    if (!RamBaseFile::seekCur(pos)) {
      goto fail;
    }
    break;

  case SEEK_SET:
    if (!RamBaseFile::seekSet(offset)) {
      goto fail;
    }
    break;

  case SEEK_END:
    if (!RamBaseFile::seekEnd(offset)) {
      goto fail;
    }
    break;

  default:
    goto fail;
  }
  m_r = 0;
  m_p = m_buf;
  return 0;

 fail:
  return EOF;
}
//------------------------------------------------------------------------------
int32_t RamStream::ftell() {
  uint32_t pos = RamBaseFile::curPosition();
  if (m_flags & F_SRD) {
    if (m_r > pos) return -1L;
    pos -= m_r;
  } else if (m_flags & F_SWR) {
    pos += m_p - m_buf;
  }
  return pos;
}
//------------------------------------------------------------------------------
size_t RamStream::fwrite(const void* ptr, size_t size, size_t count) {
  return write(ptr, count*size) < 0 ? EOF : count;  //////////////////////////////////////////

  const uint8_t* src = reinterpret_cast<const uint8_t*>(ptr);
  size_t total = count*size;
  if (total == 0) return 0;
  size_t todo = total;

  while (todo > m_w) {
    memcpy(m_p, src, m_w);
    m_p += m_w;
    src += m_w;
    todo -= m_w;
    if (!flushBuf()) {
      return (total - todo)/size;
    }
  }
  memcpy(m_p, src, todo);
  m_p += todo;
  m_w -= todo;
  return count;
}
//------------------------------------------------------------------------------
int RamStream::write(const void* buf, size_t count) {
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
  size_t todo = count;

  while (todo > m_w) {
    memcpy(m_p, src, m_w);
    m_p += m_w;
    src += m_w;
    todo -= m_w;
    if (!flushBuf()) return EOF;
  }
  memcpy(m_p, src, todo);
  m_p += todo;
  m_w -= todo;
  return count;
}
//------------------------------------------------------------------------------
size_t RamStream::print(const __FlashStringHelper *str) {
  const char PROGMEM *p = (const char PROGMEM *)str;
  uint8_t c;
  while (c = pgm_read_byte(p)) {
    if (putc(c) < 0) return 0;
    p++;
  }
  return p - (const char PROGMEM *)str;
}
//------------------------------------------------------------------------------
int RamStream::printDec(float value, uint8_t prec) {
//  #define NEW_WAY
#ifdef NEW_WAY
  char buf[24];
  char *ptr = fmtFloat(value, prec, buf + sizeof(buf));
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
#else
  char* ptr;
  uint8_t rtn = 0;
  uint8_t sign = 0;
  if (value < 0) {
    value = -value;
    sign = '-';
  }
  // check for NaN INF OVF
  if (isnan(value)) {
    if (fputs_P(PSTR("nan")) < 0) return -1;
    rtn += 3;
  } else if (isinf(value)) {
    if (fputs_P(PSTR("inf")) < 0) return -1;
    rtn += 3;
  } else if (value > 4294967040.0) {
    if (fputs_P(PSTR("ovf")) < 0) return -1;
    rtn += 3;
  } else {
    if (sign) {
     if (putc(sign) < 0) return -1;
      rtn++;
    }
    if (prec > 9) prec = 9;
    uint32_t s = 1;
    for (uint8_t i = 0; i < prec; i++) {
      // s *= 10;
      s = ((s << 2) + s) << 1;
    }
    // round value
    value += 0.5/s;
    uint32_t whole = value;
    int np;
    if ((np = printDec(whole)) < 0) return -1;
    rtn += np;
    if (prec) {
      if (putc('.') < 0) return -1;
      char* str = fmtSpace(prec);
      if (!str) return -1;
      char* tmp = str - prec;
      uint32_t fraction = s*(value - whole);

      ptr = fmtDec(fraction, str);
      while (ptr > tmp) *--ptr = '0';
      rtn += prec + 1;
    }
  }
  return rtn;
  #endif
}
//------------------------------------------------------------------------------
int RamStream::printDec(int8_t n) {
  uint8_t s = 0;
  if (n < 0) {
    if (fputc('-') < 0) return -1;
    n = -n;
    s = 1;
  }
  printDec((uint8_t)n);
}
//------------------------------------------------------------------------------
int RamStream::printDec(int16_t n) {
  int s;
  uint8_t rtn = 0;
  if (n < 0) {
    if (fputc('-') < 0) return -1;
    n = -n;
    rtn++;
  }
  if ((s = printDec((uint16_t)n)) < 0) return s;
  return rtn;
}
//------------------------------------------------------------------------------
int RamStream::printDec(uint16_t n) {
//  #define NEW_WAY
#ifdef NEW_WAY
  char buf[5];
  char *ptr = fmtDec(n, buf + sizeof(buf));
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
#else
  uint8_t len;
  if (n < 100) {
    len = n < 10 ? 1 : 2;
  } else {
    len = n < 1000 ? 3 : n < 10000 ? 4 : 5;
  }
  char* str = fmtSpace(len);
  if (!str) return -1;
  fmtDec(n, str);
  return len;
#endif
}
//------------------------------------------------------------------------------
int RamStream::printDec(int32_t n) {
  uint8_t s = 0;
  if (n < 0) {
    if (fputc('-') < 0) return -1;
    n = -n;
    s = 1;
  }
  int rtn = printDec((uint32_t)n);
  return rtn > 0 ? rtn + s : -1;
}
//------------------------------------------------------------------------------
int RamStream::printDec(uint32_t n) {
#define NEW_WAY
#ifdef NEW_WAY
  char buf[10];
  char *ptr = fmtDec(n, buf + sizeof(buf));
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
#else
  uint8_t len;
  if (n < 0X10000) {
    return printDec((uint16_t)n);
  }
  if (n < 10000000) {
    len = n < 100000 ? 5 : n < 1000000 ? 6 : 7;
  } else {
    len = n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
  }

  char* str = fmtSpace(len);
  if (!str) return -1;
  fmtDec(n, str);
  return len;
#endif
}
//------------------------------------------------------------------------------
int RamStream::printHex(uint32_t n) {
#define NEW_WAY
#ifdef NEW_WAY
  char buf[8];
  char *ptr = fmtHex(n, buf + sizeof(buf));
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
#else  // NEW_WAY
  size_t len;
  if (n < 0X10000) {
    len = n < 0X10 ? 1 : n < 0X100 ? 2 : n < 0X1000 ? 3 : 4;
  } else {
    len = n < 0X100000 ? 5 : n < 0X1000000 ? 6 : n < 0X10000000 ? 7 : 8;
  }
  char* str = fmtSpace(len);
  if (!str) return -1;

  do {
    uint8_t h = n & 0XF;
    *str-- = h + (h < 10 ? '0' : 'A' - 10);
    n >>= 4;
  } while (n);
  return len;
#endif  // NEW_WAY
}
//------------------------------------------------------------------------------
bool RamStream::rewind() {
  if (m_flags & F_SWR) {
    if (!flushBuf()) return false;
  }
  RamBaseFile::seekSet(0);
  m_r = 0;
  return true;
}
//------------------------------------------------------------------------------
int RamStream::ungetc(int c) {
  // error if EOF.
  if (c == EOF) return EOF;
  // error if not reading.
  if ((m_flags & F_SRD) == 0) return EOF;
  // error if no space.
  if (m_p == m_buf) return EOF;
  m_r++;
  m_flags &= ~F_EOF;
  return *--m_p = (uint8_t)c;
}
//==============================================================================
// private
//------------------------------------------------------------------------------
int RamStream::fillGet() {
  if (!fillBuf()) {
    return EOF;
  }
  m_r--;
  return *m_p++;
}
//------------------------------------------------------------------------------
// private
bool RamStream::fillBuf() {
  if (!(m_flags & F_SRD)) {   /////////////check for F_ERR and F_EOF ??/////////////////
    if (!(m_flags & F_SRW)) {
      m_flags |= F_ERR;
      return false;
    }
    if (m_flags & F_SWR) {
      if (!flushBuf()) {
        return false;
      }
      m_flags &= ~F_SWR;
      m_flags |= F_SRD;
      m_w = 0;
    }
  }
  m_p = m_buf + RAM_UNGETC_BUF_SIZE;
  int nr = RamBaseFile::read(m_p, sizeof(m_buf) - RAM_UNGETC_BUF_SIZE);
  if (nr <= 0) {
    m_flags |= nr < 0 ? F_ERR : F_EOF;
    m_r = 0;
    return false;
  }
  m_r = nr;
  return true;
}
//------------------------------------------------------------------------------
// private
bool RamStream::flushBuf() {
  if (!(m_flags & F_SWR)) {   /////////////////check for F_ERR ??////////////////////////
    if (!(m_flags & F_SRW)) {
      m_flags |= F_ERR;
      return false;
    }
    m_flags &= ~F_SRD;
    m_flags |= F_SWR;
    m_r = 0;
    m_w = sizeof(m_buf);
    m_p = m_buf;
    return true;
  }
  uint8_t n = m_p - m_buf;
  m_p = m_buf;
  m_w = sizeof(m_buf);
  if (RamBaseFile::write(m_buf, n) == n) return true;
  m_flags |= F_ERR;
  return false;
}
//------------------------------------------------------------------------------
int RamStream::flushPut(uint8_t c) {
  if (!flushBuf()) return EOF;
  m_w--;
  return *m_p++ = c;
}
//------------------------------------------------------------------------------
char* RamStream::fmtSpace(uint8_t len) {
  if (m_w < len) {
    if (!flushBuf() || m_w < len) {
      return 0;
    }
  }
  if (len > m_w) return 0;
  m_p += len;
  m_w -= len;
  return reinterpret_cast<char*>(m_p);
}

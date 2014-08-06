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
#include <RamFile.h>
//------------------------------------------------------------------------------
/** Write data to an open file.
 *
 * \note Data is moved to the cache but may not be written to the
 * storage device until sync() is called.
 *
 * \param[in] buf Pointer to the location of the data to be written.
 *
 * \param[in] nbyte Number of bytes to write.
 *
 * \return For success write() returns the number of bytes written, always
 * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
 * include write() is called before a file has been opened, write is called
 * for a read-only file, device is full, a corrupt file system or an I/O error.
 *
 */
int RamFile::write(const void* buf, size_t nbyte) {
  return RamBaseFile::write(buf, nbyte);
}
//------------------------------------------------------------------------------
/** Write a byte to a file. Required by the Arduino Print class.
 * \param[in] b the byte to be written.
 * Use getWriteError to check for errors.
 * \return 1 for success and 0 for failure.
 */
size_t RamFile::write(uint8_t b) {
  return RamBaseFile::write(&b, 1) == 1 ? 1 : 0;
}
//------------------------------------------------------------------------------
/** Write a string to a file. Used by the Arduino Print class.
 * \param[in] str Pointer to the string.
 * Use getWriteError to check for errors.
 * \return count of characters written for success or -1 for failure.
 */
int RamFile::write(const char* str) {
  return RamBaseFile::write(str, strlen(str));
}
//------------------------------------------------------------------------------
/** Write a PROGMEM string to a file.
 * \param[in] str Pointer to the PROGMEM string.
 * Use getWriteError to check for errors.
 */
void RamFile::write_P(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) write(c);
}
//------------------------------------------------------------------------------
/** Write a PROGMEM string followed by CR/LF to a file.
 * \param[in] str Pointer to the PROGMEM string.
 * Use getWriteError to check for errors.
 */
void RamFile::writeln_P(PGM_P str) {
  write_P(str);
  write_P(PSTR("\r\n"));
}

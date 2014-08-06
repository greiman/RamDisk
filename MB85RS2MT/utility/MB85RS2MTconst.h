#ifndef MB85RS2MTconst_h
#define MB85RS2MTconst_h
//------------------------------------------------------------------------------
// Status register
/** Bit 7 WPEN - Status Register Write Protect.
 *
 * This is a bit composed of nonvolatile memories (FRAM). WPEN protects
 * writing to a status register (refer to “¦ WRITING PROTECT”) relating with
 * WP input. Writing with the WRSR command and reading with the RDSR
 * command are possible.
 */
/** Bit 6, 5, and 4 - Not used Bits.
 *
 * These are bits composed of nonvolatile memories, writing with the WRSR
 * command is possible. These bits are not used but they are read with the
 * RDSR command. 
 */
/** Bit 3 and 2 - Block Protect.
 * This is a bit composed of nonvolatile memory. This defines size of write
 * protect block for the WRITE command (refer to “¦ BLOCK PROTECT”).
 * Writing with the WRSR command and reading with the RDSR command
 * are possible.
 */
/** Bit 1 - Write Enable Latch.
 *
 * This indicates FRAM Array and status register are writable. The WREN
 * command is for setting, and the WRDI command is for resetting. With the
 * RDSR command, reading is possible but writing is not possible with the
 * WRSR command. WEL is reset after the following operations.
 *   After power ON.
 *   After WRDI command recognition.
 *   The rising edge of CS after WRSR command recognition.
 *   The rising edge of CS after WRITE command recognition. 
 */
/** Bit 0 - This bit is fixed to "0". */
//------------------------------------------------------------------------------
/** Set Write Enable Latch.
 * 
 * The WREN command sets WEL (Write Enable Latch) . WEL has to be set with
 * the WREN command before writing operation (WRSR command and WRITE command).
 * WREN command is applicable to “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_WREN = 0X06;

/** Reset Write Enable Latch.
 *
 * The WRDI command resets WEL (Write Enable Latch). Writing operation
 * (WRSR command and WRITE command) are not performed when WEL is reset.
 * WRDI command is applicable to “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_WRDI = 0X04;

/** Read Status Register. 
 *
 * The RDSR command reads status register data. After op-code of RDSR is
 * input to SI, 8-cycle clock is input to SCK. The SI value is invalid for
 * this time. SO is output synchronously to a falling edge of SCK. In the
 * RDSR command, repeated reading of status register is enabled by sending
 * SCK continuously before rising of CS. RDSR command is applicable to
 * “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_RDSR = 0X05;

/** Write Status Register. 
 *
 * The WRSR command writes data to the nonvolatile memory bit of status
 * register. After performing WRSR op-code to a SI pin, 8 bits writing data
 * is input. WEL (Write Enable Latch) is not able to be written with WRSR
 * command. A SI value correspondent to bit 1 is ignored. Bit 0 of the
 * status register is fixed to “0” and cannot be written. The SI value
 * corresponding to bit 0 is ignored. WP signal level shall be fixed before
 * performing WRSR command, and do not change the WP signal level until
 * the end of command sequence. WRSRcommand is applicable to
 * “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_WRSR = 0X01;

/** Read Memory Code.
 * 
 * The READ command reads FRAM memory cell array data. Arbitrary 24 bits
 * address and op-code of READ are input to SI. The 6-bit upper address bit
 * is invalid. Then, 8-cycle clock is input to SCK. SO is output
 * synchronously to the falling edge of SCK. While reading, the SI value
 * is invalid. When CS is risen, the READ command is completed, but keeps
 * on reading with automatic address increment which is enabled by
 * continuously sending clocks to SCK in unit of 8 cycles before CS rising.
 * When it reaches the most significant address, it rolls over to the
 * starting address, and reading cycle keeps on infinitely. READ command is
 * applicable to “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_READ = 0X03;

/** Write Memory Code. 
 *
 * The WRITE command writes data to FRAM memory cell array. WRITE op-code,
 * arbitrary 24 bits of address and 8 bits of writing data are input to SI.
 * The 6-bit upper address bit is invalid. When 8 bits of writing data is
 * input, data is written to FRAM memory cell array. Risen CS will terminate
 * the WRITE command, but if you continue sending the writing data for 8
 * bits each before CS rising, it is possible to continue writing with
 * automatic address increment. When it reaches the most significant address,
 * it rolls over to the starting address, and writing cycle can be continued
 * infinitely. WRITE command is applicable to “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_WRITE = 0X02;

/** Read Device ID. 
 *
 * The RDID command reads fixed Device ID. After performing RDID op-code to
 * SI, 32-cycle clock is input to SCK. The SI value is invalid for this time.
 * SO is output synchronously to a falling edge of SCK. The output is in
 * order of Manufacturer ID (8bit)/Continuation code (8bit)/Product ID
 * 1st Byte)/Product ID (2nd Byte). In the RDID command, SO holds the output
 * state of the last bit in 32-bit Device ID until CS is risen. RDID command
 * is applicable to “Up to 25 MHz operation”.
 */
const uint8_t MB85RS_RDID = 0X9F;

/** Fast Read Memory Code.
 *
 * The FSTRD command reads FRAM memory cell array data. Arbitrary 24 bits
 * address and op-code of FSTRD are input to SI followed by 8 bits dummy.
 * The 6-bit upper address bit is invalid. Then, 8-cycle clock is input to
 * SCK. SO is output synchronously to the falling edge of SCK. While reading,
 * the SI value is invalid. When CS is risen, the FSTRD command is completed,
 * but keeps on reading with automatic address increment which is enabled by
 * continuously sending clocks to SCK in unit of 8 cycles before CS rising.
 * When it reaches the most significant address, it rolls over to the starting
 * address, and reading cycle keeps on infinitely. FSTRD command is applicable
 * to “Up to 25 MHz (1.8 V to 2.7 V) and 40 MHz (2.7 V to 3.6 V) operation”.
 */
const uint8_t MB85RS_FSTRD = 0X0B;

/** Sleep Mode. 
 *
 * The SLEEP command shifts the LSI to a low power mode called “SLEEP mode”.
 * The transition to the SLEEP mode is carried out at the rising edge of CS
 * after operation code in the SLEEP command. However, when at least one SCK
 * clock is inputted before the rising edge of CS after operation code in
 * the SLEEP command, this SLEEP command is canceled.  After the SLEEP mode
 * transition, SCK and SI inputs are ignored and SO changes to a Hi-Z state.
 */
const uint8_t MB85RS_SLEEP = 0XB9;

#endif  //MB85RS2MTconst_h
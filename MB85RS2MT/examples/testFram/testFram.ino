#include <RamDisk.h>
#include <MB85RS2MT.h>

uint8_t buf[64];
T_MB85RS2MT<9> fram;

//------------------------------------------------------------------------------
void printHex(uint8_t val) {
  if (val < 16) Serial.write('0');
  Serial.print(val, HEX);
}
//------------------------------------------------------------------------------
void dumpBuf() {
  for (uint8_t row = 0; row < sizeof(buf)/16; row++) {
    printHex(16*row);
    Serial.write(' ');
    for (uint8_t col = 0; col < 16; col++) {
      Serial.write(' ');
      printHex(buf[16*row + col]);
    }
    Serial.println();
  }
  Serial.println();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.println("Type any character to begin");
  while (!Serial.available());
  dumpBuf();
  fram.begin();
  fram.read(0L, buf, sizeof(buf));
  dumpBuf();
  for (uint8_t i = 0; i < sizeof(buf); i++) buf[i] = i;
  fram.write(0L, buf, sizeof(buf));
  for (uint8_t i = 0; i < sizeof(buf); i++) buf[i] = 0XFF - i;
  fram.read(0L, buf, sizeof(buf));  
  dumpBuf();
}
void loop() {}
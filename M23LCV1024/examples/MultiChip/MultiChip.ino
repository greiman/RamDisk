#include <RamDisk.h>
#include <M23LCV1024.h>
uint8_t csPins[] = {6, 7, 8, 9};
const uint8_t RAM_CHIP_COUNT = sizeof(csPins)/sizeof(uint8_t);
const uint32_t TOTAL_BYTES = 131072L*RAM_CHIP_COUNT;
M23LCV1024 ram;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Type any character to start"));
  while (!Serial.available());

  if (!ram.begin(csPins, RAM_CHIP_COUNT)) {
    Serial.println(F("ram.begin error"));
    while(1);
  }
  Serial.print(F("Ram size bytes: "));
  Serial.println(TOTAL_BYTES);
  for (uint32_t add = 0; add < TOTAL_BYTES; add += 4) {
    if (!ram.write(add, &add, 4)) {
      Serial.print(F("Write Error: "));
      Serial.println(add);
      while(1);
    }
  }
  for (uint32_t add = 0; add < TOTAL_BYTES; add += 4) {
    uint32_t tmp;
    if (!ram.read(add, &tmp, 4)) {
      Serial.print(F("Read Error: "));
      Serial.println(add);
      while(1);    
    }
    if (tmp != add) {
      Serial.print(F("Data Error: "));
      Serial.print(add);
      Serial.write(',');
      Serial.println(tmp);
      while(1);
    }      
  }
  Serial.println(F("Done!"));
}
void loop() {}
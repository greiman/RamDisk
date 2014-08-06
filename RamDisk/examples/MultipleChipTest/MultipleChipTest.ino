// Test of multiple chips.
#include <RamDisk.h>

#define USE_FRAM 0

#if USE_FRAM
#include <MB85RS2MT.h>
// Multiple FRAM chips so use CS pin list.
uint8_t csPins[] = {8, 9};
const uint8_t CHIP_COUNT = 2;
MB85RS2MT ram;
#else  // USE_FRAM
#include <M23LCV1024.h>
// Multiple SRAM chips so use CS pin list.
uint8_t csPins[] = {6, 7, 8, 9};
const uint8_t CHIP_COUNT = 4;
M23LCV1024 ram;
#endif  // USE_FRAM

RamVolume vol;

RamBaseFile file;
//------------------------------------------------------------------------------
void setup() {
uint32_t n;
  Serial.begin(9600);
  Serial.print(F("Format (Y/N): "));
  while (!Serial.available());

  if (!ram.begin(csPins, CHIP_COUNT)) {
    Serial.println(F("ram.begin error"));
    while(1);
  }
  char c = toupper(Serial.read());
  Serial.println(c);

  if (c == 'Y') {
    // use defaults:
    // totalBlocks: entire RAM
    // dirBlocks: 4  (64 entries)
    // clusterSizeBlocks: 1 (one 512 byte block per cluster)
    if (!vol.format(&ram)) {
      Serial.println(F("format fail"));
      return;
    }
  } 
  else if (c != 'N') {
    Serial.println(F("Invalid entry"));
    return;
  }
  if (!vol.init(&ram)) {
    Serial.println(F("init fail"));
    return;
  }
  vol.printInfo(&Serial);  
  if (!file.open("TEST.BIN", O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println(F("file.open failed"));
    while(1);
  }
  // Total number of writes/reads.
  n = (uint32_t)vol.clusterCount()*vol.clusterSizeBytes()/sizeof(uint32_t);
  Serial.print(F("Writing "));
  Serial.print(n);
  Serial.println(F(" numbers"));
  for (uint32_t i = 0; i < n; i++) {
    if (file.write(&i, 4) != 4) {
      Serial.print(F("Write Error: "));
      Serial.println(i);
      while(1);
    }
  }
  // Update Dir entry.
  file.sync();
  file.rewind();
  Serial.println(F("Starting Read Test"));
  for (uint32_t i = 0; i < n; i++) {
    uint32_t tmp;
    if (file.read(&tmp, 4) != 4) {
      Serial.print(F("Read Error: "));
      Serial.println(i);
      while(1);    
    }
    if (tmp != i) {
      Serial.print(F("Data Error: "));
      Serial.print(i);
      Serial.write(',');
      Serial.println(tmp);
      while(1);
    }      
  }
  Serial.println(F("Done!"));
}
void loop() {}
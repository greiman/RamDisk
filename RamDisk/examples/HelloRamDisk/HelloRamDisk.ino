#include <RamDisk.h>

#define USE_FRAM 0

#define USE_MULTIPLE_CHIPS 0

#if USE_FRAM
#include <MB85RS2MT.h>
#if USE_MULTIPLE_CHIPS
// Multiple FRAM chips so use CS pin list.
uint8_t csPins[] = {8, 9};
const uint8_t CHIP_COUNT = 2;
MB85RS2MT ram;
#else  // USE_MULTIPLE_CHIPS
// Single FRAM chip so use fast single chip template class.
const uint8_t RAM_CS_PIN = 9;
T_MB85RS2MT<RAM_CS_PIN> ram;
#endif  // USE_MULTIPLE_CHIPS
#else  // USE_FRAM
#include <M23LCV1024.h>
#if USE_MULTIPLE_CHIPS
// Multiple SRAM chips so use CS pin list.
uint8_t csPins[] = {6, 7, 8, 9};
const uint8_t CHIP_COUNT = 4;
M23LCV1024 ram;
#else  // USE_MULTIPLE_CHIPS
// Single SRAM chip so use fast single chip template class.
const uint8_t RAM_CS_PIN = 9;
T23LCV1024<RAM_CS_PIN> ram;
#endif  // USE_MULTIPLE_CHIPS
#endif  // USE_FRAM

RamVolume vol;
RamFile file;
char buf[40];
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  
  // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS

  Serial.print(F("Format (Y/N): "));
  while (!Serial.available());
  char c = toupper(Serial.read());
  Serial.println(c);

  if (c == 'Y') {
    // use defaults:
    // totalBlocks: entire RAM
    // dirBlocks: 4  (64 entries)
    // clusterSizeBlocks: 1 (one 512 byte block per cluster)
    if (!vol.format(&ram)) {
      Serial.println(F("format failed"));
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
  if (!file.open("TEST.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F("open fail"));
    return;
  }
  file.println("Hello RamDisk!");
  file.println("Line to test fgets");
  file.rewind();
  int lineNumber = 0;
  // Read file line at a time.
  while (file.fgets(buf, sizeof(buf)) > 0) {
    Serial.print(++lineNumber);
    Serial.print(F(": "));
    Serial.print(buf);
  }
  file.close();
  Serial.println();
  Serial.println(F("Done!"));
}
//------------------------------------------------------------------------------
void loop() {}
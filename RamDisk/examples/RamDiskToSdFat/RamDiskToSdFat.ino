// Create a text fine on the RamDisk an then copy the file to an SD.
// Warning this requires a new test version of SdFat.
#include <SdFat.h>
#include <SdFatUtil.h>
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

#define FILENAME "TEST.CSV"
const uint8_t SD_CS_PIN = SS;

SdFat sd;
SdBaseFile sdFile;
RamVolume vol;
RamFile ramFile;
char buf[40];
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.print(F("FreeRam: "));
  Serial.println(FreeRam());
  
  Serial.print(F("Format (Y/N): "));
  while (!Serial.available());
  
   if (!sd.begin(SD_CS_PIN)) sd.errorHalt();
   
   // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS 
  
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
  } else if (c != 'N') {
    Serial.println(F("Invalid entry"));
    return;
  }
  if (!vol.init(&ram)) {
    Serial.println(F("init fail"));
    return;
  }
  // Remove old version.
  vol.remove(FILENAME);
  if (!ramFile.open(FILENAME, O_CREAT | O_RDWR)) {
    Serial.println(F("open fail"));
    return;
  }
  Serial.println(F("Writing ramFile"));
  uint32_t m0 = micros();
  for (int i = 0; i < 1000; i++) {
    ramFile.print(micros() - m0);
    ramFile.write(",Line,");
    ramFile.println(i);
  }
  // like closing and opening file (need to update dir for ls).
  ramFile.sync();
  ramFile.rewind();
  
  vol.ls(&Serial, LS_DATE | LS_SIZE);

  if (!sdFile.open(FILENAME, O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F("sdFile.open failed"));
    return;
  }
  Serial.println(F("Copying ramFile to sdFile"));
  int n;
  while ((n = ramFile.read(buf, sizeof(buf))) > 0) {
    if (sdFile.write(buf, n) != n) {
      Serial.println(F("sdFile.write failed"));
      return;
    }
  }
  ramFile.close();
  sdFile.close();
  sd.ls(&Serial, LS_DATE | LS_SIZE);

  Serial.println(F("Done"));
}
void loop() {}
// Example that compares single byte read/write
// speed of buffered stream with simple file.
#include <SPI.h>
#include <RamDisk.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <math.h>

#define CHECK_INPUT_DATA 0

const uint16_t LOOP_COUNT = 200;

const uint8_t SD_CS_PIN = SS;
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
RamFile ramFile;
RamStream ramStream;

void setup() {
  uint32_t fileSize;
  uint32_t streamSize;
  uint32_t readTime;
  uint32_t writeTime;
  uint32_t putcTime;
  uint32_t getcTime;
  
  Serial.begin(9600);
  Serial.print(F("FreeRam: "));
  Serial.println(FreeRam());

  Serial.print(F("Format (Y/N): "));
  while (!Serial.available());
  
  // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS
  
  char ans = toupper(Serial.read());
  Serial.println(ans);

  if (ans == 'Y') {
   // use defaults:
    // totalBlocks: entire RAM
    // dirBlocks: 4  (64 entries)
    // clusterSizeBlocks: 1 (one 512 byte block per cluster)
    if (!vol.format(&ram)) {
      Serial.println(F("vol.format failed"));
      return;
    }
  } else if (ans != 'N') {
    Serial.println(F("Invalid entry"));
    return;
  }
  if (!vol.init(&ram)) {
    Serial.println(F("vol.init failed"));
    return;
  }
  if (!ramFile.open("FILE.BIN", O_CREAT | O_RDWR |O_TRUNC)) {
    Serial.println(F("ramFile.open failed"));
    return;
  }
  if (!ramStream.fopen("STREAM.BIN", "w+")) {
    Serial.println(F("ramStream.fopen failed"));
      return;
  }
  uint32_t m = millis();
  for (uint16_t j = 0; j < LOOP_COUNT; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      ramFile.write(i);
    }
  }
  ramFile.rewind();
  writeTime = millis() - m;
  m = millis();
  for (uint16_t j = 0; j < LOOP_COUNT; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      #if CHECK_INPUT_DATA
      if (i != ramFile.read()) {
        Serial.println(F("read error"));
        return;
       }
       #else  // CHECK_INPUT_DATA
       ramFile.read();
       #endif  // CHECK_INPUT_DATA
    }
  }
  readTime = millis() - m;

  m = millis();
  for (uint16_t j = 0; j < LOOP_COUNT; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      ramStream.putc(i);
    }
  }
  ramStream.rewind();
  putcTime = millis() - m;

  m = millis();
  for (uint16_t j = 0; j < LOOP_COUNT; j++) {
    for (uint8_t i = 0; i < 255; i++) {
      #if CHECK_INPUT_DATA
      if (i != ramStream.getc()) {
        Serial.println(F("getc error"));
        return;
      }
      #else  // CHECK_INPUT_DATA
      ramStream.getc();
      #endif  // CHECK_INPUT_DATA
    }
  }
  getcTime = millis() - m;
  
  Serial.println();
  Serial.println(F("File read/write stats"));
  Serial.print(F("fileSize: "));
  Serial.println(ramFile.fileSize());  
  Serial.print(F("write millis: "));
  Serial.println(writeTime);  
  Serial.print(F("read millis: "));
  Serial.println(readTime);

  Serial.println();
  Serial.println(F("Stream getc/putc stats"));
   Serial.print(F("streamSize: "));
  Serial.println(ramStream.ftell()); 
  Serial.print(F("putc millis: "));
  Serial.println(putcTime);  
  Serial.print(F("getc millis: "));
  Serial.println(getcTime);

  ramFile.close();
  ramStream.fclose();
  Serial.println(F("Done!"));
}
void loop() {}
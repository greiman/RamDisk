// Analog pin logging test.
// Formatting will cause a new file to be created.
// Max write and sync time will be longer for a new file.
// Rewriting a file require no directory update so sync() just returns.
//
#include <SPI.h>
#include <RamDisk.h>

const uint32_t N_SAMPLE = 12000L;
const uint8_t  ANALOG_PIN_COUNT = 5;

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
RamBaseFile file;
uint16_t data[ANALOG_PIN_COUNT];

void setup() {
  Serial.begin(9600);
  
  // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS

  while (1) {
    // read any Serial input.
    do {delay(10);} while (Serial.read() >= 0);
 
    Serial.print(F("Format (Y/N): "));
    while (!Serial.available());
    char c = toupper(Serial.read());
    Serial.println(c);
    if (c == 'N') break;
    if (c == 'Y') {
      // use defaults:
      // totalBlocks: entire RAM
      // dirBlocks: 4  (64 entries)
      // clusterSizeBlocks: 1 (one 512 byte block per cluster)
      if (vol.format(&ram)) break;
      Serial.println(F("format fail"));
      return;
    } else {
      Serial.println(F("Invalid entry"));
    }
  }
  if (!vol.init(&ram)) {
    Serial.println(F("init fail"));
    return;
  }
  vol.printInfo(&Serial);
  
  if (!file.open("TEST.BIN", O_CREAT | O_RDWR)) {
    Serial.println(F("open fail"));
    return;
  }
  Serial.println(F("Starting test"));
  Serial.flush();
  uint32_t maxMicros = 0;
  uint32_t minMicros = 9999;
  uint32_t sumMicros = 0;
  uint32_t t = micros();
  uint32_t m;
  for (uint32_t i = 0; i < N_SAMPLE; i++) {
    for (int j = 0; j < 5; j++) data[j] = analogRead(j);
    m = micros();
    if(sizeof(data) != file.write(data, sizeof(data))) {
      Serial.print(F("write failed"));
      while(1);
    }
    m = micros() - m;
    sumMicros += m;
    if (m > maxMicros) maxMicros = m;
    if (m < minMicros) minMicros = m;
  }
  t = micros() - t;
  Serial.print(F("t: "));
  Serial.print(1e-6*t);
  Serial.println(F(" sec"));
  Serial.print(F("minMicros: "));
  Serial.println(minMicros);
  Serial.print(F("maxMicros: "));
  Serial.println(maxMicros);
  Serial.print(F("avgMicros: "));
  Serial.println(sumMicros/N_SAMPLE);
  m = micros();
  file.sync();
  m = micros() - m;
  Serial.print(F("sync micros: "));
  Serial.println(m);
  Serial.print(F("filesize: "));
  Serial.println(file.fileSize());
  
  file.rewind();
  Serial.println();
  for (int j = 0; j < ANALOG_PIN_COUNT; j++) {
    if (j) Serial.write(',');
    Serial.print(F("pin"));
    Serial.print(j);
    }
    Serial.println();
  for (uint16_t i = 0; i < 10; i++) {
    if (file.read(data, sizeof(data)) != sizeof(data)) {
      Serial.println(F("read failed"));
      return;
    }
    for (uint8_t j = 0; j < ANALOG_PIN_COUNT; j++) {
      if (j) Serial.write(',');
      Serial.print(data[j]);
    }
    Serial.println();
  }
  Serial.println(F("Done"));
  file.close();
}
void loop() {
}

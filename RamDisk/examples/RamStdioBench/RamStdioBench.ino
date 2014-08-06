#include <RamDisk.h>
#include <SdFat.h>

// Define PRINT_FIELD nonzero to use printField.
#define PRINT_FIELD 0

// copy all test data to an SD for diff check if non-zero
#define COPY_TO_SD 0

// Number of lines to list on Serial.
#define STDIO_LIST_COUNT 0

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

#if COPY_TO_SD
const uint8_t SD_CS_PIN = SS;
SdFat sd;
SdFile sdFile;
#define PRFILE   "PRFILE.TXT"
#define PRSTREAM "PRSTREAM.TXT"
#endif  // COPY_TO_SD

RamVolume vol;
RamFile printFile;
RamStream stdioFile;

float f[100];
char buf[20];
char* label[] = 
  {"uint8_t 0 to 255, 100 times ", "uint16_t 0 to 18000",
  "uint32_t 0 to 18000", "uint32_t 1000000000 to 1000010000",
  "float nnn.ffff, 10000 times"};
//------------------------------------------------------------------------------
#if COPY_TO_SD
void copyToSd(uint8_t fileType) {
  int n;
  sdFile.close();
  if (fileType == 0) {
    if (!sdFile.open(PRFILE, O_CREAT | O_RDWR | O_AT_END)) {
      Serial.println(F(" open PRFILE failed"));
      return;
    }
    printFile.rewind();
    while ((n = printFile.read(buf, sizeof(buf))) > 0) sdFile.write(buf, n);
  } else {
    if (!sdFile.open(PRSTREAM, O_CREAT | O_RDWR | O_AT_END)) {
      Serial.println(F(" open PRSTREAM failed"));
      return;
    }  
    stdioFile.rewind();
    while ((n = stdioFile.fread(buf, 1, sizeof(buf))) > 0) sdFile.write(buf, n);
    stdioFile.rewind();
  }  
  sdFile.close(); 
}
#endif  // COPY_TO_SD
//------------------------------------------------------------------------------
void setup() {
  uint32_t m;
  uint32_t printSize;
  uint32_t stdioSize;
  uint32_t printTime;
  uint32_t stdioTime;
  
  Serial.begin(9600);
  Serial.println(F("Type any character to start"));
  while (!Serial.available());
  
#if COPY_TO_SD
  if (!sd.begin(SD_CS_PIN)) sd.errorHalt();
  sd.remove(PRFILE);
  sd.remove(PRSTREAM);
#endif  // COPY_TO_SD
  
  for (uint8_t i = 0; i < 100; i++) {
    f[i] = 123.0 + 0.12345*i;
  }  
  // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS
  
  for (uint8_t dataType = 0; dataType < 5; dataType++) {
    for (uint8_t fileType = 0; fileType < 2; fileType++) {
      // Use default format parameters.
      if (!vol.format(&ram)) {
        Serial.println(F("format failed"));
        return;
      }
      if (!vol.init(&ram)) {
        Serial.println(F("init failed"));
        return;    
      }
      if (!fileType) {
        if (!printFile.open("FTEST.TXT", O_CREAT | O_RDWR)) {
          Serial.println("open fail");
            return;
        }
        printTime = millis();
        switch (dataType) {
        case 0:
          for (uint16_t i =0; i < 100; i++) {
            for (uint8_t j = 0; j < 255; j++) {
              printFile.println(j);
            }
          }            
          break;
        case 1:
          for (uint16_t i = 0; i < 18000; i++) {
            printFile.println(i);
          }
          break;
             
        case 2:
          for (uint32_t i = 0; i < 18000; i++) {
            printFile.println(i);
          }
          break;
             
        case 3:
          for (uint16_t i = 0; i < 10000; i++) {
            printFile.println(i + 1000000000UL);
          }
          break;
        
        case 4:
          for (int j = 0; j < 100; j++) {
            for (uint8_t i = 0; i < 100; i++) {
              printFile.println(f[i], 4);
            }
          }
          break;        
        default:
          break;
        }
        printFile.sync();        
        printTime = millis() - printTime;
        printFile.rewind();
        printSize = printFile.fileSize(); 

      } else {
        if (!stdioFile.fopen("FTEST.TXT", "w+")) {
          Serial.println("fopen fail");
          return;
        }
        stdioTime = millis();
        
         switch (dataType) {
        case 0:
          for (uint16_t i =0; i < 100; i++) {
            for (uint8_t j = 0; j < 255; j++) {
              #if PRINT_FIELD
              stdioFile.printField(j, '\n');
              #else  // PRINT_FIELD
              stdioFile.println(j);
              #endif  // PRINT_FIELD
            }
          }            
          break;
        case 1:
          for (uint16_t i = 0; i < 18000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i);
            #endif  // PRINT_FIELD
          }
          break;
             
        case 2:
          for (uint32_t i = 0; i < 18000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i);
            #endif  // PRINT_FIELD
          }
          break;
             
        case 3:
          for (uint16_t i = 0; i < 10000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i + 1000000000UL, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i + 1000000000UL);
            #endif  // PRINT_FIELD      
          }
          break;
        
        case 4:
          for (int j = 0; j < 100; j++) {
            for (uint8_t i = 0; i < 100; i++) {
              #if PRINT_FIELD
              stdioFile.printField(f[i], '\n', 4);
              #else  // PRINT_FIELD
              stdioFile.println(f[i], 4);              
              #endif  // PRINT_FIELD                            
            }
          }
          break;        
        default:
          break;
        }
        stdioFile.fflush();
        stdioTime = millis() - stdioTime;
        stdioSize = stdioFile.ftell();   
        if (STDIO_LIST_COUNT) {
          size_t len;
          stdioFile.rewind();
          for (int i = 0; i < STDIO_LIST_COUNT; i++) {
            stdioFile.fgets(buf, sizeof(buf), &len);
            Serial.print(len);Serial.print(',');
            Serial.print(buf);
          }
        }

      }
      #if COPY_TO_SD
      copyToSd(fileType);
      #endif  // COPY_TO_SD
      printFile.close();     
      stdioFile.fclose();      
    }
    Serial.println(label[dataType]);
    Serial.print("fileSize: ");
    if (printSize != stdioSize) {
      Serial.print(printSize);
      Serial.print(" != ");
    }
    Serial.println(stdioSize);    
    Serial.print("print millis: ");
    Serial.println(printTime);
    Serial.print("stdio millis: ");
    Serial.println(stdioTime);
    Serial.print("ratio: ");
    Serial.println((float)printTime/(float)stdioTime);
    Serial.println();
  }
  Serial.println("Done");
}
void loop() {}
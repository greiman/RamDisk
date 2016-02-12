#include <SPI.h>
#include <RamDisk.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <math.h>

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
SdFat sd;
SdFile sdFile;
//------------------------------------------------------------------------------
void copyToSd() {
  int b;
  sdFile.close();
  if (!sdFile.open("R_FILE.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F(" open R_FILE.TXT failed"));
    return;
  }
  ramFile.rewind();
  while ((b = ramFile.read()) > 0) sdFile.write(b);
  sdFile.close();
  if (!sdFile.open("R_STREAM.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F(" open R_STREAM.TXT failed"));
    return;
  }  
  ramStream.rewind();
  while ((b = ramStream.fgetc()) > 0) sdFile.write(b);
  sdFile.close(); 
}
//------------------------------------------------------------------------------
void floatException() {
  float sv[] = {-INFINITY, -NAN, -1e11, 1e11, NAN, INFINITY};
  for (int i = 0; i < sizeof(sv)/sizeof(float); i++) {
    sdFile.println(sv[i]);    
    ramFile.println(sv[i]);
    ramStream.println(sv[i]);
  };
  // uncomment return for debug.
  return;
  int b; 
  Serial.println();
  Serial.println("Special float values differ:");
  Serial.println(F("Arduino -INFINITY, -NAN, -1e11, 1e11, NAN, INFINITY"));
  ramFile.rewind();
  while((b = ramFile.read()) > 0) Serial.print((char)b);
  ramFile.rewind();  
  Serial.println(F("Stdio -INFINITY, -NAN, -1e11, 1e11, NAN, INFINITY")); 
  ramStream.rewind(); 
  while((b = ramStream.fgetc()) > 0) Serial.print((char)b);
  ramStream.rewind();
  sdFile.rewind();
  Serial.println();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.print(F("FreeRam: "));
  Serial.println(FreeRam());
  
  // Initialize RAM
#if USE_MULTIPLE_CHIPS
  ram.begin(csPins, CHIP_COUNT);
#else  // USE_MULTIPLE_CHIPS
  ram.begin();
#endif  // USE_MULTIPLE_CHIPS

  Serial.print(F("Format (Y/N): "));
  while (!Serial.available());
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
  if (!sd.begin(SD_CS_PIN)) sd.errorHalt();
  if (!sdFile.open("SDFILE.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F("open SDFILE.TXT failed"));
    return;
  } 
  if (!vol.init(&ram)) {
    Serial.println(F("vol.init failed"));
    return;
  }
  if (!ramFile.open("FILE.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
    Serial.println(F("ramFile.open failed"));
    return;
  }
  if (!ramStream.fopen("STREAM.TXT", "w+")) {
    Serial.println(F("ramStream.fopen failed"));
      return;
  }
  // do float exception first
  floatException();
  
  // strings
  sdFile.println("regular string");
  ramFile.println("regular string");  
  ramStream.println("regular string");  
  
  // flash strings
  sdFile.println(F("flash string"));
  ramFile.println(F("flash string"));  
  ramStream.println(F("flash string"));
  
  // test char
  char c[] = {'A', 'B', 'C', 'D', 'E'};
  for (int i = 0; i < sizeof(c); i++) {
    sdFile.println(c[i]);  
    ramFile.println(c[i]);
    ramStream.println(c[i]);
  }
  // test signed char
  signed char sc[] = {0, 1, 0X7F, 0X80, 0XFF};
  for (int i = 0; i < sizeof(sc); i++) {
    sdFile.println(sc[i]);  
    ramFile.println(sc[i]);
    ramStream.println(sc[i]);
  } 
  // test unsigned char
  unsigned char uc[] = {0, 1, 0X7F, 0X80, 0XFF};
  for (int i = 0; i < sizeof(uc); i++) {
    sdFile.println(uc[i]);  
    ramFile.println(uc[i]);
    ramStream.println(uc[i]);
  }
  // test signed short
  signed short ss[] = {0, 1, 0X7FFF, 0X8000, 0XFFFF};
  for (int i = 0; i < sizeof(ss)/sizeof(short); i++) {
    sdFile.println(ss[i]);  
    ramFile.println(ss[i]);
    ramStream.println(ss[i]);
  } 
  // test unsigned signed short
  unsigned short us[] = {0, 1, 0X7FFF, 0X8000, 0XFFFF};
  for (int i = 0; i < sizeof(us)/sizeof(short); i++) {
     sdFile.println(us[i]); 
    ramFile.println(us[i]);
    ramStream.println(us[i]);
  }   
  // test signed int
  signed int si[] = {0, 1, 0X7FFF, 0X8000, 0XFFFF};
  for (int i = 0; i < sizeof(si)/sizeof(int); i++) {
    sdFile.println(si[i]);  
    ramFile.println(si[i]);
    ramStream.println(si[i]);
  }
  // test unsigned signed int
  unsigned int ui[] = {0, 1, 0X7FFF, 0X8000, 0XFFFF};
  for (int i = 0; i < sizeof(ui)/sizeof(int); i++) {
    sdFile.println(ui[i]);  
    ramFile.println(ui[i]);
    ramStream.println(ui[i]);
  }
  // test signed long
  signed long sl[] = {0, 1, 0X7FFFFFFF, 0X80000000, 0XFFFFFFFF};
  for (int i = 0; i < sizeof(sl)/sizeof(long); i++) {
    sdFile.println(sl[i]);  
    ramFile.println(sl[i]);
    ramStream.println(sl[i]);
  }  
  // test unsigned long
  unsigned long ul[] = {0, 1, 0X7FFFFFFF, 0X80000000, 0XFFFFFFFF};
  for (int i = 0; i < sizeof(ul)/sizeof(long); i++) {
    sdFile.println(ul[i]);  
    ramFile.println(ul[i]);
    ramStream.println(ul[i]);
  }
  // test float
  float flt[] = {-1234.5678, -123.45678, -M_PI, 0, M_PI, 123.45678, 1234.5678};
  for (int i = 0; i < sizeof(flt)/sizeof(float); i++) {
    sdFile.println(flt[i], 4);   
    ramFile.println(flt[i], 4);
    ramStream.println(flt[i], 4);
  }              
  // test double - should be same as float
  double dbl[] = {-1234.5678, -123.45678, -M_PI, 0, M_PI, 123.45678, 1234.5678};
  for (int i = 0; i < sizeof(dbl)/sizeof(float); i++) {
    sdFile.println(dbl[i], 4);   
    ramFile.println(dbl[i], 4);
    ramStream.println(dbl[i], 4);
  }
  // test hex - should be same as float
  unsigned long hex[] = {0, 1, 0XF, 0X76543210, 0XFEDCBA98};
  for (int i = 0; i < sizeof(hex)/sizeof(long); i++) {
    sdFile.println(hex[i], HEX);   
    ramFile.println(hex[i], HEX);
    ramStream.printHexln(hex[i]);
  }  
  //----------------------------------------------------------------------------
  // verify results
  size_t rfs = ramFile.fileSize();
  size_t rss = ramStream.ftell();
  size_t sds = sdFile.fileSize();
  
  if (sds != rfs || sds != rss) {
    Serial.println(F("file size error"));
    copyToSd();
    return;
  }
  Serial.print(F("fileSize: "));
  Serial.println(sds);
  ramFile.rewind();
  ramStream.rewind();  
  sdFile.rewind();
  size_t pos;
  for (pos = 0; pos < sds; pos++) {
    uint8_t sb = sdFile.read();
    uint8_t rb = ramFile.read();
    if (sb != rb) {
      Serial.println(F("sdFile ramFile differ"));
      break;
    }
    rb = ramStream.fgetc();
    if (sb != rb) {
      Serial.println(F("sdFile ramStream differ"));
      break;
    }      
  }
  if (pos == sds) Serial.println(F("files verify"));
  copyToSd();
  sdFile.close();
  ramFile.close();
  ramStream.fclose();
 // Serial.println(F("Done!"));  
}
void loop() {}
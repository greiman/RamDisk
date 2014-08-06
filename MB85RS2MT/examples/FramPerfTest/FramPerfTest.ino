#include <RamDisk.h>
#include <MB85RS2MT.h>
#include <SPI.h>

const uint8_t RAM_CS_PIN = 9;

uint8_t buf[1024];

#define USE_T_MB85RS2MT 1

#if USE_T_MB85RS2MT
T_MB85RS2MT<RAM_CS_PIN> ram;
#else  // USE_T_MB85RS2MT
MB85RS2MT ram;
#endif  // USE_T_MB85RS2MT


uint16_t wr = 1234;
uint16_t rd = 0;

void setup() {
  Serial.begin(9600);

#if USE_T_MB85RS2MT
  ram.begin();
#else  // USE_T_MB85RS2MT 
  ram.begin(RAM_CS_PIN);
#endif   // USE_T_MB85RS2MT
  // loops for scope tests
  // while(1) {ram.read(444, &rd, 2);delay(1);}
  // while(1) {ram.write(444, &wr, 2);delay(1);}

  uint8_t *u = (uint8_t*)&wr;
  uint32_t m = micros();
  for (int i = 0; i < 100; i++) {
    ram.write(444, &wr, 2);
    ram.read(444, &rd, 2);
  }  
  m = micros() - m;

  Serial.print("micros: ");
  Serial.println(m);
  Serial.print("data: ");
  Serial.println(rd);

  for (int i = 0; i < sizeof(buf); i++) {
    buf[i] = i;
  }
  m = micros();

  if (!ram.write(333, buf, sizeof(buf))) Serial.println("write error");

  m = micros() - m;
  Serial.print("write micros: ");
  Serial.println(m);
  for (int i = 0; i < sizeof(buf); i++)  buf[i] = 0;
  
  
  
  Serial.println(F("Read test"));
  Serial.println(F("Bytes,Micros,KB/sec")); 
  for (size_t n = 1; n <= sizeof(buf); n *= 2) {
    m = micros();
    for (size_t j = 0; j < sizeof(buf)/n; j++) {
      if (!ram.read(333, buf, n)) Serial.println("read error");
    }
    m = micros() - m;
    Serial.print(n);
    Serial.write(',');
    Serial.print((float)m*n/sizeof(buf));
    Serial.write(',');
    Serial.println(1000.0*sizeof(buf)/m);
  }
  Serial.println();
  Serial.println(F("Write test"));
  Serial.println(F("Bytes,Micros,KB/sec")); 
  for (size_t n = 1; n <= sizeof(buf); n *= 2) {
    m = micros();
    for (size_t j = 0; j < sizeof(buf)/n; j++) {
      if (!ram.write(333, buf, n)) Serial.println("write error");
    }
    m = micros() - m;
    Serial.print(n);
    Serial.write(',');
    Serial.print((float)m*n/sizeof(buf));
    Serial.write(',');
    Serial.println(1000.0*sizeof(buf)/m);
  }
  Serial.println("Done!");
}
void loop() {
}
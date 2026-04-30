#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Wire2.begin();

  Serial.println("I2C scanner started on Wire2 (SCL2=pin 3, SDA2=pin 4)");
}

void loop() {
  byte error, address;
  int devices = 0;

  for (address = 1; address < 127; address++) {
    Wire2.beginTransmission(address);
    error = Wire2.endTransmission();

    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }

  if (devices == 0) {
    Serial.println("No I2C devices found");
  }

  Serial.println("----");
  delay(2000);
}
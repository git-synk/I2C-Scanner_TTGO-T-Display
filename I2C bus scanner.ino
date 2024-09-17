#include <Wire.h>         
#include <TFT_eSPI.h>     

TFT_eSPI tft = TFT_eSPI();  
void setup() {
 
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(8, 5);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.println("I2C Scanner");

  Wire.begin(21, 22); 
  scanI2C();
}

void loop() { }

void scanI2C() {
  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(10, 26 + nDevices * 20);
      tft.printf("Device found: 0x%02X", address);
      nDevices++;
    }
  }

  if (nDevices == 0) {
    tft.setCursor(10, 30);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("No devices found");
  }
}

/*
This code will scan continuously for i2c devices on pins 6 and 7
and display it on a SSD1306 128x64 i2c OLED.
This has been tested on a RP2040-Zero on 14/09/2025 and was 
working as intended.

Important pins:
 --- RP2040-Zero ---
 GP4 = SDA (display)
 GP5 = SCL (display)
 GP6 = SDA (scanning)
 GP7 = SCL (scanning)

 --- OLED ---
 VCC = 3.3V (3v3 pin - RP2040-Zero's internal regulator)
 GND = GND (GND pin on RP2040-Zero)
 SCL = GP5 (default)
 SDA = GP4 (default)

This code is written by Dan from the Simple Electronics YouTube
channel and is mostly a mashup of library examples and has been
over-commented to be beginner-friendly for all to use.
***THIS SPECIFIC PROJECT DID RELY HEAVILY ON AI FOR CODE***
https://www.youtube.com/SimpleElectronics

The code contributed by Dan is open-source under the MIT Licence
however, the libraries retain their own licensing

Copyright <2025> <Dan, Simple Electronics>

Permission is hereby granted, free of charge, to any person obtaining 
a copy of this software and associated documentation files (the “Software”), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Wire.h> // Needed for communications
#include <Adafruit_GFX.h> // Needed to style the fonts on the display
#include <Adafruit_SSD1306.h> // Needed for the specific OLED display

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // Common address for SSD1306, change if needed

// Create display object on I2C0 (pins 4,5)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2C1 pins for RP2040-zero
#define I2C1_SDA 6
#define I2C1_SCL 7

// Create second I2C instance
TwoWire Wire1(I2C1_SDA, I2C1_SCL);

// Array to store previously found devices
uint8_t previousDevices[128];
uint8_t prevDeviceCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize I2C0 for OLED (pins 4,5 - default Wire)
  Wire.begin();
  
  // Initialize I2C1 for device scanning
  Wire1.begin();
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("I2C Scanner Ready"));
  display.println(F("Plug device into"));
  display.println(F("I2C1 bus"));
  display.println();
  display.print(F("SDA: Pin "));
  display.println(I2C1_SDA);
  display.print(F("SCL: Pin "));
  display.println(I2C1_SCL);
  display.display();
  
  Serial.println("I2C Scanner initialized");
  Serial.println("OLED on I2C0 (pins 4,5)");
  Serial.println("Scanning I2C1 (pins " + String(I2C1_SDA) + "," + String(I2C1_SCL) + ")");
}

void loop() {
  uint8_t currentDevices[128];
  uint8_t currentDeviceCount = 0;
  
  // Scan I2C1 bus for devices
  for (uint8_t address = 1; address < 127; address++) {
    Wire1.beginTransmission(address);
    uint8_t error = Wire1.endTransmission();
    
    if (error == 0) {
      currentDevices[currentDeviceCount] = address;
      currentDeviceCount++;
    }
  }
  
  // Check if devices have changed
  bool devicesChanged = false;
  
  if (currentDeviceCount != prevDeviceCount) {
    devicesChanged = true;
  } else {
    // Compare device lists
    for (uint8_t i = 0; i < currentDeviceCount; i++) {
      bool found = false;
      for (uint8_t j = 0; j < prevDeviceCount; j++) {
        if (currentDevices[i] == previousDevices[j]) {
          found = true;
          break;
        }
      }
      if (!found) {
        devicesChanged = true;
        break;
      }
    }
  }
  
  // Update display if devices changed
  if (devicesChanged) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    
    if (currentDeviceCount == 0) {
      display.println(F("No I2C devices found"));
      display.println(F("on I2C1 bus"));
      display.println();
      display.println(F("Waiting for device..."));
    } else {
      display.println(F("I2C Devices Found:"));
      display.println();
      
      for (uint8_t i = 0; i < currentDeviceCount; i++) {
        display.print(F("0x"));
        if (currentDevices[i] < 16) display.print(F("0"));
        display.print(currentDevices[i], HEX);
        display.print(F(" ("));
        display.print(currentDevices[i]);
        display.println(F(")"));
        
        // Print to serial as well
        Serial.print("Device found at address 0x");
        if (currentDevices[i] < 16) Serial.print("0");
        Serial.print(currentDevices[i], HEX);
        Serial.print(" (");
        Serial.print(currentDevices[i]);
        Serial.println(")");
      }
      
      // Show scan time
      display.println();
      display.print(F("Scan: "));
      display.print(millis() / 1000);
      display.println(F("s"));
    }
    
    display.display();
    
    // Update previous devices list
    prevDeviceCount = currentDeviceCount;
    for (uint8_t i = 0; i < currentDeviceCount; i++) {
      previousDevices[i] = currentDevices[i];
    }
  }
  
  delay(1000); // Wait a second to scan again
}

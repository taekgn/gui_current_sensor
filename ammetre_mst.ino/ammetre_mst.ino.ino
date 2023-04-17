#include <Adafruit_GFX.h>           // Include core graphics library
#include <Adafruit_ILI9341.h>       // Include Adafruit_ILI9341 library to drive the display
#include <Fonts/FreeSerif24pt7b.h>  // Add a custom font
#include <Wire.h>
#include <Adafruit_INA219.h>

#if ARDUINO >= 100  // Arduino IDE versions before 100 need to use the older library
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <INA.h>  // Zanshin INA Library

#if defined(_SAM3XA_) || defined(ARDUINO_ARCH_SAMD)
// The SAM3XA architecture needs to include this library, it is already included automatically on
// other platforms //
#include <avr/dtostrf.h>  // Needed for the SAM3XA (Arduino Zero)
#endif

const uint32_t SHUNT_MICRO_OHM{ 20000 };  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS{ 6 };         ///< Max expected amps, clamped from 1A to a max of 1022A
uint8_t devicesFound{ 0 };                ///< Number of INAs found
INA_Class INA;                            ///< INA class instantiation to use EEPROM

// Declare pins for the display:
#define TFT_DC 9
#define TFT_RST 8  // You can also connect this to the Arduino reset in which case, set this #define pin to -1!
#define TFT_CS 10
// The rest of the pins are pre-selected as the default hardware SPI for Arduino Uno (SCK = 13 and SDA = 11)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
volatile float rd_current;

void setup()  // Start of setup
{
  tft.begin();  // Initialize display
  tft.setRotation(1);
  tft.fillScreen(0x0000);         // Fill screen with black
  tft.setTextWrap(false);         // By default, long lines of text are set to automatically “wrap” back to the leftmost column.
  tft.setCursor(0, 30);           // Set position (x,y)
  tft.setFont(&FreeSerif24pt7b);  // Set a custom font
  tft.setTextColor(0xFFE0);       // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(0);             // Set text size. We are using custom font so you should always set text size as 0
  tft.println("DC Current");      // Print a text or value
  tft.setCursor(0, 80);           // Set position (x,y)
  tft.println("Ammeter");         // Print a text or value

  tft.setFont();                                    // Reset to standard font, to stop using any custom font previously set
  tft.drawRoundRect(236, 110, 80, 60, 10, 0x07FF);  // Draw rounded rectangle (x,y,width,height,radius,color)
  tft.drawRoundRect(236, 180, 80, 60, 10, 0x07FF);  // Draw rounded rectangle (x,y,width,height,radius,color)

  Serial.begin(115200);
  devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
  while (devicesFound == 0) {
    Serial.println(F("No INA device found, retrying in 10 seconds..."));
    delay(10000);                                             // Wait 10 seconds before retrying
    devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
  }                                                           // while no devices detected
  INA.setBusConversion(8500);             // Maximum conversion time 8.244ms
  INA.setShuntConversion(9000);           // Maximum conversion time 8.244ms
  INA.setAveraging(128);                  // Average each reading n-times
  INA.setMode(INA_MODE_CONTINUOUS_BOTH);  // Bus/shunt measured continuously
}

void loop() {
  auto current = 0.000;
  auto buff = INA.getBusMicroAmps(0) / 1000.0 + 0.55;

  for (int i = 0; i < 10; i++)
    current += buff; delay(5);

  current /= 10.000;
  current = (current > 999.99) ? (current / 1000) : current;

  auto unit = (buff > 999) ? "  A" : " mA";
  rd_current = (buff > 999) ? (buff / 1000) : buff;

  tft.setTextSize(2);
  tft.setCursor(12, 100);    // Set position (x,y)
  tft.println("Real-time");  // Print a text or value
  tft.setCursor(12, 170);    // Set position (x,y)
  tft.println("Average");    // Print a text or value

  tft.setCursor(48, 125);            // Set position (x,y)
  tft.setTextColor(0xFFFF, 0x0000);  // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(4);                // Set text size. Goes from 0 (the smallest) to 20 (very big)
  tft.println(rd_current);           // Print a text or value

  tft.setCursor(238, 125);           // Set position (x,y)
  tft.setTextColor(0x07E0, 0x0000);  // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(4);                // Set text size. Goes from 0 (the smallest) to 20 (very big)
  tft.println(unit);                 // Print a text or value

  tft.setCursor(48, 195);            // Set position (x,y)
  tft.setTextColor(0xFFFF, 0x0000);  // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(4);                // Set text size. Goes from 0 (the smallest) to 20 (very big)
  tft.println(current);              // Print a text or value

  tft.setCursor(238, 195);           // Set position (x,y)
  tft.setTextColor(0x07E0, 0x0000);  // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(4);                // Set text size. Goes from 0 (the smallest) to 20 (very big)
  tft.println(unit);                 // Print a text or value

  tft.setFont();

  Serial.print("Current:       ");
  Serial.print(buff / 1000.0, 6);
  Serial.println(unit);
  Serial.print("Mean Current:  ");
  Serial.print(current, 6);
  Serial.println(unit);
  Serial.println("");
  delay(500);
}
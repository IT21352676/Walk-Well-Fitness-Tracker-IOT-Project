#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Create an SSD1306 object for a 128x32 OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin for 801S Vibration Sensor
const int vibrationPin = 34;  // GPIO pin connected to the sensor
bool fallDetected = false;

void setup() {
  // Initialize the vibration sensor pin
  pinMode(vibrationPin, INPUT);

  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize the OLED display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);  // Stay here if OLED initialization fails
  }

  // Clear the display buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Monitoring for Falls...");
  display.display();
}

void loop() {
  // Read the vibration sensor output
  int vibrationValue = digitalRead(vibrationPin);
  //Serial.println(vibrationValue);

  // If vibration is detected, assume a fall occurred
  if (vibrationValue == LOW) {
    fallDetected = true;
    Serial.println("Fall Detected!");
    
    // Display the fall detection message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("FALL");
    display.setCursor(0, 16);
    display.print("DETECTED!");
    display.display();

    // Add delay to avoid multiple triggers
    delay(2000);  // Wait 2 seconds
  } else {
    // Reset the display to monitoring message if no fall is detected
    if (fallDetected) {
      fallDetected = false;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.print("Monitoring for Falls...");
      display.display();
    }
  }

  delay(100);  // Small delay to stabilize sensor readings
}

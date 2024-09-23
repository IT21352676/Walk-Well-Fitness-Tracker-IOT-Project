#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Create an SSD1306 object for a 128x32 OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MAX30100 sensor object
PulseOximeter pox;

// Timer for updating heart rate every second
uint32_t lastUpdateTime = 0;

// Function to initialize the OLED display
void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);  // Stay here if OLED initialization fails
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Heart Rate Monitor");
  display.display();
}

// Callback function to get heart rate and oxygen saturation
void onBeatDetected() {
  Serial.println("Beat Detected!");
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize I2C communication with custom pins
  Wire.begin(17, 16);  // SDA = GPIO 17, SCL = GPIO 16
  
  // Initialize OLED display
  setupDisplay();

  // Initialize MAX30100 sensor
  if (!pox.begin()) {
    Serial.println("Failed to initialize MAX30100. Check connections.");
    while (1);  // Stay here if the sensor doesn't initialize
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Set up display message
  display.setCursor(0, 16);
  display.print("Initializing...");
  display.display();
}

void loop() {
  // Update the sensor readings
  pox.update();

  // Update the display and serial output every second
  if (millis() - lastUpdateTime > 1000) {
    lastUpdateTime = millis();

    // Get heart rate and oxygen saturation
    float heartRate = pox.getHeartRate();
    float SpO2 = pox.getSpO2();

    // Clear and update the OLED display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Heart Rate: ");
    display.print(heartRate);
    display.println(" BPM");

    display.setCursor(0, 16);
    display.print("SpO2: ");
    display.print(SpO2);
    display.println(" %");

    display.display();

    // Log the values to the serial monitor
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.println(" BPM");

    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println(" %");
  }

  delay(100);  // Small delay for stability
}

#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MPU6050 sensor object
MPU6050 mpu;

// Variables for step counting
int steps = 0;
float lastAccelerationZ = 0;
float lastAccelerationX = 0;
float lastAccelerationY = 0;
const int threshold = 3000;  // Adjust based on testing
const int debounceTime = 200; // Milliseconds between valid steps
unsigned long lastStepTime = 0;


// Pin for 801S Vibration Sensor
const int vibrationPin = 34;   // GPIO pin connected to the sensor
bool fallDetected = false;



void setup() {
  pinMode(vibrationPin, INPUT);
  
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize I2C communication
  Wire.begin();

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);
  }
  
  // Clear display buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  Serial.println("Initialization done. Start moving!");
}

void loop() {
  // Get current time
  unsigned long currentTime = millis();

  // Get acceleration data from MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Calculate magnitude of acceleration (Pythagorean theorem)
  float accelMagnitude = sqrt(sq(ax) + sq(ay) + sq(az));

  // Detect step by checking changes in Z-axis primarily and supporting with X and Y axes
  if ((abs(az - lastAccelerationZ) > threshold) && 
      (abs(ax - lastAccelerationX) > threshold / 2 || abs(ay - lastAccelerationY) > threshold / 2)) {
      
    // Debounce the step detection to avoid double-counting
    if (currentTime - lastStepTime > debounceTime) {
      steps++;
      lastStepTime = currentTime;

      // Debug: Print step count
      Serial.print("Steps: ");
      Serial.println(steps);
    }
  }

  // Store the previous acceleration values
  lastAccelerationZ = az;
  lastAccelerationX = ax;
  lastAccelerationY = ay;

  // Display the step count on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Steps: ");
  display.println(steps);
  
  // Show the display content
  display.display();

  // Delay for stability and to avoid excessive sensor reading
  delay(100);
//Serial.println(vibrationPin);
int vibrationValue = digitalRead(vibrationPin);
Serial.println(vibrationValue);

//  if (vibrationValue == LOW) {
//    fallDetected = true;
//    Serial.println("Fall Detected!");
//    
//    // Display the fall detection message
//    display.clearDisplay();
//    display.setCursor(0, 0);
//   //
//   display.setTextSize(2);
//    display.print("FALL");
//    display.setCursor(0, 16);
//    display.print("DETECTED!");
//    display.display();
//
//    // Add delay to avoid multiple triggers
//    delay(2000);  // Wait 2 seconds
 // }
}

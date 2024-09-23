#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>



#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// WiFi and MQTT settings
const char* ssid = "Hirusha";         // Your WiFi SSID
const char* password = "hirusha8316";   // Your WiFi Password
const char* mqtt_server = "broker.hivemq.com"; // Public MQTT broker or your local one
const char* topic = "fitness/steps";
const char* topicPattern = "fitness/pattern";

// LED Pin and LED count
#define PIN 5    // WS2812B connected to GPIO 4
#define NUMPIXELS 1 // We use one LED

// MQTT and WiFi objects
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


// Timer variables for blinking the LED
bool isBlinkOn = true;
unsigned long lastBlinkTime = 0;
unsigned long blinkInterval = 500; // 500 milliseconds for blink speed


// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MPU6050 sensor object
MPU6050 mpu;

// Variables for step counting
int steps = 0;
String currentPattern = "Idle";
float lastAccelerationZ = 0;
float lastAccelerationX = 0;
float lastAccelerationY = 0;
const int threshold = 5000;  // Adjust based on testing
const int debounceTime = 500; // Milliseconds between valid steps
unsigned long lastStepTime = 0;

int fallThreshold = 5000;  // Adjust based on testing
bool isFalling = false;
unsigned long fallStartTime = 0;


// Pin for 801S Vibration Sensor
const int vibrationPin = 34;   // GPIO pin connected to the sensor
bool fallDetected = false;


// Variables for animation
int16_t posX = 0; // X position for animated text
int8_t direction = 1; // Direction for animation 








void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    
    Serial.print(".");
    pixels.setPixelColor(0, pixels.Color(127, 63, 0)); // Red: Not connected
    pixels.show();
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  pixels.setPixelColor(0, pixels.Color(0, 127, 0)); // Green: Connected
  pixels.show();
  delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages if necessary
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      pixels.setPixelColor(0, pixels.Color(0, 0, 127)); // Green: Connected
      pixels.show();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      pixels.setPixelColor(0, pixels.Color(127, 0, 0)); // Red: MQTT not connected
      pixels.show();
    }
  }
}

void handleMQTTConnectedLED() {
  unsigned long currentMillis = millis();

  // Blink the LED every 500 ms when MQTT is connected (Blue color)
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;

    if (isBlinkOn) {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue: MQTT connected (LED ON)
    } else {
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));   // LED OFF
    }

    pixels.show();
    isBlinkOn = !isBlinkOn; // Toggle the blink state
  }
}


void manageMQTT(void *parameter) {
  for (;;) {
  // Check if the client is connected before running client.loop()
  if (client.connected()) {
    //Serial.println(steps);
    // Publish step count to MQTT broker
   String stepCountStr = String(steps);
    client.publish(topic, stepCountStr.c_str());

    String patternStr = String(currentPattern);
    client.publish(topicPattern, patternStr.c_str());
    client.loop();  // Run MQTT client processing
  } else {
    reconnect();  // Reconnect if MQTT client is disconnected
    }
  }
}



TaskHandle_t MQTTTaskHandle = NULL;



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






  pixels.begin(); // Initialize WS2812B LED
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  xTaskCreatePinnedToCore(manageMQTT, "MQTTTask", 10000, NULL, 1, &MQTTTaskHandle, 1);
}


void fallDetection() {
  int accelerationX = mpu.getAccelerationX();
  int accelerationY = mpu.getAccelerationY();
  int accelerationZ = mpu.getAccelerationZ();

  int totalAcceleration = sqrt(accelerationX * accelerationX + accelerationY * accelerationY + accelerationZ * accelerationZ);

  //Serial.print("Fall Detected!");
 //Serial.println(totalAcceleration);
  
  if (totalAcceleration < fallThreshold) {
   Serial.println("Fall Detected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    //display.setTextSize(2);
    display.print("FALL");
    display.setCursor(0, 16);
    display.print("DETECTED!");
    display.display();
 
    delay(2000);
    
    if (!isFalling) {
      
      isFalling = true;
      fallStartTime = millis();
    }
  } else {
    isFalling = false;
  }

  // Check for inactivity after the fall
  if (isFalling && (millis() - fallStartTime > 500)) {  // 3 seconds of inactivity
    
    // Take action (e.g., alert or log the event)
    isFalling = false;
  }

  delay(100);  // Adjust delay for fall recognition
}


void loop() {

fallDetection();
  
 // manageMQTT();
  display.clearDisplay();
  
  display.setCursor(posX, 0);
  display.print("*** Well Walker ***");

  // Update the position for the animation
  posX += direction;
  if (posX > SCREEN_WIDTH - 115 || posX < 0) {  // Adjust '70' based on text width
    direction = -direction;  // Reverse direction when hitting the screen boundaries
  }



   // Display the step count on OLED
 
  display.setTextSize(1);
  display.setCursor(0, 13);
  display.print("Steps: ");
  display.println(steps);

  display.setTextSize(1);
  display.setCursor(0, 23); // Adjust the cursor to the next line
  display.print("Pattern: ");
  display.println(currentPattern);
  
  // Show the display content
  display.display();


// Get current time
unsigned long currentTime = millis();

int16_t ax, ay, az;
mpu.getAcceleration(&ax, &ay, &az);

// Calculate the magnitude of acceleration
float accelMagnitude = sqrt(sq(ax) + sq(ay) + sq(az));

// Calculate step interval
unsigned long stepInterval = currentTime - lastStepTime;


if (stepInterval > 3000) {
    currentPattern = "Idle";
    //delay(100);
 }

// Detect step by checking changes in Z-axis primarily and supporting with X and Y axes
if ((abs(az - lastAccelerationZ) > threshold) && 
    (abs(ax - lastAccelerationX) > threshold / 2 || abs(ay - lastAccelerationY) > threshold / 2)) {
  
  // Debounce the step detection to avoid double-counting
  if (stepInterval > debounceTime) {
    //steps++;
    lastStepTime = currentTime;


//Serial.println(stepInterval);

   if (stepInterval > 650 && stepInterval < 2000) {
   //delay(1000);
    currentPattern = "Walking"; 
    steps++;
  } else if (stepInterval < 650) {
    currentPattern = "Running";
    steps++;
    //delay(100);
  }
  

  
//   if (stepInterval > 1000) {
//    currentPattern = "Slow Walking";
//  } else if (stepInterval > 700) {
//    currentPattern = "Walking";
//  } else if (stepInterval > 500) {
//    currentPattern = "Running";
//  } else {
//    currentPattern = "Idle";
//  }

   // Print the current pattern
  Serial.println("Current Pattern: " + currentPattern);

    // Debug: Print step count
    Serial.print("Steps: ");
    Serial.println(steps);
  }
}

// Store the previous acceleration values
lastAccelerationZ = az;
lastAccelerationX = ax;
lastAccelerationY = ay;

// Delay for stability
delay(100);

//Serial.pri.ntln(vibrationPin);
//int vibrationValue = digitalRead(vibrationPin);
//Serial.println(vibrationValue);

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


  // Handle blinking for the MQTT connected state
  //handleMQTTConnectedLED();
  //pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Green: Connected
  //pixels.show();
 

  //delay(5000); // Publish every 5 seconds
}

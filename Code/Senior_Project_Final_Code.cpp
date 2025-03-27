#include <Ticker.h>  // Used for scheduling timed events

// Blynk template and authentication details
#define BLYNK_TEMPLATE_ID "YPL12F6pCbKp0"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "1Lx7b06xnsGfcGn214PeLkuRWFZ8PL-L"

#define BLYNK_PRINT Serial  // Enable serial debug output for Blynk

#include <ESP8266WiFi.h>         // ESP8266 WiFi support
#include <BlynkSimpleEsp8266.h>  // Blynk library for ESP8266
#include <Servo.h>               // For controlling servo motors
#include <Wire.h>                // I2C communication
#include <Adafruit_GFX.h>        // OLED display graphics
#include <Adafruit_SSD1306.h>    // OLED driver library

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Hardware pins and virtual pin setup
#define RELAY_PIN D3
#define VIRTUAL_PIN V3
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"

// WiFi credentials
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

BlynkTimer timer;  // Blynk's software timer for periodic tasks

// State variables
int relaySTATE = LOW;
int servoSTATE1 = 0;
int servoSTATE2 = 90;
int previousButtonTime = 0;

// Servo setup
Servo gServo, gServo2;
#define SERVO1_PIN D6
#define SERVO2_PIN D7

// Arduino setup function
void setup()
{
  Serial.begin(115200);  // Start serial monitor
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Connect to Blynk server

  timer.setInterval(1000L, myTimerEvent);  // Run a function every 1 second

  // Attach servos to pins and initialize position
  gServo.attach(SERVO1_PIN);
  gServo.write(0);
  gServo2.attach(SERVO2_PIN);
  gServo2.write(90);

  pinMode(LED_BUILTIN, OUTPUT);  // Built-in LED for status
  pinMode(RELAY_PIN, OUTPUT);    // Output relay pin
  digitalWrite(RELAY_PIN, relaySTATE);  // Set initial relay state

  // Initialize OLED display
  if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  oledDisplay.display();
  oledDisplay.clearDisplay();
  delay(2000);
  oledTextWrite("BLYNK Smart Plug App", 0, 0, 1);
  oledDisplay.display();
}

// Main loop that runs continuously
void loop()
{
  Blynk.run();  // Maintain connection to Blynk server
  timer.run();  // Execute Blynk timers

  // Check for changes in servo states and update
  if (gServo.read() != servoSTATE1) {
    gServo.write(servoSTATE1);
    previousButtonTime = millis();
  }
  if (gServo2.read() != servoSTATE2) {
    gServo2.write(servoSTATE2);
  }

  // Logic to reset states after a delay
  if (millis() - previousButtonTime > 1000) {
    if (gServo.read() == 90) {
      servoSTATE1 = 0;
      gServo.write(servoSTATE1);
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V0, 0);
    }
    if (gServo2.read() == 0) {
      servoSTATE2 = 90;
      gServo2.write(servoSTATE2);
    }
    previousButtonTime = millis();
  }
}

// This function is triggered when button on V3 is toggled from Blynk
BLYNK_WRITE(V3)
{
  int value = param.asInt();
  if (value == 1) {
    relaySTATE = HIGH;
    digitalWrite(RELAY_PIN, relaySTATE);
    Serial.println("Relay ON");
  } else {
    relaySTATE = LOW;
    digitalWrite(RELAY_PIN, relaySTATE);
    Serial.println("Relay OFF");
  }
  oledUpdate();  // Update the OLED display
}

// When device connects to Blynk Cloud, customize buttons
BLYNK_CONNECTED()
{
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// Interrupt placeholder for button release (unused)
ICACHE_RAM_ATTR void buttonReleasedInterrupt()
{
  // Logic for button release if used
}

// Toggle relay state from code logic
void updateButtonRelay()
{
  relaySTATE = !relaySTATE;
  digitalWrite(RELAY_PIN, relaySTATE);
  Blynk.virtualWrite(V3, relaySTATE);
}

// Update OLED display with current values
void oledUpdate()
{
  oledDisplay.clearDisplay();
  oledTextWrite("BLYNK Smart Plug App", 0, 0, 1);

  char msg[128];
  sprintf(msg, "SERVO 1: %d", servoSTATE1);
  oledTextWrite(msg, 10, 8, 1);

  sprintf(msg, "SERVO 2: %d", servoSTATE2);
  oledTextWrite(msg, 10, 16, 1);

  if (relaySTATE == HIGH) {
    oledTextWrite("RELAY: ON", 10, 24, 1);
  } else {
    oledTextWrite("RELAY: OFF", 10, 24, 1);
  }
  oledDisplay.display();
}

// Helper function to print text to OLED screen
void oledTextWrite(const char *text, int x, int y, int size)
{
  oledDisplay.setTextSize(size);
  oledDisplay.setTextColor(SSD1306_WHITE);
  oledDisplay.setCursor(x, y);
  oledDisplay.println(text);
}

// Called every second to send device uptime to Blynk
void myTimerEvent()
{
  Blynk.virtualWrite(V2, millis() / 1000);
}

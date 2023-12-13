#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "ESP8266-AP";   // SSID for the AP
const char* password = "password";  // Password for the AP
const char* server_ip = "192.168.4.2";  // IP address of the client ESP8266

const int multiplexerS0 = D1;  // Multiplexer S0 pin
const int multiplexerS1 = D2;  // Multiplexer S1 pin
const int multiplexerS2 = D3;  // Multiplexer S2 pin
const int multiplexerS3 = D4;  // Multiplexer S3 pin
const int multiplexerSignal = A0;  // Multiplexer signal pin

const int button1Pin = D5;  // Button 1 pin
const int button2Pin = D6;  // Button 2 pin
const int button3Pin = D7;  // Button 3 pin
const int switchPin = D8;   // Switch pin

const int redLedPin = D9;    // Red LED pin
const int greenLedPin = D10;  // Green LED pin

void setup() {
  Serial.begin(115200);
  
  // Set up the ESP8266 as an Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Set up the multiplexer pins
  pinMode(multiplexerS0, OUTPUT);
  pinMode(multiplexerS1, OUTPUT);
  pinMode(multiplexerS2, OUTPUT);
  pinMode(multiplexerS3, OUTPUT);

  // Set up the button and switch pins
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);

  // Set up the LED pins
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // Turn on the green LED to indicate standby
  digitalWrite(greenLedPin, HIGH);
}

void loop() {
  int sensorA = getDistance(0) <= 15 ? 1 : 0;
  int sensorB = getDistance(1) <= 15 ? 1 : 0;
  int sensorC = getDistance(2) <= 15 ? 1 : 0;
  int sensorD = getDistance(3) <= 15 ? 1 : 0;

  int button1State = digitalRead(button1Pin);
  int button2State = digitalRead(button2Pin);
  int button3State = digitalRead(button3Pin);
  int switchState = digitalRead(switchPin);

  int command = 0;

  // Check the state of the switch to determine whether to use sensors or buttons
  if (switchState == LOW) {  // LOW indicates that the switch is ON
    // Determine the command based on button states
    if (button1State == LOW) {
      command = 1;
    } else if (button2State == LOW) {
      command = 2;
    } else if (button3State == LOW) {
      command = 3;
    }
  } else {
    // Determine the command based on ultrasonic sensor readings
    if (sensorA - sensorB > 0) {
      command = 1;
    } else if (sensorA - sensorC > 0) {
      command = 2;
    } else if (sensorA - sensorD > 0) {
      command = 3;
    }
  }

  // Turn on the red LED to indicate operation
  digitalWrite(redLedPin, HIGH);
  // Send the command to the second ESP8266
  if (sendCommand(command)) {
    Serial.println("Going to sleep");
    // Turn off the red LED and turn on the green LED to indicate standby
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    ESP.deepSleep(0);  // Sleep indefinitely
  }

  delay(5000); // Adjust delay as needed
}

bool sendCommand(int command) {
  HTTPClient http;

  // Use the IP address of the client ESP8266
  String url = "http://" + String(server_ip) + "/process_command?cmd=" + String(command);

  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("Command sent successfully");

    // Check for "ok" response from the second ESP8266
    if (http.getString() == "ok") {
      Serial.println("Confirmation received, going to sleep");
      return true;
    } else {
      Serial.println("Error: No confirmation received");
      return false;
    }
  } else {
    Serial.println("Error sending command");
    return false;
  }
  
  http.end();
}

long getDistance(int channel) {
  // Select the multiplexer channel
  digitalWrite(multiplexerS0, channel & 0x01);
  digitalWrite(multiplexerS1, (channel >> 1) & 0x01);
  digitalWrite(multiplexerS2, (channel >> 2) & 0x01);
  digitalWrite(multiplexerS3, (channel >> 3) & 0x01);

  // Measure distance using the HC-SR04 ultrasonic sensor
  digitalWrite(multiplexerSignal, HIGH);
  delayMicroseconds(10);
  digitalWrite(multiplexerSignal, LOW);
  
  return pulseIn(multiplexerSignal, HIGH) / 58.2;  // Convert pulse duration to distance in cm
}

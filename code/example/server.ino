#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32-AP";   // SSID for the AP
const char* password = "password";  // Password for the AP
const char* server_ip = "192.168.4.1";  // IP address of the AP (1st ESP32)

const int numSensors = 12;  // Total number of sensors including the master

const int triggerPins[numSensors] = {2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
const int echoPins[numSensors] = {15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32};

const char sensorNames[numSensors] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'};

const int masterTriggerPin = 14;  // Master trigger pin
const int masterEchoPin = 32;    // Master echo pin

const int redLedPin = 33;    // Red LED pin
const int greenLedPin = 34;  // Green LED pin

WiFiServer server(80); // 80 is the port number

void setup() {
  Serial.begin(115200);

  // Set up the ESP32 as an Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Set up the trigger pins as OUTPUT
  for (int i = 0; i < numSensors; i++) {
    pinMode(triggerPins[i], OUTPUT);
  }

  // Set up the echo pins as INPUT
  for (int i = 0; i < numSensors; i++) {
    pinMode(echoPins[i], INPUT);
  }

  // Set up the master trigger pin as OUTPUT
  pinMode(masterTriggerPin, OUTPUT);

  // Set up the master echo pin as INPUT
  pinMode(masterEchoPin, INPUT);

  // Set up the LED pins
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // Turn on the green LED to indicate standby
  digitalWrite(greenLedPin, HIGH);
}

void loop() {
  // Check master distance only once
  int masterDistance = getDistance(masterTriggerPin, masterEchoPin);

  // If master distance is less than or equal to 10cm for around 15 seconds
  if (masterDistance <= 10 && millis() < 15000) {
    int command = 0;

    // Array to store the status of each sensor (1 if distance <= 10cm, 0 otherwise)
    int sensorStatus[numSensors] = {0};

    // Loop through all sensors
    for (int i = 0; i < numSensors; i++) {
      int currentDistance = getDistance(triggerPins[i], echoPins[i]);

      // Assign a value of 1 to each sensor if its distance is less than or equal to 10 cm
      sensorStatus[i] = (currentDistance <= 10) ? 1 : 0;
    }

    // Loop through all sensor pairs (A + B, A + C, ..., A + L)
    for (int i = 1; i < numSensors; i++) {
      if (sensorStatus[0] + sensorStatus[i] > 1) {
        command = i + 1;  // Increment by 1 to match your sensor indexing
        break;
      }
    }

    // Turn on the red LED to indicate operation
    digitalWrite(redLedPin, HIGH);

    // Send the command to the second ESP32 with retry mechanism
    for (int retry = 0; retry < 5; retry++) {
      if (sendCommandWithRetry(command)) {
        Serial.println("Going to sleep");
        // Turn off the red LED and turn on the green LED to indicate standby
        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH);
        ESP.deepSleep(0);  // Sleep indefinitely
      } else {
        Serial.println("Retrying...");
        delay(1000);  // Wait for 1 second before retrying
      }
    }

    // If all retries failed, re-read the master status
    Serial.println("All retries failed. Re-reading master status...");
  } else {
    // Reset the timer and turn off the red LED
    digitalWrite(redLedPin, LOW);
    // Add any additional logic if needed
  }

  delay(1000); // Adjust delay as needed
}

bool sendCommandWithRetry(int command) {
  HTTPClient http;

  // Use the IP address of the client ESP32
  String url = "http://" + String(server_ip) + "/process_command?cmd=" + String(command);

  // Retry for a maximum of 4 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 4000) {
    http.begin(url);
    int httpCode = http.GET();
  
    if (httpCode > 0) {
      Serial.println("Command sent successfully");

      // Check for "ok" response from the second ESP32
      if (http.getString() == "ok") {
        http.end();
        return true;
      } else {
        Serial.println("Error: Invalid confirmation received");
      }
    } else {
      Serial.println("Error sending command");
    }

    http.end();
    delay(500);  // Wait for 500 milliseconds before retrying
  }

  return false;  // Retry limit reached
}

long getDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  return pulseIn(echoPin, HIGH) / 58.2;  // Convert pulse duration to distance in cm
}

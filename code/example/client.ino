#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32-AP";   // SSID for the AP
const char* password = "password";  // Password for the AP
const char* server_ip = "192.168.4.1";  // IP address of the AP (1st ESP32)

WiFiClient client;

const int redLedPin = 33;    // Red LED pin
const int greenLedPin = 34;  // Green LED pin

WiFiServer server(80); // 80 is the port number

void setup() {
  Serial.begin(115200);
  
  // Connect the ESP32 to the Access Point
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up the LED pins
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    // Turn on the green LED to indicate operation
    digitalWrite(greenLedPin, HIGH);

    while (client.connected()) {
      if (client.available()) {
        String command = client.readStringUntil('\r');
        int data = processData(command.toInt());

        Serial.print("Received Command: ");
        Serial.println(command);
        Serial.print("Data to Serial: ");
        Serial.println(data);

        // Respond with "ok" to confirm data reception
        sendConfirmation();

        // Turn off the green LED and turn on the red LED to indicate standby
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, HIGH);
        client.stop();
      }
    }
  }
}

int processData(int command) {
  int data = 0;
  
  switch (command) {
    case 1:
      data = 1000;
      break;
    case 2:
      data = 1500;
      break;
    case 3:
      data = 2000;
      break;
    default:
      break;
  }

  return data;
}

void sendConfirmation() {
  HTTPClient http;

  // Use the IP address of the first ESP32
  String url = "http://" + String(server_ip) + "/confirmation";

  http.begin(url);
  int httpCode = http.GET();

  // Check for a successful HTTP GET request
  if (httpCode > 0) {
    Serial.println("Confirmation sent successfully");
  } else {
    Serial.println("Error sending confirmation");
  }

  http.end();
}


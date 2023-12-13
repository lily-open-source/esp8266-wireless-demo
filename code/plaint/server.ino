#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "ESP8266-AP";   // SSID for the AP
const char* password = "password";  // Password for the AP
const char* server_ip = "192.168.4.2";  // IP address of the client ESP8266

void setup() {
  Serial.begin(115200);
  
  // Set up the ESP8266 as an Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  int sensorA = analogRead(A0);
  int sensorB = analogRead(A1);
  int sensorC = analogRead(A2);
  int sensorD = analogRead(A3);

  int command = 0;

  // Determine the command based on sensor readings
  if (sensorA - sensorB > 0) {
    command = 1;
  } else if (sensorA - sensorC > 0) {
    command = 2;
  } else if (sensorA - sensorD > 0) {
    command = 3;
  }

  // Send the command to the second ESP8266
  sendCommand(command);
  delay(5000); // Adjust delay as needed
}

void sendCommand(int command) {
  HTTPClient http;

  // Use the IP address of the client ESP8266
  String url = "http://" + String(server_ip) + "/process_command?cmd=" + String(command);

  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("Command sent successfully");
  } else {
    Serial.println("Error sending command");
  }
  
  http.end();
}

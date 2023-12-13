#include <ESP8266WiFi.h>

const char* ssid = "ESP8266-AP";   // SSID for the AP
const char* password = "password";  // Password for the AP
const char* server_ip = "192.168.4.1";  // IP address of the AP (1st ESP8266)

WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  // Connect the ESP8266 to the Access Point
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up a server to listen for commands
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String command = client.readStringUntil('\r');
        int data = processData(command.toInt());

        Serial.print("Received Command: ");
        Serial.println(command);
        Serial.print("Data to Serial: ");
        Serial.println(data);

        client.println("Data received successfully");
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

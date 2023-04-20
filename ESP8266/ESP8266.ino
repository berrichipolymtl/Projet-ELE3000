#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>


const int LED_PIN = 16; 
const int udpPort = 12345;

WiFiUDP udp;

int wave_period = 0;
int led_on_time = 0;
int led_off_time = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin("robots", "HKHKRR4857GG"); 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }  

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (udp.begin(udpPort)) {
    Serial.println("UDP server started");
  } else {
    Serial.println("Failed to start UDP server");
  }
}

void loop() {
  // Check if there is a UDP packet available
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[packetSize];
    udp.read(packetBuffer, packetSize);

    // Parse the JSON message
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, packetBuffer);
    if (error) {
      Serial.println("Failed to parse JSON message");
      return;
    }

    // Get the device index, wave period, led on time, and led off time from the JSON message
    int device_index = doc["device_index"];
    wave_period = doc["wave_period"];
    led_on_time = doc["led_on_time"];
    led_off_time = doc["led_off_time"];

    // Calculate the delay for this device based on its index
    int device_delay = wave_period / 8 * device_index;

    // Turn on the LED after the device-specific delay
    digitalWrite(LED_PIN, HIGH);
    delay(led_off_time);
    digitalWrite(LED_PIN, LOW);
    delay(led_on_time);
  }
}

  

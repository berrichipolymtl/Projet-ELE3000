#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h> 
#include <WebSocketsServer.h>

const char* ssid = "robots";
const char* password = "HKHKRR4857GG";

const IPAddress esp8266IPs[] = {
  IPAddress(132, 207, 155, 98),
  IPAddress(132, 207, 155, 99),
  IPAddress(132, 207, 155, 100),
  IPAddress(132, 207, 155, 101),
  IPAddress(132, 207, 155, 102),
  IPAddress(132, 207, 155, 103),
  IPAddress(132, 207, 155, 104),
  IPAddress(132, 207, 155, 105)
};

const int udpPort = 12345;

uint32_t wavePeriod = 2000; // ms
uint32_t ledOnTime =200; // ms
uint32_t ledOffTime = 1800; // ms

WiFiUDP udp;


AsyncWebServer server(80);
WebSocketsServer webSocket(81);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>

  <title>Serveur Projet ELE3000</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
  <style>
    body {
      background-color: lightblue;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Serveur Projet ELE3000</h1>
    <form action="/setValues" method="post">
      <div class="form-group">
        <label for="lapTime">Lap time (ms):</label>
        <input type="number" class="form-control" name="lapTime" id="lapTime" required>
      </div>
      <button type="submit" class="btn btn-default">Submit</button>
    </form>
    <div id="lapTimeDisplay" style="font-size: 24px; font-weight: bold;"></div>
  </div>
  <script>
     var ws;
  var lapTimeDisplay = document.getElementById("lapTimeDisplay");

  function displayLapTime(lapTime) {
    lapTimeDisplay.innerHTML = "Lap time: " + lapTime + " ms";
  }

  function setupWebSocket() {
    ws = new WebSocket("ws://" + window.location.hostname + ":81/");
    ws.onopen = function() {
      console.log("Connected to WebSocket server");
    };

    ws.onmessage = function(evt) {
      console.log("Received lap time: ", evt.data);
      displayLapTime(evt.data);
    };

    ws.onclose = function() {
      console.log("Disconnected from WebSocket server");
      setTimeout(setupWebSocket, 3000);
    };
  }
    setupWebSocket();
  </script>
</body>
</html>
)rawliteral";

const int TRIGGER_PIN = 23;
const int ECHO_PIN = 22;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  udp.begin(udpPort);

 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  
 server.on("/setValues", HTTP_POST, [](AsyncWebServerRequest *request) {
  if (request->hasParam("lapTime", true)) {
    wavePeriod = request->getParam("lapTime", true)->value().toInt();
    ledOnTime = wavePeriod/8;
    ledOffTime = wavePeriod - ledOnTime;
    request->send(200, "text/plain", "Values updated successfully");
  } else {
    request->send(400, "text/plain", "Missing parameters");
  }
});

 
  server.begin();
  webSocket.begin();

  pinMode(23, OUTPUT);
  pinMode(22, INPUT);
}


}

void loop() {
  uint32_t startTime = millis();
  for (int i = 0; i < 8; i++) {
    DynamicJsonDocument doc(1024);
    doc["device_index"] = i;
    doc["wave_period"] = wavePeriod;
    doc["led_on_time"] = ledOnTime;
    doc["led_off_time"] = ledOffTime;

    String message;
    serializeJson(doc, message);
    Serial.println(message);

   
    udp.beginPacket(esp8266IPs[i], udpPort);
    udp.print(message);
    udp.endPacket();
    delay(wavePeriod / 8);
  }

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  uint32_t duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0344 / 2;

 uint32_t lapTime = millis() - startTime;
String lapTimeDisplay = "Lap time: " + String(lapTime) + " ms";
webSocket.broadcastTXT(lapTimeDisplay);
delay(500); 

}

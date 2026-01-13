#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "MAX30105.h"
#include "heartRate.h"

const char* ssid = "ESP32-TEST";
const char* password = "12345678";

MAX30105 particleSensor;

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred

int i = 0;

float beatsPerMinute;
int beatAvg;


AsyncWebServer server(80);
AsyncEventSource events("/events");

int counter = 1;
unsigned long lastSend = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Auto Counter</title>
  <style>
    body { font-family: Arial; text-align:center; margin-top:40px; }
    .num { font-size:64px; font-weight:bold; }
  </style>
</head>
<body>
  <h1>ESP32 Counter Test</h1>
  <div class="num" id="num">-</div>

<script>
  const evt = new EventSource('/events');

  evt.onmessage = function(e) {
    const data = JSON.parse(e.data);
    document.getElementById("num").innerText = data.value;
  };
</script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  
  // Connect to Wi-Fi & Web-Server setup
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient *client){
    // kirim nilai awal
    DynamicJsonDocument doc(64);
    doc["value"] = counter;
    String msg;
    serializeJson(doc, msg);
    client->send(msg.c_str(), NULL, millis());
  });

  server.addHandler(&events);
  server.begin();

  // Initialize sensor
  Wire.begin();
  Wire.setClock(400000); // I2C 400kHz
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();                     //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED
}

void loop() {
    // Sensor reading
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;  //Store this reading in the array
      rateSpot %= RATE_SIZE;                     //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
      }
    }
  
  // WebSocket event sending
  if (millis() - lastSend >= 1000) { // update tiap 1 detik
    lastSend = millis();

    DynamicJsonDocument doc(128);
    doc["value"] = counter;
    doc["ir"] = irValue;
    doc["bpm"] = beatsPerMinute;
    doc["avg_bpm"] = beatAvg;


    String msg;
    serializeJson(doc, msg);
    events.send(msg.c_str(), NULL, millis());

    counter++;
    if (counter > 20) counter = 1;
  }

  // Debug Serial Output
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
  delay(10); 
}

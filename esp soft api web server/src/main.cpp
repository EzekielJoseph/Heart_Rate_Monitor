#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "MAX30105.h"
#include "heartRate.h"

const char* ssid = "ESP32-TEST";
const char* password = "12345678";

MAX30105 particleSensor;
AsyncWebServer server (80);

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred

float beatsPerMinute = 0;
int beatAvg = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  
  // Wi-Fi AP
  WiFi.softAP(ssid, password);
  Serial.println("AP Started");
  Serial.println(WiFi.softAPIP());

  // API BPM
  server.on("/bpm", HTTP_GET, [] (AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(128);
    doc[""] = beatAvg;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", String(beatAvg));
  });

  // Begin Server
  server.begin();
  Serial.println("HTTP server started");

  // Initialize sensor
  Wire.begin();
  Wire.setClock(400000); // I2C 400kHz
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }

  particleSensor.setup();                     //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED

  Serial.println("Place your index finger on the sensor with steady pressure.");
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

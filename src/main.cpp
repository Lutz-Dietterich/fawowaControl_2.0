#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <LittleFS.h>


#define DHTPIN 2
#define RELAIS_PIN 5 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float targetTemp = 26.0;
float targetHumidity = 60.0;

char ssid[32];      // Für den SSID-Speicherplatz
char password[64];  // Für das Passwort-Speicherplatz


ESP8266WebServer server(80);

void handleRoot();
void handleData();
void handleSetTargets();

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(RELAIS_PIN, OUTPUT);


// Initialisieren Sie das LittleFS-Dateisystem
  if (LittleFS.begin()) {
    Serial.println("LittleFS initialisiert");

    // Öffnen Sie die Konfigurationsdatei zum Lesen
    File configFile = LittleFS.open("/config.txt", "r");
    if (configFile) {
      // Lesen Sie den SSID-Namen und das Passwort aus der Datei
      String ssidFromFile = configFile.readStringUntil('\n');
      String passwordFromFile = configFile.readStringUntil('\n');
      configFile.close();

      if (ssidFromFile.length() > 0 && passwordFromFile.length() > 0) {
        ssidFromFile.toCharArray(ssid, sizeof(ssid));
        passwordFromFile.toCharArray(password, sizeof(password));
        Serial.println("SSID und Passwort aus der Datei geladen.");
      } else {
        Serial.println("Ungültige Daten in der Datei.");
      }
    } else {
      Serial.println("Fehler beim Öffnen der Datei.");
    }
  } else {
    Serial.println("Fehler beim Initialisieren des LittleFS-Dateisystems");
  }

    WiFi.begin(ssid, password);


  // Überprüfen Sie den Status der WLAN-Verbindung
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Verbunden mit WLAN");
  } else {
    Serial.println("Verbindung zum WLAN fehlgeschlagen");
  }

  Serial.println("ssid: "); Serial.println(ssid);
  Serial.println("password: "); Serial.println(password);


  // Starten Sie den Webserver
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/setTargets", HTTP_GET, handleSetTargets);
  server.on("/styles.css", HTTP_GET, [](){
    File file = LittleFS.open("/styles.css", "r");
    size_t sent = server.streamFile(file, "text/css");
    file.close();
  });

  server.begin();
}



void loop() {
  server.handleClient();
}

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "Datei nicht gefunden");
    return;
  }

  server.streamFile(file, "text/html");
  file.close();
}

void handleSetTargets() {
  if (server.hasArg("temp") && server.hasArg("humidity")) {
    targetTemp = server.arg("temp").toFloat();
    targetHumidity = server.arg("humidity").toFloat();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Fehlende Argumente");
  }
}

void handleData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (temp > targetTemp || humidity > targetHumidity) {
    digitalWrite(RELAIS_PIN, LOW);
    Serial.println("Relais EIN");
  } else {
    digitalWrite(RELAIS_PIN, HIGH);
    Serial.println("Relais AUS");
  }

  if (isnan(temp) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\": \"Sensor lesen fehlgeschlagen\"}");
    return;
  }
  
  String jsonResponse = "{\"temp\": ";
  jsonResponse += String(temp);
  jsonResponse += ", \"humidity\": ";
  jsonResponse += String(humidity);
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}

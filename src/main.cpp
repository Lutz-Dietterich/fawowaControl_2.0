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
  server.begin();
}



void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<html>";
  html += "<head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Temperatur- und Feuchtigkeitsüberwachung</title>";  // Titel der Webseite

  // CSS im <head>-Bereich
  html += "<style>";
  html += "body {background-image: url('https://res.cloudinary.com/dnojoo4vt/image/upload/v1685634485/samples/landscapes/nature-mountains.jpg'); background-size: cover; background-position: center; background-repeat: no-repeat;}";
  html += ".wrapper {display: flex; justify-content: center; flex-direction: column; background-color: rgba(255,255,255,0.9); width: 50%; margin: 5vh auto 0 auto; padding: 20px; border-radius: 5px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.3); }";
  html += ".header-wrapper { display: flex; justify-content: center; width: 50%; margin: auto; }";
  html += ".ist { display: flex; justify-content: space-evenly; flex-direction: row; background-color: blue; witdh: 80%; }";
  html += ".istWerte { display: flex; justify-content: center; flex-direction: column; background-color: green; witdh: 30%; padding: 10px;}";
  html += ".column { flex-direction: column; }";
  html += ".button-wrapper {display: flex; flex-direction: row-reverse; justify-content: flex-end; }";
  html += ".button { width: 3rem; height: 3rem; margin: 5px; border: none; border-radius: 5px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.5);  }";
  html += ".button:hover { background-color: rgba(0,128,0,0.3); }"; 
  html += ".button:active { transform: scale(0.95); }"; 
  html += "h2 {padding: 0 10px 0 10px; margin: 0 0 0 0; }";

  // Media Query für Bildschirmbreiten unter 768 Pixeln
  html += "@media (max-width: 768px) {";
  html += ".wrapper { width: 80%; }";
  html += ".header-wrapper { width: 80%; }";
  html += "}";

  // Media Query für Bildschirmbreiten unter 480 Pixeln
  html += "@media (max-width: 480px) {";
  html += ".wrapper { width: 95%; padding: 10px; }";
  html += ".button { width: 4rem; height: 4rem; }";
  html += "}";

  html += "</style>";

  html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.min.js'></script>";

  html += "</head>";

  html += "<body style='width: 100vw; font-family: sans-serif; '>";
  html += "<div class='wrapper'>";
  html += "<div class='header-wrapper'>";
    html += "<header>";
      html += "<h1>Fahrradwohnwagen Steuerung</h1>";
    html += "</header>";
  html += "</div>";

  html += "<div class='ist'>";
    html += "<div class='istWerte'>";
    html += "<script src='https://code.jquery.com/jquery-3.6.0.min.js'></script>";
    html += "<h3 style='color: red;' id='temp'>Temperatur: </h2>";
    html += "<h3 id='humidity'>Luftfeuchtigkeit: </h2>";
    html += "</div>";

    html += "<div class='istWerte'>";
    html += "<h3 style='color: red;' id='temp'>Temperatur: </h2>";
    html += "<h3 id='humidity'>Luftfeuchtigkeit: </h2>";
    html += "</div>";
  html += "</div>";


  // Buttons zum Ändern der Sollwerte
  html += "<h3>Temperatur-Sollwert: <span id='targetTemp'></span></h3>";
  html += "<div class='button-wrapper'>";
  html += "<button class='button' onclick='adjustTargetTemp(1)'>+</button>";
  html += "<button class='button' onclick='adjustTargetTemp(-1)'>-</button>";
  html += "</div>";
  
  html += "<h3>Luftfeuchtigkeits-Sollwert: <span id='targetHumidity'></span></h3>";
  html += "<button class='button' onclick='adjustTargetHumidity(1)'>+</button>";
  html += "<button class='button' onclick='adjustTargetHumidity(-1)'>-</button>";

  html += "<div>";
  html += "<canvas id='tempChart' width='400' height='200'></canvas>";
  html += "<canvas id='humidityChart' width='400' height='200'></canvas>";
  html += "</div>";


  html += "<script>";
  html += "var targetTemp = 26;";
  html += "var targetHumidity = 60;";
  html += "document.getElementById('targetTemp').innerText = targetTemp;";
  html += "document.getElementById('targetHumidity').innerText = targetHumidity;";
  
  html += "function adjustTargetTemp(delta) {";
  html += "  targetTemp += delta;";
  html += "  document.getElementById('targetTemp').innerText = targetTemp;";
  html += "  updateTargets();";
  html += "}";

  html += "function adjustTargetHumidity(delta) {";
  html += "  targetHumidity += delta;";
  html += "  document.getElementById('targetHumidity').innerText = targetHumidity;";
  html += "  updateTargets();";
  html += "}";
  
  html += "function updateTargets() {";
  html += "  $.get('/setTargets?temp=' + targetTemp + '&humidity=' + targetHumidity);";
  html += "}";

  html += "setInterval(function(){";
  html += "  $.get('/data', function(data) {";
  html += "    $('#temp').text('Temperatur: ' + data.temp + ' C');";
  html += "    $('#humidity').text('Luftfeuchtigkeit: ' + data.humidity + ' %');";
  html += "  });";
  html += "}, 3000);";

  html += "</script>";

  html += "<script>";
  html += "var tempChart = new Chart(document.getElementById('tempChart'), {";
  html += "  type: 'line',";
  html += "  data: {";
  html += "    labels: [],";
  html += "    datasets: [{";
  html += "      label: 'Temperatur',";
  html += "      data: [],";
  html += "      backgroundColor: 'rgba(255, 99, 132, 0.2)',";
  html += "      borderColor: 'rgba(255, 99, 132, 1)',";
  html += "      borderWidth: 1";
  html += "    }]";
  html += "  },";
  html += "  options: {";
  html += "    scales: {";
  html += "      yAxes: [{";
  html += "        ticks: {";
  html += "          beginAtZero: true";
  html += "        }";
  html += "      }]";
  html += "    }";
  html += "  }";
  html += "});";

  html += "var humidityChart = new Chart(document.getElementById('humidityChart'), {";
  html += "  type: 'line',";
  html += "  data: {";
  html += "    labels: [],";
  html += "    datasets: [{";
  html += "      label: 'Luftfeuchtigkeit',";
  html += "      data: [],";
  html += "      backgroundColor: 'rgba(54, 162, 235, 0.2)',";
  html += "      borderColor: 'rgba(54, 162, 235, 1)',";
  html += "      borderWidth: 1";
  html += "    }]";
  html += "  },";
  html += "  options: {";
  html += "    scales: {";
  html += "      yAxes: [{";
  html += "        ticks: {";
  html += "          beginAtZero: true";
  html += "        }";
  html += "      }]";
  html += "    }";
  html += "  }";
  html += "});";

  html += "setInterval(function(){";
  html += "  $.get('/data', function(data) {";
  html += "    tempChart.data.labels.push(new Date().toLocaleTimeString());";
  html += "    tempChart.data.datasets[0].data.push(data.temp);";
  html += "    tempChart.update();";
  html += "    humidityChart.data.labels.push(new Date().toLocaleTimeString());";
  html += "    humidityChart.data.datasets[0].data.push(data.humidity);";
  html += "    humidityChart.update();";
  // html += "    localStorage.setItem('tempReadings', JSON.stringify(tempChart.data.datasets[0].data));";
  // html += "    localStorage.setItem('humidityReadings', JSON.stringify(humidityChart.data.datasets[0].data));";
  html += "  });";
  html += "}, 6000);";
  html += "</script>";

  


  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
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

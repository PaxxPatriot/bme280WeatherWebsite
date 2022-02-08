#include "secrets.h"

#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


Adafruit_BME280 bme;

struct Measurement {
  float  temperature;
  float  humidity;
  float  pressure;
};

struct Measurement pastValues[20];

const long interval = 3600000; // 60 minutes interval
unsigned long previousMillis = 0;

ESP8266WebServer server(80);

// const char* ssid = "WLAN-735353";
auto ssid = SECRET_SSID;
auto password = SECRET_PASSWD;
const char* hostname = "esp8266";

void setup() {
  Serial.begin(115200);
  delay(100);

  // Array initialisieren
  for (int i = 0; i < 20; i++) {
    pastValues[i] = Measurement{0.0, 0.0, 0.0};
  }


  bme.begin(0x76);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}
void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 1; i < 20; i++) {
      pastValues[i - 1] = pastValues[i];
    }
    pastValues[19] = (Measurement) getMeasurement();
  }
}

Measurement getMeasurement() {
  struct Measurement measurement;
  measurement.temperature = bme.readTemperature();
  measurement.humidity = bme.readHumidity();
  measurement.pressure = bme.readPressure() / 100.0F;
  return measurement;
}

void handle_OnConnect() {
  struct Measurement measurement = getMeasurement();
  server.send(200, "text/html", SendHTML(measurement.temperature, measurement.humidity, measurement.pressure));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float humidity, float pressure) {
  String ptr = "<!DOCTYPE html> <html lang=\"de\">\n";
  ptr += "<head><meta charset=\"utf-8\"/><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 Wetterstation</title>\n";
  ptr += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.6.0/chart.min.js\" integrity=\"sha512-GMGzUEevhWh8Tc/njS0bDpwgxdCJLQBWG3Z2Ct+JGOpVnEmjvNx6ts4v6A2XJf1HOrtOsfhv3hBKpK9kE5z8AQ==\" crossorigin=\"anonymous\" referrerpolicy=\"no-referrer\"></script>";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;margin-bottom: 10px;}\n";
  ptr += "canvas {margin: 0 auto;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Wetterstation</h1>\n";
  ptr += "<p>Temperatur: ";
  ptr += temperature;
  ptr += "&deg;C</p>";
  ptr += "<p>Relative Luftfeuchtigkeit: ";
  ptr += humidity;
  ptr += "%</p>";
  ptr += "<p>Luftdruck: ";
  ptr += pressure;
  ptr += "hPa</p>";
  ptr += "</div>\n";
  ptr += "<canvas id=\"weatherData\" style=\"display: block; width: 1000px; height: 600px;\"></canvas>";
  ptr += "<script>";
  ptr += "const ctx = document.getElementById('weatherData').getContext('2d');";
  ptr += "const weatherData = new Chart(ctx, {";
  ptr += "type: 'line',";
  ptr += "data: {";
  ptr += "labels: ['vor 20 Std.', 'vor 19 Std.', 'vor 18 Std.', 'vor 17 Std.', 'vor 16 Std.', 'vor 15 Std.', 'vor 14 Std.', 'vor 13 Std.', 'vor 12 Std.', 'vor 11 Std.', 'vor 10 Std.', 'vor 9 Std.', 'vor 8 Std.', 'vor 7 Std.', 'vor 6 Std.', 'vor 5 Std.', 'vor 4 Std.', 'vor 3 Std.', 'vor 2 Std.', 'vor 1 Std.'],";
  ptr += "datasets: [";
  ptr += "{label: 'Temperatur',data: [";
  for (int i = 0; i < 20; i++) {
    struct Measurement m;
    m = pastValues[i];
    ptr +=  String(m.temperature);
    ptr += ",";
  }
  ptr += "],backgroundColor: 'rgb(255, 0, 0)', borderColor: 'rgb(0, 0, 0)'},";
  ptr += "{label: 'relative Luftfeuchtigkeit',data: [";
  for (int i = 0; i < 20; i++) {
    struct Measurement m;
    m = pastValues[i];
    ptr +=  String(m.humidity);
    ptr += ",";
  }
  ptr += "],backgroundColor: 'rgb(0, 255, 0)', borderColor: 'rgb(0, 0, 0)'}";
  ptr += "]},options: {scales: {yAxis: {type: 'logarithmic',}}}});";
  ptr += "</script>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

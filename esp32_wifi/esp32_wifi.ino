// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "vector.h"

#define DELAY_WIFI 500
#define TIMEOUT_WIFI 20
#define PARAM_SEPARATOR ','

#define LENGTH_BYTES 6

AsyncWebServer server(80);

const char* ssid_ap = "esp32";
const char* pwd_ap = "12345678";

String ssid_connect = "";
String pwd_connect = "";
bool connect_to_wifi = false;

IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Connect To WiFi</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FFFFFF;}
  </style>
  </head><body>
  <h2>HTML Form to Input Data</h2> 
  <form action="/connect">
    SSID: <input type="text" name="ssid_string"><br>
    PWD: <input type="text" name="pwd_string"><br>
    <input type="submit" value="Connect">
  </form><br>
</body></html>)rawliteral";


Vector<String> parseParameters(const String&);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
}


void loop() {
  String received;
  char cmd;

  if (connect_to_wifi) {
    WiFi.softAPdisconnect(true);
    if (!connectToWiFi(ssid_connect, pwd_connect))
      setAsAP();
    connect_to_wifi = false;
  }

  if (Serial2.available()) {
    received = Serial2.readString();
    received.trim();
    cmd = received.substring(0, 1)[0];
    ssize_t params_size = 0;
    Vector<String> params = parseParameters(received.substring(1));
    Serial.println(received);
    Serial.println(cmd);
    switch (cmd) {
      case 'c':
        if (params_size > 2)
          Serial.println("Error en cantidad de parametros");
        disconnectFromWiFi();
        sendToSerial2(connectToWiFi(params[0], params[1]));
        break;
      case 'w':
        if (params_size > 0)
          Serial.println("Error en cantidad de parametros");
        Serial.println(getWiFiStatus());
        sendToSerial2(getWiFiStatus());
        break;
      case 'p':
        if (params_size > 2)
          Serial.println("Error en cantidad de parametros");
        sendToSerial2(post(params[0], params[1]));
        break;
      case 'g':
        if (params_size > 1)
          Serial.println("Error en cantidad de parametros");
        sendToSerial2(get(params[0]));
        break;
      case 'd':
        if (params_size > 0)
          Serial.println("Error en cantidad de parametros");
        sendToSerial2(disconnectFromWiFi());
        break;
      case 'a':
        if (params_size > 0)
          Serial.println("Error en cantidad de parametros");
        setAsAP();
        break;
      // para guardarme la ssid.
    }
    // Serial2.flush();
  }
}

void sendToSerial2(int toSend) {
  char size_buffer[LENGTH_BYTES + 1];
  char str[4];  // harcodeado

  snprintf(size_buffer, sizeof(size_buffer), "%06d", int_length(toSend) + 2);
  Serial2.print(size_buffer);
  Serial2.println(toSend);
}

void sendToSerial2(String toSend) {
  char size_buffer[LENGTH_BYTES + 1];

  snprintf(size_buffer, sizeof(size_buffer), "%06d", toSend.length() + 2);
  Serial2.print(size_buffer);
  Serial2.println(toSend);
}

size_t int_length(uint number) {
  size_t length = 0;

  if (number == 0)
    return 1;

  while (number > 0) {
    number /= 10;
    length++;
  }

  return length;
}

Vector<String> parseParameters(const String & string) {
  size_t parameters_size = 0;
  int index_from = -1, index_to;
  if (string.length() > 0) {
    parameters_size++;
  }
  while ((index_from = string.indexOf(PARAM_SEPARATOR, index_from+1)) >= 0) {
    parameters_size++;
  }

  Vector<String> params(parameters_size);

  index_from = -1;
  for (size_t i = 0; i < parameters_size; i++) {
    index_to = string.indexOf(PARAM_SEPARATOR, index_from+1);
    if (index_to < 0)
      index_to = string.length();
    params[i] = string.substring(index_from+1, index_to);
    index_from = index_to;
  }
  return params;
}


bool connectToWiFi(const String& ssid, const String& pwd) {
  Vector<char> ssid_vector(ssid.length() + 1);
  Vector<char> pwd_vector(pwd.length() + 1);

  ssid.toCharArray(ssid_vector.data(), ssid.length() + 1);
  pwd.toCharArray(pwd_vector.data(), pwd.length() + 1);

  WiFi.begin(ssid_vector.data(), pwd_vector.data());

  int timeout_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(DELAY_WIFI);
    timeout_counter++;
    if (timeout_counter > TIMEOUT_WIFI) {
      return false;
    }
  }
  return true;
}

bool disconnectFromWiFi() {
  bool response = WiFi.disconnect();
  while (WiFi.status() == WL_CONNECTED) {}
  return response;
}

int getWiFiStatus() {
  return WiFi.status();
}

void setAsAP() {
  WiFi.softAP(ssid_ap, pwd_ap);
  WiFi.config(local_ip, gateway, subnet);
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/connect", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("ssid_string") && request->hasParam("pwd_string")) {
      ssid_connect = request->getParam("ssid_string")->value();
      pwd_connect = request->getParam("pwd_string")->value();
    }
    Serial.println(ssid_connect);
    Serial.println(pwd_connect);

    connect_to_wifi = true;
  });
  server.onNotFound(notFound);
  server.begin();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// TODO(matiascharrut) chequear que el wifi este conectado
String post(String server, String request) {
  HTTPClient http;
  Serial.println("Server = " + server);
  Serial.println("Request = " + request);
  http.begin(server);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(request);
  String response = http.getString();
  http.end();
  return response;
}

String get(String url) {
  HTTPClient http;
  http.begin(url);
  http.GET();
  String response = http.getString();
  http.end();
  return response;
}

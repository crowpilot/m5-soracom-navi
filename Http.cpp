#include "Http.h"
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<ArduinoJson.h>
#include<WiFi.h>
#include<Preferences.h>

Http::Http(TinyGsmClientSecure& ctx) {
  init(&ctx);
}

bool Http::init(TinyGsmClientSecure* ctx) {
  this->c = ctx;
  return 1;
}

bool Http::getMap(String path, String filename) {
  if (SD.exists(filename)) {
    return 1;
  }

  Serial.println("Download");
  if (!c->connect("cyberjapandata.gsi.go.jp", 443)) {
    Serial.println(F("Connect failed."));
    return 0;
  }

  c->println(path);
  c->print("Host: ");
  c->println("cyberjapandata.gsi.go.jp");
  c->println();
  //ctx.print("Connection: close\r\n\r\n");
  Serial.println("Header");

  while (c->connected()) {
    String line = c->readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r") {
      Serial.println("header ok");
      break;
    }
  }
  Serial.println("body");

  f = SD.open(filename, FILE_WRITE);
  while (c->connected() && c->available()) {
    char ch = c->read();
    //Serial.print(ch);
    f.print(ch);
  }
  //f.write((const unsigned char *)_buf,strlen(_buf));
  f.close();
  c->stop();
  Serial.println("f close HTTP stop");
  return 1;
}

bool Http::getLocation(float &lat, float &lon) {
  f = SD.open("/apikey.txt");
  String apikey;
  while (f.available()) {
    apikey += char(f.read());
  }
  f.close();

  _prefs.begin("location", true);
  lat = _prefs.getFloat("lat", 34.704);
  lon = _prefs.getFloat("lon", 137.734);
  _prefs.end();
  Serial.println("load Preferenses");

  //Serial.print("GPS modem start");
  //modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
  //WIFI  geo location

  String json;
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no wifi");
    return 0;
  }
  const size_t capacity = JSON_ARRAY_SIZE(3) + (3) * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 34 + (3 * 29);
  DynamicJsonDocument doc(capacity);
  doc["considerIp"] = "false";
  JsonArray wifiAccessPoints = doc.createNestedArray("wifiAccessPoints");

  int breakflg = 0;
  for (int i = 0; i < n; i++) {
    if (i == 3) break;
    String bssid = WiFi.BSSIDstr(i);
    JsonObject wifiAP = wifiAccessPoints.createNestedObject();
    wifiAP["macAddress"] = bssid;
    if (_wifi[i].equals(bssid)) {
      breakflg++;
    }
    _wifi[i] = bssid;
  }
  if (breakflg == 3) {
    Serial.println("same wifi BSSID");
    return 0;
  }
  serializeJson(doc, json);
  Serial.println(json);

  if (!c->connect("www.googleapis.com", 443)) {
    Serial.println(F("Connect failed."));
    return 0;
  }
  Serial.println("google connected");
  String path = "POST /geolocation/v1/geolocate?key=" + apikey + " HTTP/1.1";
  c->println(path);
  c->print("Host: ");
  c->println("www.googleapis.com");
  c->println("Content-Type: application/json");
  c->print("Content-Length: ");
  c->println(json.length());
  c->println();
  c->println(json.c_str());
  //ctx.print("Connection: close\r\n\r\n");
  Serial.println("Header");

  while (c->connected()) {
    String line = c->readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r") {
      Serial.println("header ok");
      break;
    }
  }
  Serial.println("body");
  String locBody;
  c->readStringUntil('\n');
  while (c->connected()) {
    String line = c->readStringUntil('\n');
    Serial.print(line);
    locBody += line;
    if (line == "\r") {
      Serial.println("json body ok");
      break;
    }
  }
  //Serial.println(locBody);
  c->stop();

  Serial.println("json perser");

  const size_t capacity2 = 2 * JSON_OBJECT_SIZE(2) + 30;
  DynamicJsonDocument doc2(capacity2);
  deserializeJson(doc2, locBody);
  if (doc2["location"]["lat"]) {
    Serial.println("wifi get location");
    lat = doc2["location"]["lat"];
    lon = doc2["location"]["lng"];

    _prefs.begin("location", false);
    _prefs.putFloat("lat", lat);
    _prefs.putFloat("lon", lon);
    _prefs.end();
  }
  Serial.println("put Preferenses");

  Serial.print(lat);
  Serial.print(lon);
  return 1;
}

bool getHeader() {
  return 1;
}

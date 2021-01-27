//#define LGFX_AUTODETECT
#include <M5Stack.h>
#include<WiFi.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<string.h>
#include<ArduinoJson.h>
#include <math.h>

#include<SD.h>

#include<LovyanGFX.hpp>

#include "Geo.h"
#include "Http.h"



TinyGsm modem(Serial2); /* 3G board modem */
TinyGsmClientSecure ctx(modem);
Http http(ctx);


File f;

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

int zoom = 14;
String path;
String host = "cyberjapandata.gsi.go.jp";
int x, y;

float lat, lon, speed, alt, accuracy;
int vsat, usat;
int year, month, day, hour, minute, second;

int mcc, mnc;
long lac, cellid;


void setup() {
  M5.begin();
  SD.begin();
  lcd.init();
  sprite.setColorDepth(8);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  //  lcd.setRotation(1);
  // M5.Lcd.setBrightness(128);

  Serial.begin(115200);


  lcd.print(F("modem.restart()"));
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  modem.restart();

  lcd.print(F("getModemInfo:"));
  String modemInfo = modem.getModemInfo();
  lcd.println(modemInfo);

  while (!modem.waitForNetwork()) {
    Serial.print(".");
  }

  modem.gprsConnect("soracom.io", "sora", "sora");

  while (!modem.isNetworkConnected()) {
    Serial.print(".");
  }

  lcd.println(F("enable GPS"));
  modem.enableGPS();

  Serial.println(modem.localIP());

  delay(1000);
  lcd.clear();
}


void loop() {
  //M5.update();
  f = SD.open("/apikey.txt");
  String apikey;
  while (f.available()) {
    apikey += char(f.read());
  }
  f.close();
  Serial.println(apikey);

  Serial.print("GPS modem start");
  //modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
  lat = 34;
  lon = 134;
  //WIFI  geo location

  String json;
  int n = WiFi.scanNetworks();
  const size_t capacity = JSON_ARRAY_SIZE(3) + (3) * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 34 + (3 * 29);
  DynamicJsonDocument doc(capacity);
  doc["considerIp"] = "false";
  JsonArray wifiAccessPoints = doc.createNestedArray("wifiAccessPoints");
  for (int i = 0; i < n; i++) {
    if (i == 3) break;
    String bssid = WiFi.BSSIDstr(i);
    JsonObject wifiAP = wifiAccessPoints.createNestedObject();
    wifiAP["macAddress"] = WiFi.BSSIDstr(i);
  }
  serializeJson(doc, json);
  Serial.println(json);

  if (!ctx.connect("www.googleapis.com", 443)) {
    Serial.println(F("Connect failed."));
    return;
  }
  Serial.println("google connected");
  String path = "POST /geolocation/v1/geolocate?key=" + apikey + " HTTP/1.1";
  ctx.println(path);
  ctx.print("Host: ");
  ctx.println("www.googleapis.com");
  ctx.println("Content-Type: application/json");
  ctx.print("Content-Length: ");
  ctx.println(json.length());
  ctx.println();
  ctx.println(json.c_str());
  //ctx.print("Connection: close\r\n\r\n");
  Serial.println("sent");

  while (ctx.connected()) {
    String line = ctx.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("header ok");
      break;
    }
  }
  Serial.println("body");
  String locBody;
  ctx.readStringUntil('\n');
  while (ctx.connected()) {
    String line = ctx.readStringUntil('\n');
    Serial.print(line);
    locBody += line;
    if (line == "\r") {
      Serial.println("body ok");
      break;
    }
  }
  //Serial.println(locBody);
  ctx.stop();

  Serial.println("stop");

  const size_t capacity2 = 2 * JSON_OBJECT_SIZE(2) + 30;
  DynamicJsonDocument doc2(capacity2);
  deserializeJson(doc2, locBody);
  lat = doc2["location"]["lat"];
  lon = doc2["location"]["lng"];
  accuracy = doc2["accuracy"];

  Serial.print(lat);
  Serial.print(lon);
  //WIFI location

  if (!lat) {
    Serial.println("modem gps");
    modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
  }

  Serial.printf("%f,%f  speed=%f\n accuracy=", lat, lon, speed);
  Serial.println(accuracy);
  lcd.setCursor(0, 0);
  lcd.setTextSize(2);
  lcd.printf("%3.3f%3.3f\n", lat, lon);

  Geo main = Geo(lat, lon, zoom);

  Serial.println(main.filename());
  Serial.println(main.path().c_str());
  //http.getMap(main.path(), main.filename());

  for (int x=-1;x<=1;x++){
    for(int y=-1;y<=1;y++){
      if(0<(main.offsetX()+256*(x+1)) or (main.offsetX()+256*x)<320){
        if(0<(main.offsetY()+256*(y+1))or (main.offsetY()+256*y)<240){
          http.getMap(main.path(x,y),main.filename(x,y));
        }
      }
    }
  }
  
  lcd.clear();

  Serial.println(main.filename());
  //lcd.drawPngFile(SD, main.filename().c_str(), main.offsetX(), main.offsetY());
  for (int x=-1;x<=1;x++){
    for(int y=-1;y<=1;y++){
      if(0<(main.offsetX()+256*(x+1)) or (main.offsetX()+256*x)<320){
        if(0<(main.offsetY()+256*(y+1))or (main.offsetY()+256*y)<240){
          sprite.createSprite(256,256);
          sprite.drawPngFile(SD, main.filename(x,y).c_str(), 0, 0);
          sprite.pushSprite(main.offsetX(x),main.offsetY(y));
          //lcd.drawPngFile(SD, main.filename(x,y).c_str(), main.offsetX(x), main.offsetY(y));
        }
      }
    }
  }
  //   _buf[0] = '\0';
  lcd.drawCircle(160,120,10,TFT_RED);
  lcd.drawCircle(160,120,9,TFT_RED);
  lcd.drawCircle(160,120,8,TFT_RED);
  Serial.println("done");
  while (1) {
    delay(1000 * 600);
  }
}

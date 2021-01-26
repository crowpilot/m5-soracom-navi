//#define LGFX_AUTODETECT
#include <M5Stack.h>
#include<WiFi.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<string.h>
#include<ArduinoJson.h>

#include<SD.h>

#include<LovyanGFX.hpp>

#include <math.h>

TinyGsm modem(Serial2); /* 3G board modem */
TinyGsmClientSecure ctx(modem);

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
    String line=ctx.readStringUntil('\n');
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

  if(!lat){
    Serial.println("modem gps");
    modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
  }

  Serial.printf("%f,%f  speed=%f\n accuracy=", lat, lon, speed);
  Serial.println(accuracy);

  

  lcd.setCursor(0, 0);
  lcd.setTextSize(2);
  lcd.printf("%3.3f%3.3f\n", lat, lon);
  x = pow(2, 13) * (180 + lon) / 180;
  y = pow(2, 13) * (1 - log(tan(lat * 3.14 / 180) + 1 / cos(lat * 3.14 / 180)) / 3.14);
  String openfile = "/" + String(zoom) + "-" + String(x) + "-" + String(y) + ".png";

  if (SD.exists(openfile)) {
    Serial.println("file exists");
  } else {

    path = String("GET /xyz/std/");
    path += String(zoom) + "/";
    path += String(int(x)) + "/";
    path += String(int(y)) + ".png";
    path += " HTTP/1.0";
    Serial.println(path.c_str());

    //M5.M5.Lcd.printf("XYZ=%d,%d/14",int(pow(2,13)*(180+lon)/180),int(pow(2,13)*(1-log(tan(lat*3.14/180)+1/cos(lat*3.14/180))/3.14)));

    if (!ctx.connect("cyberjapandata.gsi.go.jp", 443)) {
      Serial.println(F("Connect failed."));
      return;
    }
    Serial.println("connected");

    ctx.println(path);
    ctx.print("Host: ");
    ctx.println(host);
    ctx.println();
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

    //   ctx.readBytes(_buf, sizeof(_buf));

    uint32_t readLen = 0;
    //String openfile = "/" + String(zoom) + "-" + String(x) + "-" + String(y) + ".png";
    f = SD.open(openfile, FILE_APPEND);
    while (ctx.connected()) {
      char c = ctx.read();
      Serial.print(c);
      f.print(c);
    }
    //f.write((const unsigned char *)_buf,strlen(_buf));
    f.close();
    Serial.println("f close");

    //ctx.readBytes(_buf, sizeof(_buf)); /* body */

    ctx.stop();

    Serial.println("stop ");
  }

  lcd.clear();


  Serial.println(openfile);
  lcd.drawPngFile(SD, openfile.c_str(), 0, 0);
  //   _buf[0] = '\0';
  Serial.println("done");
  while (1) {
    delay(1000 * 600);
  }
}

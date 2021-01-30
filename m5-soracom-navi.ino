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

  Serial.print(F("modem.restart()"));
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  modem.restart();

  Serial.print(F("getModemInfo:"));
  String modemInfo = modem.getModemInfo();
  Serial.println(modemInfo);

  while (!modem.waitForNetwork()) {
    Serial.print(".");
  }

  modem.gprsConnect("soracom.io", "sora", "sora");

  while (!modem.isNetworkConnected()) {
    Serial.print(".");
  }

  lcd.println(F("modem done"));
  

  Serial.println(modem.localIP());

  delay(500);
  lcd.clear();
}


void loop() {
  //M5.update();
  //WIFI location

  http.getLocation(lat,lon);

  if (!lat) {
    Serial.println("modem gps");
    modem.enableGPS();
    modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
    zoom=10;
    
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
  int loadx=-2;
  int loady=-2;
  while (1) {
    M5.update();
    //download sarani surround tile
    if(M5.BtnA.wasPressed()){
      zoom--;
      break;
    }
    if(M5.BtnC.wasPressed()){
      zoom++;
      break;
    }
    //http.getMap(main.path(loadx,loady),main.filename(loadx,loady));
    http.getLocation(lat,lon);
    main.setPlot(lat,lon);
    lcd.drawCircle(main.plotX(),main.plotY(),7,TFT_RED);
    if(main.plotX()<40 or main.plotX()>280){
      break;
    }
    if(main.plotY()<40 or main.plotY()>200){
      break;
    }
    Serial.println("wait 30 sec");
    delay(30*1000);
    if(http.getMap(main.path(loadx,loady),main.filename(loadx,loady))){
      Serial.println("download surround map");
    }else{
      Serial.println("wait 30");
      delay(30*1000);
    }
    Serial.printf("load tile %d,%d",loadx,loady);
    loadx++;
    if(loadx==2){
      loady++;
      loadx=-2;
    }
    if(loady==2){
      loadx=2;
    }
  }
}

//#define LGFX_AUTODETECT
#include <M5Stack.h>
#include<WiFi.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<string.h>
#include<ArduinoJson.h>
#include <math.h>

#include<Preferences.h>

#include<SD.h>

#include<LovyanGFX.hpp>

#include "Geo.h"
#include "Http.h"
#include "Slack.h"



TinyGsm modem(Serial2); /* 3G board modem */
TinyGsmClientSecure ctx(modem);
TinyGsmClient ctx2(modem);
Http http(ctx);
Slack slack(ctx2);


File f;

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

volatile int zoom = 14;
String path;
String host = "cyberjapandata.gsi.go.jp";
int x, y;

float lat, lon, speed, alt, accuracy;
int vsat, usat;
int year, month, day, hour, minute, second;

int mcc, mnc;
long lac, cellid;

//TASK
//button check
void buttonTask(void* arg);
//download surround tiles
void downloadTask(void* arg);

SemaphoreHandle_t xMutex = NULL;


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

  lcd.setCursor(0,100);
  lcd.setTextSize(2);
  lcd.println("NAVIGATION");
  lcd.setTextSize(1);
  lcd.println("chiriin tile");
  lcd.println("https://maps.gsi.go.jp/development/ichiran.html");
  
  lcd.setCursor(0,0);
  lcd.println(F("modem.restart()"));
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  modem.restart();

  lcd.print(F("getModemInfo:"));
  String modemInfo = modem.getModemInfo();
  lcd.println(modemInfo);

  lcd.println("wait for network()");
  while (!modem.waitForNetwork()) {
    lcd.print(".");
  }

  lcd.println("gprs connect");
  modem.gprsConnect("soracom.io", "sora", "sora");

  lcd.println("is network connected");
  while (!modem.isNetworkConnected()) {
    lcd.print(".");
  }

  lcd.println(F("modem done"));

  lcd.println(modem.localIP());

  Preferences prefs;

  prefs.begin("location", true);
  lat = prefs.getFloat("lat", 34.704);
  lon = prefs.getFloat("lon", 137.734);
  prefs.end();
  
   slack.postSlack(lat,lon);

  lcd.clear();

  //start tasks
  xTaskCreatePinnedToCore(buttonTask, "button", 8192, NULL, 1, NULL, 1);
  //xTaskCreatePinnedToCore(downloadTask,"download",8192,1,NULL,0);

 
}


void loop() {
  //loop function download tile & draw map
  //WIFI location
  http.getLocation(lat, lon);

  if (!lat) {
    Serial.println("modem gps");
    modem.enableGPS();
    modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
    zoom = 10;
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

  lcd.clear();
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      if (0 < (main.offsetX() + 256 * (x + 1)) or (main.offsetX() + 256 * x) < 320) {
        if (0 < (main.offsetY() + 256 * (y + 1)) or (main.offsetY() + 256 * y) < 240) {
          http.getMap(main.path(x, y), main.filename(x, y));
          sprite.createSprite(256, 256);
          sprite.drawPngFile(SD, main.filename(x, y).c_str(), 0, 0);
          sprite.pushSprite(main.offsetX(x), main.offsetY(y));
        }
      }
    }
  }


  Serial.println(main.filename());

  lcd.drawCircle(160, 120, 10, TFT_RED);
  lcd.drawCircle(160, 120, 9, TFT_RED);
  lcd.drawCircle(160, 120, 8, TFT_RED);
  lcd.setTextSize(2);
  lcd.setCursor(50,220);
  lcd.print("exp");
  lcd.setCursor(225,220);
  lcd.print("detail");
  Serial.println("done");
  int loadx = -2;
  int loady = -2;
  int lastZoom = zoom;
  while (1) {
    int breakFlg = 0;
    int interval = 30000;
    //download section
    main.setPlot(lat, lon);
    lcd.drawCircle(main.plotX(), main.plotY(), 7, TFT_RED);
    lastZoom = zoom;
    if (main.plotX()<40 or main.plotX()>280 or main.plotY()<40 or main.plotY()>200) {
      break;
    }
    if (loady <= 2) {
      interval = 0;
      if (http.getMap(main.path(loadx, loady), main.filename(loadx, loady))) {
        Serial.printf("load tile %d,%d\n", loadx, loady);
      }

      loadx++;
      if (loadx > 2) {
        loady++;
        loadx = -2;
      }
    } else {
      Serial.println("surround map ended");
      http.getLocation(lat, lon);
    }

    for (int i = 0; i < interval; i++) {
      if (lastZoom != zoom) {
        breakFlg=1;
        break;
      }
      vTaskDelay(1);
    }
    if(breakFlg){
      break;
    }
  }
}



void buttonTask(void* arg) {
  //Button check Task 0.01 sec interval
  while (1) {
    M5.update();
    //download sarani surround tile
    if (M5.BtnA.wasPressed()) {
      //Serial.println("zoom -");
      zoom--;
    }
    if (M5.BtnC.wasPressed()) {
      //Serial.println("zoom +");
      zoom++;
    }
    if(M5.BtnB.wasPressed()){
      //mode set
    }

    vTaskDelay(10);
  }
}

void downloadTask(void* arg) {
  while (1) {
    vTaskDelay(1);
  }
}

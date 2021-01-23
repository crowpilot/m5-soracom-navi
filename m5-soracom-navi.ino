#define LGFX_AUTODETECT
#include <M5Stack.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<string.h>

#include<LovyanGFX.hpp>

#include <math.h>

TinyGsm modem(Serial2); /* 3G board modem */
TinyGsmClientSecure ctx(modem);

static LGFX lcd;
//static LGFX_Sprite sprite(&lcd);

int zoom = 14;
String path;
String host = "cyberjapandata.gsi.go.jp";
int x, y;
char _buf[100 * 1024] = {0};

float lat, lon, speed, alt, accuracy;
int vsat, usat;
int year, month, day, hour, minute, second;

void setup() {
  M5.begin(false);
  lcd.init();
  //  lcd.setRotation(1);
  lcd.setBrightness(128);

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
  Serial.print("loop");

  //modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second);
  lat=35.0;
  lon=138.0;
  speed=0;
  Serial.printf("%f,%f  speed=%f\n", lat, lon, speed);
  lcd.setCursor(0, 0);
  lcd.setTextSize(2);
  lcd.printf("%3.3f%3.3f\n", lat, lon);

  x = pow(2, 13) * (180 + lon) / 180;
  y = pow(2, 13) * (1 - log(tan(lat * 3.14 / 180) + 1 / cos(lat * 3.14 / 180)) / 3.14);

  path = String("GET /xyz/std/");
  path += String(zoom) + "/";
  path += String(int(x)) + "/";
  path += String(int(y)) + ".png";
  path += " HTTP/1.0";
  Serial.println(path.c_str());

  //M5.Lcd.printf("XYZ=%d,%d/14",int(pow(2,13)*(180+lon)/180),int(pow(2,13)*(1-log(tan(lat*3.14/180)+1/cos(lat*3.14/180))/3.14)));

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
  char* p=_buf;
  Serial.println("body");
  while(ctx.connected()){
    String line=ctx.readStringUntil('\n');
    Serial.println(line);
    strcat(_buf,line.c_str());
  }
  
  Serial.print("size of _buf: ");
  Serial.println(sizeof(_buf));
  
  //ctx.readBytes(_buf, sizeof(_buf)); /* body */
  
  ctx.stop();

  //Serial.print("buf: ");
  //Serial.println(_buf);

  size_t _buf_s = strlen(_buf);
  Serial.print("buf_s:");
  Serial.println(_buf_s);
  
  lcd.clear();
  lcd.drawPng((const unsigned char *)_buf, _buf_s, 0, 0);
   _buf[0] = '\0';
  Serial.println("done");
  while(1){
    delay(1000 * 600);
  }
}

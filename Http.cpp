#include "Http.h"
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>

Http::Http(TinyGsmClientSecure& ctx) {
  init(&ctx);
}

bool Http::init(TinyGsmClientSecure* ctx) {
  this->c = ctx;
  return 1;
}

bool Http::getMap(String path, String filename) {
  if(SD.exists(filename)){
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
  Serial.println("sent");

   while (c->connected()) {
      String line = c->readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        Serial.println("header ok");
        break;
      }
    }
    Serial.println("body");

  f = SD.open(filename, FILE_WRITE);
  while (c->connected()&& c->available()) {
    char ch = c->read();
    Serial.print(ch);
    f.print(ch);
  }
  //f.write((const unsigned char *)_buf,strlen(_buf));
  f.close();
  c->stop();
 Serial.println("f close HTTP stop");
  return 1;
}

bool getHeader() {
  return 1;
}

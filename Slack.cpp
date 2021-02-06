#include "Slack.h"
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<ArduinoJson.h>
#include<WiFi.h>
#include<Preferences.h>

Slack::Slack(TinyGsmClient& ctx) {
  init(&ctx);
}

bool Slack::init(TinyGsmClient* ctx) {
  this->c = ctx;
  return 1;
}

bool Slack::postSlack(float lat,float lon) {

  f = SD.open("/soracom.txt");
  // /WEBHOOK URI + token 
  //check api.slack.com Incoming webhooks
  String soracom;
  while (f.available()) {
    soracom += char(f.read());
  }
  f.close();

  String json="{\"text\":\""+String(lat)+","+String(lon)+"\"}";

  if (!c->connect("beam.soracom.io", 8888)) {
    Serial.println(F("Connect failed."));
    return 0;
  }
  Serial.println("slack connected");
  String path = "POST "+soracom +" HTTP/1.1";
  c->println(path);
  c->print("Host: ");
  c->println("beam.soracom.io");
  c->println("Content-Type: application/json");
  c->print("Content-Length: ");
  c->println(json.length());
  c->println();
  c->println(json.c_str());
  //ctx.print("Connection: close\r\n\r\n");
  Serial.println("Header");

  while (c->connected()) {
    String line = c->readStringUntil('\n');
    Serial.println(line);
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
  c->stop();
  return 1;
}

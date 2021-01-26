#ifndef HTTP_H
  #define HTTP_H
#include<M5Stack.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<string.h>
#include<ArduinoJson.h>
#include<SD.h>

class Http:public TinyGsmClientSecure {
  public:
  Http(){}
  explicit Http(TinyGsmClientSecure& ctx);
  bool init(TinyGsmClientSecure* ctx);
  bool getMap(String path,String filename);
  private:
  bool getHeader();
  TinyGsmClientSecure* c;
  File f;
};

#endif

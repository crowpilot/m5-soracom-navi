//HTTP Map.png Wifi-geolocation
#ifndef HTTP_H
  #define HTTP_H
#include<M5Stack.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<Preferences.h>

class Http:public TinyGsmClientSecure {
  public:
  Http(){}
  explicit Http(TinyGsmClientSecure& ctx);
  bool init(TinyGsmClientSecure* ctx);
  bool getMap(String path,String filename);
  bool getLocation(float &lat,float &lon);
  bool postSlack(float lat,float lon);
  private:
  bool getHeader();
  TinyGsmClientSecure* c;
  File f;
  Preferences _prefs;
  String _wifi[3];
};

#endif

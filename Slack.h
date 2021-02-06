#ifndef SLACK_H
  #define SLACK_H
#include<M5Stack.h>
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
#include<Preferences.h>

class Slack:public TinyGsmClient {
  public:
  Slack(){}
  explicit Slack(TinyGsmClient& ctx);
  bool init(TinyGsmClient* ctx);
  bool postSlack(float lat,float lon);
  
  private:
  bool getHeader();
  TinyGsmClient* c;
  File f;
  Preferences _prefs;
  String _wifi[3];
};

#endif

#ifndef GEO_H
  #define GEO_H
#include<M5Stack.h>
#include<math.h>

class Geo{
  public:
  Geo(float lat,float lon,int zoom=14);
  void setLocation(float lat,float lon,int zoom=14);
  
  String filename();

  String path();

  
  private:
  int _tileX;
  int _tileY;
  float _lat;
  float _lon;
  int _zoom;
  
};

#endif

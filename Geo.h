#ifndef GEO_H
  #define GEO_H
#include<M5Stack.h>
#include<math.h>

class Geo{
  public:
  Geo();
  void setLocation(float lat,float lon,int zoom=14);
  
  String filename();
  String surroundTilename(int x,int y);

  String path();
  String surroundPath();

  int mainTileX;
  int mainTileY;
  private:
  float _lat;
  float _lon;
  int _zoom;
  
};

#endif

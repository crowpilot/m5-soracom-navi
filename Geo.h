#ifndef GEO_H
  #define GEO_H
#include<M5Stack.h>
#include<math.h>

class Geo{
  public:
  Geo(float lat,float lon,int zoom=14);
  void setLocation(float lat,float lon,int zoom=14);
  
  String filename(int x=0,int y=0);

  String path(int x=0,int y=0);

  int offsetX(int x=0);
  int offsetY(int y=0);
  int plotX();
  int plotY();

  void setZoom(int zoom);

  void setPlot(float lat,float lon);
  
  
  private:
  float _tileXraw;
  float _tileYraw;
  int _tileX;
  int _tileY;
  int _offsetX;
  int _offsetY;
  float _lat;
  float _lon;
  int _zoom;
  int _plotX;
  int _plotY;
  
};

#endif

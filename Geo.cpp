#include "Geo.h"



Geo::Geo(float lat,float lon,int zoom){
  _lat=lat;
  _lon=lon;
  _zoom=zoom;
  
  
  _tileXraw=pow(2, zoom-1) * (180 + _lon) / 180;
  _tileYraw=pow(2, zoom-1) * (1 - log(tan(_lat * 3.14 / 180) + 1 / cos(_lat * 3.14 / 180)) / 3.14);
  _tileX = (int)_tileXraw;
  _tileY=(int)_tileYraw;

  _offsetX =(int) 160-256*(_tileXraw-_tileX);
  _offsetY =(int) 120 - 256*(_tileYraw-_tileY);
  Serial.println(_offsetX);
  Serial.println(_offsetY);
}


String Geo::filename(int x,int y){
  return "/"+String(_zoom)+"-"+String(_tileX+x)+"-"+String(_tileY+y)+".png";
}


String Geo::path(int x,int y){
  return "GET /xyz/std/"+String(_zoom)+"/"+String(int(_tileX+x)) + "/"+String(int(_tileY+y)) + ".png"+" HTTP/1.1";
}

int Geo::offsetX(int x){
  return _offsetX+256*x;
}

int Geo::offsetY(int y){
  return _offsetY+256*y;
}

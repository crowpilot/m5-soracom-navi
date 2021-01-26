#include "Geo.h"



Geo::Geo(float lat,float lon,int zoom){
  _lat=lat;
  _lon=lon;
  _zoom=zoom;
  _tileX=pow(2, zoom-1) * (180 + _lon) / 180;
  _tileY=pow(2, zoom-1) * (1 - log(tan(_lat * 3.14 / 180) + 1 / cos(_lat * 3.14 / 180)) / 3.14);
}


String Geo::filename(){
  return "/"+String(_zoom)+"-"+String(_tileX)+"-"+String(_tileY)+".png";
}


String Geo::path(){
  return "GET /xyz/std/"+String(_zoom)+"/"+String(int(_tileX)) + "/"+String(int(_tileY)) + ".png"+" HTTP/1.1";
}

#include "Geo.h"



void Geo::setLocation(float lat,float lon,int zoom){
  _lat=lat;
  _lon=lon;
  _zoom=zoom;
  mainTileX=pow(2, zoom-1) * (180 + _lon) / 180;
  mainTileY=pow(2, zoom-1) * (1 - log(tan(_lat * 3.14 / 180) + 1 / cos(_lat * 3.14 / 180)) / 3.14);
}


String Geo::filename(){
  return "/"+String(_zoom)+"-"+String(mainTileX)+"-"+String(mainTileY)+".png";
}

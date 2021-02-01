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

void Geo::setZoom(int zoom){
  _zoom = zoom;
}

void Geo::setPlot(float lat,float lon){
   float posXraw = pow(2, _zoom-1) * (180 + lon) / 180;
   float posYraw=pow(2, _zoom-1) * (1 - log(tan(lat * 3.14 / 180) + 1 / cos(lat * 3.14 / 180)) / 3.14);

   float diffXraw = posXraw-_tileXraw;
   float diffYraw = posYraw-_tileYraw;

   _plotX = diffXraw*256+160;
   _plotY = diffYraw*256+120;
   Serial.println(_plotX);
   Serial.println(_plotY);
}

int Geo::plotX(){
  return _plotX;
}
int Geo::plotY(){
  return _plotY;
}

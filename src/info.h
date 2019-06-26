#ifndef __INFO_H_
#define __INFO_H_

#include <Arduino.h>

struct Info
{
  int gmtOffset = 0;
  int daylightOffset = 0;
  String city;
  String country;
  String latitude;
  String longitude;
  String timezone;

  String firmwareMD5;
};

extern Info info;

#endif
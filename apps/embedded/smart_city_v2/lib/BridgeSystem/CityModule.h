#ifndef CITY_MODULE_H
#define CITY_MODULE_H

#include <Arduino.h>

class CityModule {
public:
    virtual ~CityModule() {}
    virtual bool begin() = 0;
    virtual void update() = 0;
    virtual const char* getName() = 0;
};

#endif
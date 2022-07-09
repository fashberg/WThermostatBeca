#ifndef W_TEMPERATURE_PROPERTY_H
#define W_TEMPERATURE_PROPERTY_H

#include "WProperty.h"

class WTemperatureProperty: public WProperty {
public:
	WTemperatureProperty(const char* id, const char* title)
	: WProperty(id, title, DOUBLE) {
		this->atType = "TemperatureProperty";
		this->setUnit("celsius");
	}


protected:

private:

};

#endif

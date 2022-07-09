#ifndef W_TARGET_TEMPERATURE_PROPERTY_H
#define W_TARGET_TEMPERATURE_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_TARGETTEMPERATUR PROGMEM = "TargetTemperatureProperty";

class WTargetTemperatureProperty: public WProperty {
public:
	WTargetTemperatureProperty(const char* id, const char* title)
	: WProperty(id, title, DOUBLE) {
		this->atType = ATTYPE_TARGETTEMPERATUR;
		this->setUnit(STR_CELSIUS);
	}


protected:

private:

};

#endif

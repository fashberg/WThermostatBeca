#ifndef W_HEATING_COOLING_PROPERTY_H
#define W_HEATING_COOLING_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_HEATINGCOOLING PROGMEM = "HeatingCoolingProperty";

const char* VALUE_OFF PROGMEM = "off";
const char* VALUE_HEATING PROGMEM = "heating";
const char* VALUE_COOLING PROGMEM = "cooling";

class WHeatingCoolingProperty: public WProperty {
public:
	WHeatingCoolingProperty(const char* id, const char* title)
	: WProperty(id, title, STRING) {
		this->atType = ATTYPE_HEATINGCOOLING;
		this->setReadOnly(true);
		this->addEnumString(VALUE_OFF);
		this->addEnumString(VALUE_HEATING);
		this->addEnumString(VALUE_COOLING);
	}


protected:

private:

};

#endif

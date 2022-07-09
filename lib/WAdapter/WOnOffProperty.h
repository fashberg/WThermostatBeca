#ifndef W_ON_OFF_PROPERTY_H
#define W_ON_OFF_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_ONOFF PROGMEM = "OnOffProperty";

class WOnOffProperty: public WProperty {
public:
	WOnOffProperty(const char* id, const char* title)
	: WProperty(id, title, BOOLEAN) {
		this->atType = "";
	}


protected:

private:

};

#endif

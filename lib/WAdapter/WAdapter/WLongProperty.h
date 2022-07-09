#ifndef W_LONG_PROPERTY_H
#define W_LONG_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_LONG PROGMEM = "LongProperty";

class WLongProperty: public WProperty {
public:
	WLongProperty(const char* id)
	: WProperty(id, id, LONG) {
		this->atType = ATTYPE_LONG;
	}

protected:

private:

};

#endif

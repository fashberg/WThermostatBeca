#ifndef W_INTEGER_PROPERTY_H
#define W_INTEGER_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_INTEGER PROGMEM = "IntegerProperty";

class WIntegerProperty: public WProperty {
public:
	WIntegerProperty(const char* id, const char* title)
	: WProperty(id, title, INTEGER) {
		this->atType = ATTYPE_INTEGER;
	}

protected:

private:

};

#endif

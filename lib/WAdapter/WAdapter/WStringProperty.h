#ifndef W_STRING_PROPERTY_H
#define W_STRING_PROPERTY_H

#include "WProperty.h"

const char* ATTYPE_STRING PROGMEM = "StringProperty";
class WStringProperty: public WProperty {
public:
	WStringProperty(const char* id, const char* title, byte length)
	: WProperty(id, title, STRING, length) {
		this->atType = ATTYPE_STRING;
	}


protected:

private:

};

#endif

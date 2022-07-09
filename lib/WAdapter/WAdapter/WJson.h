#ifndef _WJSON_H__
#define _WJSON_H__

#include "Arduino.h"

const static char BBEGIN = '[';
const static char BEND = ']';
const static char COMMA = ',';
const static char DPOINT = ':';
const static char SBEGIN = '{';
const static char SEND = '}';
const static char QUOTE = '\"';

const char* JSON_TRUE PROGMEM = "true";
const char* JSON_FALSE PROGMEM = "false";
const char* JSON_NULL PROGMEM = "null";

class WJson {
public:
	WJson(Print* stream) {
		this->stream = stream;
	}

	WJson& beginObject() {
		return beginObject("");
	}

	WJson& beginObject(const char* name) {
		if (!separatorAlreadyCalled) {
			ifSeparator();
			separatorAlreadyCalled = true;
		}
		if (name && strlen(name)) {
			memberName(name);
		}
		stream->print(SBEGIN);
		firstElement = true;
		return *this;
	}

	WJson& memberName(const char *name) {
		if (name != nullptr) {
			string(name);
			stream->print(DPOINT);
		}
		return *this;
	}

	WJson& separator() {
		stream->print(COMMA);
		return *this;

	}

	WJson& endObject() {
		stream->print(SEND);
		if (firstElement){
			// object with no properties
			firstElement=false;
			separatorAlreadyCalled=false;
		}
		return *this;
	}

	WJson& beginArray() {
		if (!separatorAlreadyCalled) {
			ifSeparator();
		}
		firstElement = true;
		stream->print(BBEGIN);
		return *this;
	}

	WJson& beginArray(const char* name) {

		if (!separatorAlreadyCalled) {
			ifSeparator();
			separatorAlreadyCalled = true;
		}
		firstElement = true;
		memberName(name);
		separatorAlreadyCalled = false;
		stream->print(BBEGIN);
		return *this;
	}

	WJson& endArray() {
		stream->print(BEND);
		return *this;
	}

	WJson& propertyString(const char* name, const char *value) {
		return propertyString(name, value, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2) {
		return propertyString(name, value1, value2, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3) {
		return propertyString(name, value1, value2, value3, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4) {
		return propertyString(name, value1, value2, value3, value4, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5) {
		return propertyString(name, value1, value2, value3, value4, value5, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5, const char *value6) {
		return propertyString(name, value1, value2, value3, value4, value5, value6, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5, const char *value6, const char *value7) {
		return propertyString(name, value1, value2, value3, value4, value5, value6, value7, nullptr, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5, const char *value6, const char *value7, const char *value8) {
		return propertyString(name, value1, value2, value3, value4, value5, value6, value7, value8, nullptr, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5, const char *value6, const char *value7, const char *value8, const char *value9) {
		return propertyString(name, value1, value2, value3, value4, value5, value6, value7, value8, value9, nullptr);
	}

	WJson& propertyString(const char* name, const char *value1, const char *value2, const char *value3, const char *value4, const char *value5, const char *value6, const char *value7, const char *value8, const char *value9, const char *value10) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		string(value1, value2, value3, value4, value5, value6, value7, value8, value9, value10);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyInteger(const char* name, int value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		numberInteger(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyLong(const char* name, long value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		numberLong(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyUnsignedLong(const char* name, unsigned long value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		numberUnsignedLong(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyByte(const char* name, byte value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		numberByte(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyDouble(const char* name, double value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		numberDouble(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& propertyBoolean(const char* name, bool value) {
		ifSeparator();
		separatorAlreadyCalled = true;
		memberName(name);
		boolean(value);
		separatorAlreadyCalled = false;
		return *this;
	}

	WJson& string(const char *text) {
		return string(text, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2) {
		return string(text1, text2, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3) {
		return string(text1, text2, text3, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4) {
		return string(text1, text2, text3, text4, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5) {
		return string(text1, text2, text3, text4, text5, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, const char *text6) {
		return string(text1, text2, text3, text4, text5, text6, nullptr, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, const char *text6, const char *text7) {
		return string(text1, text2, text3, text4, text5, text6, text7, nullptr, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, const char *text6, const char *text7, const char *text8) {
		return string(text1, text2, text3, text4, text5, text6, text7, text8, nullptr, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, const char *text6, const char *text7, const char *text8, const char *text9) {
		return string(text1, text2, text3, text4, text5, text6, text7, text8, text9, nullptr);
	}

	WJson& string(const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, const char *text6, const char *text7, const char *text8, const char *text9, const char *text10) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(QUOTE);
		if (text1 != nullptr) stream->print(text1);
		if (text2 != nullptr) stream->print(text2);
		if (text3 != nullptr) stream->print(text3);
		if (text4 != nullptr) stream->print(text4);
		if (text5 != nullptr) stream->print(text5);
		if (text6 != nullptr) stream->print(text6);
		if (text7 != nullptr) stream->print(text7);
		if (text8 != nullptr) stream->print(text8);
		if (text9 != nullptr) stream->print(text9);
		if (text10 != nullptr) stream->print(text10);
		stream->print(QUOTE);
		return *this;
	}

	WJson& numberInteger(int number) {
		if(!separatorAlreadyCalled)
		ifSeparator();
		stream->print(number, DEC);
		return *this;
	}

	WJson& numberLong(unsigned long number) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(number, DEC);
		return *this;
	}
	
	WJson& numberUnsignedLong(unsigned long number) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(number, DEC);
		return *this;
	}

	WJson& numberByte(byte number) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(number, DEC);
		return *this;
	}

	WJson& numberDouble(double number) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(number);
		return *this;
	}

	WJson& null() {
		ifSeparator();
		stream->print(JSON_NULL);
		return *this;
	}

	WJson& boolean(bool value) {
		if (!separatorAlreadyCalled)
			ifSeparator();
		stream->print(value ? JSON_TRUE : JSON_FALSE);
		return *this;
	}

	int operator[](char*);

private:
	Print* stream;
	bool firstElement = true;
	bool separatorAlreadyCalled = false;

	void ifSeparator() {
		if (firstElement) {
			firstElement = false;
		} else {
			separator();
		}
	}

};

#endif

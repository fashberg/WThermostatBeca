#ifndef W_PROPERTY_H
#define W_PROPERTY_H

#include <Arduino.h>
#include "WJson.h"


const char* STR_MINIMUM PROGMEM = "minimum";
const char* STR_MAXIMUM PROGMEM = "maximum";
const char* STR_CELSIUS PROGMEM = "celsius";
const char* STRPROP_TRUE PROGMEM = "true";
const char* STRPROP_FALSE PROGMEM = "false";
const char* STRPROP_TITLE PROGMEM = "title";
const char* STRPROP_TYPE PROGMEM = "type";
const char* STRPROP_ATTYPE PROGMEM = "@type";
const char* STRPROP_BOOLEAN PROGMEM = "boolean";
const char* STRPROP_NUMBER PROGMEM = "number";
const char* STRPROP_STRING PROGMEM = "string";
const char* STRPROP_READONLY PROGMEM = "readOnly";
const char* STRPROP_UNIT PROGMEM = "unit";
const char* STRPROP_MULTIPLEOF PROGMEM = "multipleOf";
const char* STRPROP_ENUM PROGMEM = "enum";
const char* STRPROP_LINKS PROGMEM = "links";
const char* STRPROP_HREF PROGMEM = "href";
const char* STRPROP_PROPERTIES PROGMEM = "properties";
const char* STRPROP_URI_SEP PROGMEM = "/";

enum WPropertyType {
	BOOLEAN, DOUBLE, INTEGER, LONG, UNSIGNED_LONG, BYTE, STRING
};

enum WPropertyVisibility {
	ALL, NONE, MQTT, WEBTHING
};

union WPropertyValue {
	bool asBoolean;
	double asDouble;
	int asInteger;
	long asLong;
	unsigned long asUnsignedLong;
	byte asByte;
	char* string;
};

class WConstStringProperty {
public:
	WConstStringProperty(const char * str) {
		value = str;
		next = nullptr;
	}
	~WConstStringProperty() {
	}
	const char* c_str() {
		return value;
	}

	WConstStringProperty* next;
	private:
	const char * value;
};

class WProperty {
public:
	typedef std::function<void(WProperty* property)> TOnPropertyChange;

	WProperty(const char* id, const char* title, WPropertyType type) {
		initialize(id, title, type, (type == STRING ? 32 : 0));
	}

	WProperty(const char* id, const char* title, WPropertyType type, byte length) {
		initialize(id, title, type, length);
	}

	virtual ~WProperty() {
		delete this->id;
		delete this->title;
		if (this->unit) {
			delete this->unit;
		}
		if (this->atType) {
			delete this->atType;
		}
		if(this->value.string) {
		    delete[] this->value.string;
		}
	}

	void setOnValueRequest(TOnPropertyChange onValueRequest) {
		this->onValueRequest = onValueRequest;
	}

	void setOnChange(TOnPropertyChange onChange) {
		this->onChange = onChange;
	}

	void setDeviceNotification(TOnPropertyChange deviceNotification) {
		this->deviceNotification = deviceNotification;
	}

	void setSettingsNotification(TOnPropertyChange settingsNotification) {
		this->settingsNotification = settingsNotification;
	}

	const char* getId() {
		return id;
	}

	const char* getTitle() {
		if (title==nullptr) return id;
		return title;
	}

	WPropertyType getType() {
		return type;
	}

	byte getLength() {
		return length;
	}

	void setType(WPropertyType type) {
		this->type = type;
	}

	const char* getAtType() {
		return atType;
	}

	/*void setAtType(String atType) {
		this->atType = atType;
	}*/

	bool isNull() {
		return (this->valueNull);
	}

	bool isRequested() {
		return (this->requested);
	}

	void setRequested(bool requested) {
		if ((requested) && (!isNull())) {
			this->requested = true;
		} else {
			this->requested = false;
		}
	}
	void setNull() {
		this->valueNull = true;
	}

	bool isChanged() {
		return (this->changed);
	}

	void setUnChanged() {
		this->changed = false;
	}

	void setChanged() {
		this->changed = true;
	}

	virtual bool parse(String value) {
		if ((!isReadOnly()) && (value != nullptr)) {
			switch (getType()) {
				case BOOLEAN: {
					setBoolean(value.equalsIgnoreCase(STRPROP_TRUE));
					return true;
				}
				case DOUBLE: {
					setDouble(value.toDouble());
					return true;
				}
				case INTEGER: {
					setInteger(value.toInt());
					return true;
				}
				case LONG: {
					setLong(value.toInt());
					return true;
				}
				case UNSIGNED_LONG: {
					setUnsignedLong(value.toInt());
					return true;
				}
				case BYTE: {
					setByte(value.toInt());
					return true;
				}
				case STRING: {
					setString(value.c_str());
					return true;
				}
			}
	
		}
		return false;
	}

	bool getBoolean() {
		requestValue();
		return (!this->valueNull ? this->value.asBoolean : false);
	}

	void setBoolean(bool newValue) {
		if (type != BOOLEAN) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asBoolean != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asBoolean = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	void toggleBoolean() {
		if (type != BOOLEAN) {
			return;
		}
		setBoolean(!getBoolean());
	}

	double getDouble() {
		requestValue();
		return (!this->valueNull ? this->value.asDouble : 0.0);
	}

	static bool isEqual(double a, double b, double precision) {
        double diff = a - b;
        return ((diff < precision) && (-diff < precision));
    }

	void setDouble(double newValue) {
		if (type != DOUBLE) {
			return;
		}
		bool changed = ((this->valueNull) || (!isEqual(this->value.asDouble, newValue, 0.01)));
		if (changed) {
			WPropertyValue valueB;
			valueB.asDouble = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	bool equalsDouble(double number) {
		return ((!this->valueNull) && (isEqual(this->value.asDouble, number, 0.01)));
	}

	int getInteger() {
		requestValue();
		return (!this->valueNull ? this->value.asInteger : 0);
	}

	void setInteger(int newValue) {
		if (type != INTEGER) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asInteger != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asInteger = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	long getLong() {
		requestValue();
		return (!this->valueNull ? this->value.asLong : 0);
	}

	void setLong(long newValue) {
		if (type != LONG) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asLong != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asLong = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	unsigned long getUnsignedLong() {
		requestValue();
		return (!this->valueNull ? this->value.asUnsignedLong : 0);
	}

	void setUnsignedLong(unsigned long newValue) {
		if (type != UNSIGNED_LONG) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asUnsignedLong != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asUnsignedLong = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	bool equalsInteger(int number) {
		return ((!this->valueNull) && (this->value.asInteger == number));
	}

	bool equalsLong(long number) {
		return ((!this->valueNull) && (this->value.asLong == number));
	}

	bool equalsString(const char* toCompare) {
		return ((!this->valueNull) && (strcmp(this->value.string, toCompare) == 0));
	}

	bool equalsUnsignedLong(unsigned long number) {
		return ((!this->valueNull) && (this->value.asUnsignedLong == number));
	}

	byte getByte() {
		requestValue();
		return (!this->valueNull ? this->value.asByte : 0x00);
	}

	void setByte(byte newValue) {
		if (type != BYTE) {
			return;
		}
		bool changed = ((this->valueNull) || (this->value.asByte != newValue));
		if (changed) {
			WPropertyValue valueB;
			valueB.asByte = newValue;
			this->setValue(valueB);
		}
		afterSet();
	}

	bool equalsByte(byte number) {
		return ((!this->valueNull) && (this->value.asByte == number));
	}

	const char* c_str() {
		requestValue();
		return value.string;
	}

	WPropertyValue getValue() {
	    return this->value;
	}

	void setString(const char* newValue) {
		if (type != STRING) {
			return;
		}
		bool changed = ((this->valueNull) || (strcmp(value.string, newValue) != 0));
		if (changed) {
			if (newValue != nullptr) {
				int l = strlen(newValue);
				if (l > length) {
					l = length;
				}
				strncpy(value.string, newValue, l);
				value.string[l] = '\0';
				this->valueNull = false;
			} else {
				value.string[0] = '\0';
				this->valueNull = true;
			}
			this->changed = true;
			valueChanged();
			notify();
		}
		afterSet();
	}

	bool isReadOnly() {
		return readOnly;
	}

	void setReadOnly(bool readOnly) {
		this->readOnly = readOnly;
	}

	const char* getUnit() {
		return unit;
	}

	void setUnit(const char* unit) {
		this->unit = unit;
	}

	double getMultipleOf() {
		return multipleOf;
	}

	void setMultipleOf(double multipleOf) {
		this->multipleOf = multipleOf;
	}

	virtual String toString() {
		switch (getType()) {
		case BOOLEAN:
			return String( (getBoolean() ? STRPROP_TRUE : STRPROP_FALSE));
			break;
		case DOUBLE:
			return String(getDouble());
			break;
		case INTEGER:
			return String(getInteger());
			break;
		case LONG:
			return String(getLong());
			break;
		case UNSIGNED_LONG:
			return String(getUnsignedLong());
			break;
		case BYTE:
			return String(getByte());
			break;
		case STRING:
			return String(c_str());
			break;
		}
		return String("");

	}

	virtual void toJsonValue(WJson* json, bool onlyValue=false) {
		requestValue();
		const char* memberName = (onlyValue ? nullptr : getId());
		switch (getType()) {
		case BOOLEAN:
			json->propertyBoolean(memberName, getBoolean());
			break;
		case DOUBLE:
			json->propertyDouble(memberName, getDouble());
			break;
		case INTEGER:
			json->propertyInteger(memberName, getInteger());
			break;
		case LONG:
			json->propertyLong(memberName, getLong());
			break;
		case UNSIGNED_LONG:
			json->propertyUnsignedLong(memberName, getUnsignedLong());
			break;
		case BYTE:
			json->propertyByte(memberName, getByte());
			break;
		case STRING:
			json->propertyString(memberName, c_str());
			break;
		}
	}

	virtual void toJsonStructure(WJson* json, const char* memberName, const char* deviceHRef) {
		json->beginObject(memberName);
		//title
		if (this->getTitle() && strlen(this->getTitle())) {
			json->propertyString(STRPROP_TITLE, getTitle());
		}
		//type
		switch (this->getType()) {
		case BOOLEAN:
			json->propertyString(STRPROP_TYPE, STRPROP_BOOLEAN);
			break;
		case DOUBLE:
		case INTEGER:
		case LONG:
		case UNSIGNED_LONG:
		case BYTE:
			json->propertyString(STRPROP_TYPE, STRPROP_NUMBER);
			break;
		default:
			json->propertyString(STRPROP_TYPE, STRPROP_STRING);
			break;
		}
		//readOnly
		if (this->isReadOnly()) {
			json->propertyBoolean(STRPROP_READONLY, true);
		}
		//unit
		if (this->getUnit() && strlen(this->getUnit())) {
			json->propertyString(STRPROP_UNIT, (this->getUnit() ? this->getUnit() : ""));
		}
		//multipleOf
		if (this->getMultipleOf() > 0.0) {
			json->propertyDouble(STRPROP_MULTIPLEOF, this->getMultipleOf());
		}
		//enum
		if (hasEnum()) {
			json->beginArray(STRPROP_ENUM);
			WConstStringProperty* propE = this->firstEnum;
			while (propE != nullptr) {
				switch (this->getType()) {
				case STRING:
					json->string(propE->c_str());
					break;
				default:
					break;
				}
				propE = propE->next;
			}
			json->endArray();
		}
		//aType
		if (this->getAtType() && strlen(this->getAtType())) {
			json->propertyString(STRPROP_ATTYPE, (this->getAtType() ?  this->getAtType() : ""));
		}
		toJsonStructureAdditionalParameters(json);
		json->beginArray(STRPROP_LINKS);
		json->beginObject();
		json->propertyString(STRPROP_HREF, deviceHRef, STRPROP_URI_SEP, STRPROP_PROPERTIES, STRPROP_URI_SEP, this->getId());
		json->endObject();
		json->endArray();

		json->endObject();
	}

	WProperty* next;

	void addEnumString(const char* enumString) {
		if (type != STRING) {
			return;
		}
		WConstStringProperty* valueE = new WConstStringProperty(enumString);
		this->addEnum(valueE);
	}

	void addEnum(WConstStringProperty* propEnum) {
		WConstStringProperty* lastEnum = firstEnum;
		while ((lastEnum != nullptr) && (lastEnum->next != nullptr)) {
			lastEnum = lastEnum->next;
		}
		if (lastEnum != nullptr) {
			lastEnum->next = propEnum;
		} else {
			firstEnum = propEnum;
		}
	}

	bool hasEnum() {
		return (firstEnum != nullptr);
	}

	WPropertyVisibility getVisibility() {
		return visibility;
	}

	void setVisibility(WPropertyVisibility visibility) {
		this->visibility = visibility;
	}

	bool isVisible(WPropertyVisibility visibility) {
		return ((this->visibility == ALL) || (this->visibility == visibility));
	}

	void setAtType(const char* atType) {
		this->atType = atType;
	}

	void setMqttSendChangedValues(bool val) {
		this->mqttSendChangedValues = val;
	}
	bool isMqttSendChangedValues() {
		return this->mqttSendChangedValues;
	}

	void setSuppressOnChange(bool val=true) {
		this->suppressOnChange=val;
	}

protected:
	const char* atType;

	void initialize(const char* id, const char* title, WPropertyType type, byte length) {
		this->id = id;
		this->title = title;
		this->type = type;
		this->visibility = ALL;
		this->supportingWebthing = true;
		this->supportingMqtt = true;
		this->mqttSendChangedValues = false;
		this->valueNull = true;
		this->changed = true;
		this->requested = false;
		this->valueRequesting = false;
		this->suppressOnChange = false;
		this->notifying = false;
		this->readOnly = false;
		this->atType = nullptr;
		this->unit = nullptr;
		this->multipleOf = 0.0;
		this->onChange = nullptr;
		this->deviceNotification = nullptr;
		this->settingsNotification = nullptr;
		this->next = nullptr;
		switch (type) {
		case STRING:
			this->length = length;
			value.string = new char[length + 1];
			value.string[0] = '\0';
			break;
		case DOUBLE:
			this->length = sizeof(double);
			break;
		case INTEGER:
			this->length = 2;
			break;
		case LONG:
			case UNSIGNED_LONG:
			this->length = 4;
			break;
		case BYTE:
		case BOOLEAN:
			this->length = 1;
			break;
		}
	}

	void setValue(WPropertyValue newValue) {
		this->value = newValue;
		this->valueNull = false;
		this->changed = true;
		valueChanged();
		notify();
	}

	virtual void valueChanged() {
	}

	virtual void toJsonStructureAdditionalParameters(WJson* json) {

	}

private:
	const char* id;
	const char* title;
	WPropertyType type;
	WPropertyVisibility visibility;
	bool supportingMqtt;
	bool mqttSendChangedValues;
	bool supportingWebthing;
	byte length;
	bool readOnly;
	const char* unit;
	double multipleOf;
	TOnPropertyChange onChange;
	TOnPropertyChange onValueRequest;
	TOnPropertyChange deviceNotification;
	TOnPropertyChange settingsNotification;
	WPropertyValue value = {false};
	bool valueNull;
	bool changed;
	bool requested;
	bool valueRequesting;
	bool suppressOnChange;
	bool notifying;

	WConstStringProperty* firstEnum = nullptr;

	void notify() {
		if (!valueRequesting) {
			notifying = true;
			if (onChange  && !suppressOnChange) {
				onChange(this);
			}
			if (deviceNotification) {
				deviceNotification(this);
			}
			if (settingsNotification) {
				settingsNotification(this);
			}
			notifying = false;
		}
		if (suppressOnChange) suppressOnChange=false;
	}


	void afterSet() {
		if (suppressOnChange) suppressOnChange=false;
	}


	void requestValue() {
		if ((!notifying) && (onValueRequest)) {
			valueRequesting = true;
			onValueRequest(this);
			valueRequesting = false;
		}
	}

};

#endif
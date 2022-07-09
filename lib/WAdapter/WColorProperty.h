#ifndef W_COLOR_PROPERTY_H
#define W_COLOR_PROPERTY_H

#include "WProperty.h"


const char* ATTYPE_COLOR PROGMEM = "ColorProperty";

class WColorProperty: public WProperty {
public:
	WColorProperty(const char* id, const char* title, byte red, byte green, byte blue)
	: WProperty(id, title, STRING, 7) {
		this->atType = ATTYPE_COLOR;
		this->setRGB(red, green, blue);
		this->changeValue = false;
	}

	byte getRed() {
		return this->red;
	}

	byte getGreen() {
		return this->green;
	}

	byte getBlue() {
		return this->blue;
	}

	void setRGB(byte red, byte green, byte blue) {
		if ((this->red != red) || (this->green != green) || (this->blue != blue)) {
			this->red = red;
			this->green = green;
			this->blue = blue;
			const char* rgbString = createRGBString();
			this->changeValue = true;
			setString(rgbString);
			this->changeValue = false;
			delete rgbString;
		}
	}

	const char* createRGBString() {
		WStringStream result(7);
		result.print("#");
		char buffer[2];
		itoa(red, buffer, 16);
		if (red < 0x10) result.print("0");
		result.print(buffer);
		itoa(green, buffer, 16);
		if (green < 0x10) result.print("0");
		result.print(buffer);
		itoa(blue, buffer, 16);
		if (blue < 0x10) result.print("0");
		result.print(buffer);
		return result.c_str();
	}

	void parseRGBString() {
		char buffer[3];
		buffer[2] = '\0';
		buffer[0] = c_str()[1];
		buffer[1] = c_str()[2];
		this->red = strtol(buffer, NULL, 16);
		buffer[0] = c_str()[3];
		buffer[1] = c_str()[4];
		this->green = strtol(buffer, NULL, 16);
		buffer[0] = c_str()[5];
		buffer[1] = c_str()[6];
		this->blue = strtol(buffer, NULL, 16);
	}

	bool parse(String value) {
		if ((!isReadOnly()) && (value != nullptr)) {
			if ((value.startsWith("#")) && (value.length() == 7)) {
				setString(value.c_str());
				return true;
			} else if ((value.startsWith("rgb(")) && (value.endsWith(")"))) {
				value = value.substring(4, value.length() - 1);
				int theComma;
				//red
				byte red = 0;
				if ((theComma = value.indexOf(",")) > -1) {
					red = value.substring(0, theComma).toInt();
					value = value.substring(theComma + 1);
				}
				//green
				byte green = 0;
				if ((theComma = value.indexOf(",")) > -1) {
					green = value.substring(0, theComma).toInt();
					value = value.substring(theComma + 1);
				}
				//blue
				byte blue = value.toInt();
				setRGB(red, green, blue);
			}
		}
		return false;
	}


protected:
	virtual void valueChanged() {
		if (!changeValue) {
			parseRGBString();
		}
	}
private:
	bool changeValue;
	byte red, green, blue;
};

#endif

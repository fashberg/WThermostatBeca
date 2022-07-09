#ifndef W_PIN_H
#define W_PIN_H

#include "WProperty.h"

class WPin {
public:
	WPin(int pin, int mode) {
		this->pin = pin;
		this->property = nullptr;
		if (this->isInitialized() && ((mode == INPUT) || (mode == OUTPUT) || (mode == INPUT_PULLUP))) {
			pinMode(this->pin, mode);
		}
	}

	WProperty* getProperty() {
		return property;
	}

	void setProperty(WProperty* property) {
		if (this->property != property) {
			this->property = property;
			this->loop(millis());
		}
	}

	virtual void loop(unsigned long now) {
	}

	WPin* next = nullptr;

protected:

	virtual bool isInitialized() {
		return (pin > -1);
	}

	int getPin() {
		return pin;
	}

private:
	int pin;
	WProperty* property;
};

#endif

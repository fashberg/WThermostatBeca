#ifndef W_RELAY_H
#define W_RELAY_H

#include "WPin.h"

class WRelay: public WPin {
public:
	WRelay(int relayPin, bool highIsOn)
	: WPin(relayPin, OUTPUT) {
		this->highIsOn = highIsOn;
		if (this->isInitialized()) {
			digitalWrite(this->getPin(), (highIsOn ? LOW : HIGH));
		}
	}

	bool isOn() {
		return (digitalRead(this->relayPin) == (highIsOn ? HIGH : LOW));
	}

	void loop(unsigned long now) {
		if ((this->isInitialized()) && (getProperty() != nullptr)) {
			digitalWrite(this->getPin(), getProperty()->getBoolean() ? (highIsOn ? HIGH : LOW) : (highIsOn ? LOW : HIGH));
		}
	}

protected:

private:
	int relayPin;
	bool highIsOn;
};

#endif

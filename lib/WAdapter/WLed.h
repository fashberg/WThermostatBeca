#ifndef W_LED_H
#define W_LED_H

#include "WPin.h"

class WLed: public WPin {
public:
	WLed(int ledPin)
		: WPin(ledPin, OUTPUT) {
		this->blinkMillis = 0;
		this->ledOn = false;
		if (this->isInitialized()) {
			digitalWrite(this->getPin(), HIGH);
		}
	}

	bool isOn() {
		return (this->getProperty() != nullptr ? this->getProperty()->getBoolean() : this->ledOn);
	}

	void setOn(bool ledOn) {
		if (this->getProperty() != nullptr) {
			this->getProperty()->setBoolean(ledOn);
		} else if (ledOn != isOn()) {
			this->ledOn = ledOn;
		}
		blinkOn = false;
		lastBlinkOn = 0;
		/*if (ledOn != isOn()) {
			this->ledOn = ledOn;
			digitalWrite(this->getPin(), ledOn ? LOW : HIGH);
			if (this->ledOn) {
				blinkOn = true;
				lastBlinkOn = millis();
			}
			log("Switch LED " + String(ledOn ? "on" : "off") + ". pin:" + String(this->getPin()));
			notify(true);
		}*/
	}

	void setOn(bool ledOn, unsigned long blinkMillis) {
		if ((this->isOn()) && (this->blinkMillis != blinkMillis)) {
			this->setOn(false);
		}
		this->blinkMillis = blinkMillis;
		this->setOn(ledOn);
	}

	void on() {
		setOn(true);
	}

	void off() {
		setOn(false);
	}

	void toggle() {
		setOn(isOn() ? false : true);
	}

	bool isBlinking() {
		return (this->blinkMillis > 0);
	}

	void loop(unsigned long now) {
		if (isOn()) {
			if (isBlinking()) {
				if ((lastBlinkOn == 0) || (now - lastBlinkOn > this->blinkMillis)) {
					blinkOn = !blinkOn;
					lastBlinkOn = now;
					digitalWrite(this->getPin(), blinkOn ? LOW : HIGH);
				}
			} else {
				digitalWrite(this->getPin(), LOW);
			}
		} else {
			//switchoff
			digitalWrite(this->getPin(), HIGH);
		}
		/*if ((isOn()) && (isBlinking()) && (now - lastBlinkOn > this->blinkMillis)) {
			blinkOn = !blinkOn;
			lastBlinkOn = now;
			digitalWrite(this->getPin(), blinkOn ? LOW : HIGH);
		}*/
	}

protected:
private:
	bool ledOn, blinkOn;
	unsigned long blinkMillis, lastBlinkOn;
};

#endif

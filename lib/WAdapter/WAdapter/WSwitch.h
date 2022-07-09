#ifndef W_SWITCH_H_
#define W_SWITCH_H_

#include "WPin.h"

#define MODE_BUTTON 0
#define MODE_BUTTON_LONG_PRESS 1
#define MODE_SWITCH 2

const long SWITCH_SENSITIVENESS = 10;

class WSwitch: public WPin {
public:
	WSwitch(int switchPin, byte mode)
	: WPin(switchPin, INPUT) {
		startTime = 0;
		_pressed = false;
		_pressedLong = false;
		this->mode = mode;
		longPressDuration = 5000;
		switchChangeDuration = 1000;
		if (this->isInitialized()) {
			state = digitalRead(this->getPin());
			if (state == LOW) {
				_pressed = true;
			}
		}

	}
	void loop(unsigned long now) {
		if (this->isInitialized()) {
			bool currentState = digitalRead(this->getPin());
			if (currentState == LOW) { // buttons has been pressed
				// starting timer. used for switch sensitiveness
				if (startTime == 0) {
					startTime = now;
				}
				if (now - startTime >= SWITCH_SENSITIVENESS) {
					// switch pressed, sensitiveness taken into account
					if (!_pressed) {
						// This is set only once when switch is pressed
						state = !state;
						_pressed = true;
						if ((this->mode == MODE_BUTTON) || (this->mode == MODE_SWITCH)) {
							//log("Switch pressed short. pin:" + String(this->getPin()));
							if (getProperty() != nullptr) {
								getProperty()->setBoolean(!getProperty()->getBoolean());
							}
							//notify(false);
						}
					}
					if (this->mode == MODE_BUTTON_LONG_PRESS) {
						if (now - startTime >= longPressDuration && !_pressedLong) {
							_pressedLong = true;
							//log("Switch pressed long. pin:" + String(this->getPin()));
							//notify(true);
						}
					}
				}
			} else if (currentState == HIGH && startTime > 0) {
				if ((_pressed) && (!_pressedLong) &&
					((this->mode == MODE_BUTTON_LONG_PRESS) || ((this->mode == MODE_SWITCH) && (now - startTime >= switchChangeDuration)))) {
					//log("Switch pressed short. pin:" + String(this->getPin()));
					if (getProperty() != nullptr) {
						getProperty()->setBoolean(!getProperty()->getBoolean());
					}
					//notify(false);
				}
				startTime = 0;
				_pressedLong = false;
				_pressed = false;
			}
		}
	}
private:
	byte mode;
	int longPressDuration, switchChangeDuration;
	bool state;
	unsigned long startTime;
	bool _pressed;
	bool _pressedLong;
};

#endif

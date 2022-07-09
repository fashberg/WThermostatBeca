#ifndef _WJSON_PARSER_H__
#define _WJSON_PARSER_H__

#include "Arduino.h"
#include "WJson.h"

#define STATE_START_DOCUMENT     0
#define STATE_DONE               -1
#define STATE_IN_ARRAY           1
#define STATE_IN_OBJECT          2
#define STATE_END_KEY            3
#define STATE_AFTER_KEY          4
#define STATE_IN_STRING          5
#define STATE_START_ESCAPE       6
#define STATE_UNICODE            7
#define STATE_IN_NUMBER          8
#define STATE_IN_TRUE            9
#define STATE_IN_FALSE           10
#define STATE_IN_NULL            11
#define STATE_AFTER_VALUE        12
#define STATE_UNICODE_SURROGATE  13

#define STACK_OBJECT             0
#define STACK_ARRAY              1
#define STACK_KEY                2
#define STACK_STRING             3

#define BUFFER_MAX_LENGTH  64

class WJsonParser {
public:
	typedef std::function<void(const char*, const char*)> TProcessKeyValueFunction;

	WJsonParser() {
		this->logging = false;
		state = STATE_START_DOCUMENT;
		bufferPos = 0;
		unicodeEscapeBufferPos = 0;
		unicodeBufferPos = 0;
		characterCounter = 0;
		kvFunction = nullptr;
	}

	WJsonParser(bool logging) {
		this->logging = logging;
		state = STATE_START_DOCUMENT;
		bufferPos = 0;
		unicodeEscapeBufferPos = 0;
		unicodeBufferPos = 0;
		characterCounter = 0;
		kvFunction = nullptr;
	}

	void parse(const char *payload, TProcessKeyValueFunction kvFunction) {
		this->kvFunction = kvFunction;
		for (unsigned int i = 0; i < strlen(payload); i++) {
			parseChar(payload[i]);
		}
	}

	WProperty* parse(const char *payload, WDevice *device) {
		this->device = device;
		WProperty* result = nullptr;
		for (unsigned int i = 0; i < strlen(payload); i++) {
			WProperty* p = parseChar(payload[i]);
			if (p != nullptr) {
				result = p;
			}
		}
		return result;
	}

private:
	int state;
	int stack[20];
	int stackPos = 0;
	bool doEmitWhitespace = false;
	char buffer[BUFFER_MAX_LENGTH];
	int bufferPos = 0;
	char unicodeEscapeBuffer[10];
	int unicodeEscapeBufferPos = 0;
	char unicodeBuffer[10];
	int unicodeBufferPos = 0;
	int characterCounter = 0;
	int unicodeHighSurrogate = 0;
	bool logging = false;
	String currentKey;
	WDevice *device = nullptr;
	TProcessKeyValueFunction kvFunction;

	void log(String message) {
		if (logging) {
			Serial.println(message);
		}
	}

	WProperty* processKeyValue(const char* key, const char* value) {
		WProperty* result = nullptr;
		if (device != nullptr) {
			result = device->getPropertyById(key);
			if (result != nullptr) {
				if (!result->parse(value)) {
					result = nullptr;
				}
			}

		} else if (kvFunction) {
			kvFunction(key, value);
		}
		return result;
	}

	WProperty* parseChar(char c) {
		WProperty* result = nullptr;
		if ((c == ' ' || c == '\t' || c == '\n' || c == '\r')
				&& !(state == STATE_IN_STRING || state == STATE_UNICODE
						|| state == STATE_START_ESCAPE
						|| state == STATE_IN_NUMBER
						|| state == STATE_START_DOCUMENT)) {
			return result;
		}
		switch (state) {
		case STATE_IN_STRING:
			if (c == QUOTE) {
				result = endString();
			} else if (c == '\\') {
				state = STATE_START_ESCAPE;
			} else if ((c < 0x1f) || (c == 0x7f)) {
				//throw new RuntimeException("Unescaped control character encountered: " + c + " at position" + characterCounter);
			} else {
				buffer[bufferPos] = c;
				increaseBufferPointer();
			}
			break;
		case STATE_IN_ARRAY:
			if (c == BEND) {
				endArray();
			} else {
				startValue(c);
			}
			break;
		case STATE_IN_OBJECT:
			if (c == SEND) {
				endObject();
			} else if (c == QUOTE) {
				startKey();
			} else {
				//throw new RuntimeException("Start of string expected for object key. Instead got: " + c + " at position" + characterCounter);
			}
			break;
		case STATE_END_KEY:
			if (c != DPOINT) {
				//throw new RuntimeException("Expected ':' after key. Instead got " + c + " at position" + characterCounter);
			}
			state = STATE_AFTER_KEY;
			break;
		case STATE_AFTER_KEY:
			startValue(c);
			break;
		case STATE_START_ESCAPE:
			processEscapeCharacters(c);
			break;
		case STATE_UNICODE:
			processUnicodeCharacter(c);
			break;
		case STATE_UNICODE_SURROGATE:
			unicodeEscapeBuffer[unicodeEscapeBufferPos] = c;
			unicodeEscapeBufferPos++;
			if (unicodeEscapeBufferPos == 2) {
				endUnicodeSurrogateInterstitial();
			}
			break;
		case STATE_AFTER_VALUE: {
			// not safe for size == 0!!!
			int within = stack[stackPos - 1];
			if (within == STACK_OBJECT) {
				if (c == SEND) {
					endObject();
				} else if (c == COMMA) {
					state = STATE_IN_OBJECT;
				} else {
					//throw new RuntimeException("Expected ',' or '}' while parsing object. Got: " + c + ". " + characterCounter);
				}
			} else if (within == STACK_ARRAY) {
				if (c == BEND) {
					endArray();
				} else if (c == COMMA) {
					state = STATE_IN_ARRAY;
				} else {
					//throw new RuntimeException("Expected ',' or ']' while parsing array. Got: " + c + ". " + characterCounter);

				}
			} else {
				//throw new RuntimeException("Finished a literal, but unclear what state to move to. Last state: " + characterCounter);
			}
		}
			break;
		case STATE_IN_NUMBER:
			if (c >= '0' && c <= '9') {
				buffer[bufferPos] = c;
				increaseBufferPointer();
			} else if (c == '.') {
				if (doesCharArrayContain(buffer, bufferPos, '.')) {
					//throw new RuntimeException("Cannot have multiple decimal points in a number. " + characterCounter);
				} else if (doesCharArrayContain(buffer, bufferPos, 'e')) {
					//throw new RuntimeException("Cannot have a decimal point in an exponent." + characterCounter);
				}
				buffer[bufferPos] = c;
				increaseBufferPointer();
			} else if (c == 'e' || c == 'E') {
				if (doesCharArrayContain(buffer, bufferPos, 'e')) {
					//throw new RuntimeException("Cannot have multiple exponents in a number. " + characterCounter);
				}
				buffer[bufferPos] = c;
				increaseBufferPointer();
			} else if (c == '+' || c == '-') {
				char last = buffer[bufferPos - 1];
				if (!(last == 'e' || last == 'E')) {
					//throw new RuntimeException("Can only have '+' or '-' after the 'e' or 'E' in a number." + characterCounter);
				}
				buffer[bufferPos] = c;
				increaseBufferPointer();
			} else {
				result = endNumber();
				// we have consumed one beyond the end of the number
				parseChar(c);
			}
			break;
		case STATE_IN_TRUE:
			buffer[bufferPos] = c;
			increaseBufferPointer();
			if (bufferPos == 4) {
				result = endTrue();
			}
			break;
		case STATE_IN_FALSE:
			buffer[bufferPos] = c;
			increaseBufferPointer();
			if (bufferPos == 5) {
				result = endFalse();
			}
			break;
		case STATE_IN_NULL:
			buffer[bufferPos] = c;
			increaseBufferPointer();
			if (bufferPos == 4) {
				endNull();
			}
			break;
		case STATE_START_DOCUMENT:
			//myListener->startDocument();
			if (c == BBEGIN) {
				startArray();
			} else if (c == SBEGIN) {
				startObject();
			}
			break;
		}
		characterCounter++;
		return result;
	}

	void increaseBufferPointer() {
		bufferPos = min(bufferPos + 1, BUFFER_MAX_LENGTH - 1);
	}

	WProperty* endString() {
		WProperty* result = nullptr;
		int popped = stack[stackPos - 1];
		stackPos--;
		if (popped == STACK_KEY) {
			buffer[bufferPos] = '\0';
			currentKey = String(buffer);
			state = STATE_END_KEY;
		} else if (popped == STACK_STRING) {
			if (currentKey != "") {
				buffer[bufferPos] = '\0';
				result = processKeyValue(currentKey.c_str(), buffer);
			}
			//buffer[bufferPos] = '\0';
			//myListener->value(String(buffer));
			state = STATE_AFTER_VALUE;
		} else {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Unexpected end of string.");
		}
		bufferPos = 0;
		return result;
	}

	void startValue(char c) {
		if (c == '[') {
			startArray();
		} else if (c == '{') {
			startObject();
		} else if (c == '"') {
			startString();
		} else if (isDigit(c)) {
			startNumber(c);
		} else if (c == 't') {
			state = STATE_IN_TRUE;
			buffer[bufferPos] = c;
			increaseBufferPointer();
		} else if (c == 'f') {
			state = STATE_IN_FALSE;
			buffer[bufferPos] = c;
			increaseBufferPointer();
		} else if (c == 'n') {
			state = STATE_IN_NULL;
			buffer[bufferPos] = c;
			increaseBufferPointer();
		} else {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Unexpected character for value: ".$c);
		}
	}

	bool isDigit(char c) {
		// Only concerned with the first character in a number.
		return (c >= '0' && c <= '9') || c == '-';
	}

	void endArray() {
		int popped = stack[stackPos - 1];
		stackPos--;
		if (popped != STACK_ARRAY) {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Unexpected end of array encountered.");
		}
		log("jsonParser->endArray()");
		state = STATE_AFTER_VALUE;
		if (stackPos == 0) {
			endDocument();
		}
	}

	void startKey() {
		stack[stackPos] = STACK_KEY;
		stackPos++;
		state = STATE_IN_STRING;
	}

	void endObject() {
		int popped = stack[stackPos];
		stackPos--;
		if (popped != STACK_OBJECT) {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Unexpected end of object encountered.");
		}
		log("jsonParser->endObject()");
		state = STATE_AFTER_VALUE;
		if (stackPos == 0) {
			endDocument();
		}
	}

	void processEscapeCharacters(char c) {
		if (c == '"') {
			buffer[bufferPos] = '"';
			increaseBufferPointer();
		} else if (c == '\\') {
			buffer[bufferPos] = '\\';
			increaseBufferPointer();
		} else if (c == '/') {
			buffer[bufferPos] = '/';
			increaseBufferPointer();
		} else if (c == 'b') {
			buffer[bufferPos] = 0x08;
			increaseBufferPointer();
		} else if (c == 'f') {
			buffer[bufferPos] = '\f';
			increaseBufferPointer();
		} else if (c == 'n') {
			buffer[bufferPos] = '\n';
			increaseBufferPointer();
		} else if (c == 'r') {
			buffer[bufferPos] = '\r';
			increaseBufferPointer();
		} else if (c == 't') {
			buffer[bufferPos] = '\t';
			increaseBufferPointer();
		} else if (c == 'u') {
			state = STATE_UNICODE;
		} else {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Expected escaped character after backslash. Got: ".$c);
		}
		if (state != STATE_UNICODE) {
			state = STATE_IN_STRING;
		}
	}

	void processUnicodeCharacter(char c) {
		if (!isHexCharacter(c)) {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Expected hex character for escaped Unicode character. Unicode parsed: "
			// . implode($this->_unicode_buffer) . " and got: ".$c);
		}

		unicodeBuffer[unicodeBufferPos] = c;
		unicodeBufferPos++;

		if (unicodeBufferPos == 4) {
			int codepoint = getHexArrayAsDecimal(unicodeBuffer,
					unicodeBufferPos);
			endUnicodeCharacter(codepoint);
			return;
			/*if (codepoint >= 0xD800 && codepoint < 0xDC00) {
			 unicodeHighSurrogate = codepoint;
			 unicodeBufferPos = 0;
			 state = STATE_UNICODE_SURROGATE;
			 } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
			 if (unicodeHighSurrogate == -1) {
			 // throw new ParsingError($this->_line_number,
			 // $this->_char_number,
			 // "Missing high surrogate for Unicode low surrogate.");
			 }
			 int combinedCodePoint = ((unicodeHighSurrogate - 0xD800) * 0x400) + (codepoint - 0xDC00) + 0x10000;
			 endUnicodeCharacter(combinedCodePoint);
			 } else if (unicodeHighSurrogate != -1) {
			 // throw new ParsingError($this->_line_number,
			 // $this->_char_number,
			 // "Invalid low surrogate following Unicode high surrogate.");
			 endUnicodeCharacter(codepoint);
			 } else {
			 endUnicodeCharacter(codepoint);
			 }*/
		}
	}

	bool isHexCharacter(char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
				|| (c >= 'A' && c <= 'F');
	}

	int getHexArrayAsDecimal(char hexArray[], int length) {
		int result = 0;
		for (int i = 0; i < length; i++) {
			char current = hexArray[length - i - 1];
			int value = 0;
			if (current >= 'a' && current <= 'f') {
				value = current - 'a' + 10;
			} else if (current >= 'A' && current <= 'F') {
				value = current - 'A' + 10;
			} else if (current >= '0' && current <= '9') {
				value = current - '0';
			}
			result += value * 16 ^ i;
		}
		return result;
	}

	bool doesCharArrayContain(char myArray[], int length, char c) {
		for (int i = 0; i < length; i++) {
			if (myArray[i] == c) {
				return true;
			}
		}
		return false;
	}

	void endUnicodeSurrogateInterstitial() {
		char unicodeEscape = unicodeEscapeBuffer[unicodeEscapeBufferPos - 1];
		if (unicodeEscape != 'u') {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Expected '\\u' following a Unicode high surrogate. Got: " .
			// $unicode_escape);
		}
		unicodeBufferPos = 0;
		unicodeEscapeBufferPos = 0;
		state = STATE_UNICODE;
	}

	WProperty* endNumber() {
		WProperty* result = nullptr;
		if (currentKey != "") {
			buffer[bufferPos] = '\0';
			result = processKeyValue(currentKey.c_str(), buffer);
		}
		bufferPos = 0;
		state = STATE_AFTER_VALUE;
		return result;
	}

	int convertDecimalBufferToInt(char myArray[], int length) {
		int result = 0;
		for (int i = 0; i < length; i++) {
			char current = myArray[length - i - 1];
			result += (current - '0') * 10;
		}
		return result;
	}

	void endDocument() {
		//myListener->endDocument();
		state = STATE_DONE;
	}

	WProperty* endTrue() {
		WProperty* result = false;
		if (currentKey != "") {
			buffer[bufferPos] = '\0';
			//String value = String(buffer);
			if (strcasecmp(buffer, JSON_TRUE) == 0) {
				result = processKeyValue(currentKey.c_str(), JSON_TRUE);
			}
		}
		bufferPos = 0;
		state = STATE_AFTER_VALUE;
		return result;
	}

	WProperty* endFalse() {
		WProperty* result = false;
		if (currentKey != "") {
			buffer[bufferPos] = '\0';
			//String value = String(buffer);
			if (strcasecmp(buffer, JSON_FALSE) == 0) {
				result = processKeyValue(currentKey.c_str(), JSON_FALSE);
			}
		}
		bufferPos = 0;
		state = STATE_AFTER_VALUE;
		return result;
	}

	void endNull() {
		buffer[bufferPos] = '\0';
		String value = String(buffer);
		if (value.equals(JSON_NULL)) {
			//myListener->value("null");
		} else {
			// throw new ParsingError($this->_line_number, $this->_char_number,
			// "Expected 'true'. Got: ".$true);
		}
		bufferPos = 0;
		state = STATE_AFTER_VALUE;
	}

	void startArray() {
		//myListener->startArray();
		state = STATE_IN_ARRAY;
		stack[stackPos] = STACK_ARRAY;
		stackPos++;
	}

	void startObject() {
		//myListener->startObject();
		state = STATE_IN_OBJECT;
		stack[stackPos] = STACK_OBJECT;
		stackPos++;
	}

	void startString() {
		stack[stackPos] = STACK_STRING;
		stackPos++;
		state = STATE_IN_STRING;
	}

	void startNumber(char c) {
		state = STATE_IN_NUMBER;
		buffer[bufferPos] = c;
		increaseBufferPointer();
	}

	void endUnicodeCharacter(int codepoint) {
		buffer[bufferPos] = convertCodepointToCharacter(codepoint);
		increaseBufferPointer();
		unicodeBufferPos = 0;
		unicodeHighSurrogate = -1;
		state = STATE_IN_STRING;
	}

	char convertCodepointToCharacter(int num) {
		if (num <= 0x7F)
			return (char) (num);
		return ' ';
	}

};

#endif


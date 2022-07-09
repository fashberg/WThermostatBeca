#ifndef _STRING_STREAM_H_
#define _STRING_STREAM_H_

#include <Stream.h>

class WStringStream : public Stream {
public:
    
	WStringStream(unsigned int maxLength) {
		this->maxLength = maxLength;
		this->string = new char[maxLength + 1];
		this->flush();
	}

	virtual ~WStringStream() {
		if (this->string) {
			delete[] this->string;
		}
	}

    // Stream methods
    virtual int available() {
    	return getMaxLength() - position;
    }

    virtual int read() {
    	if (position > 0) {
    		char c = string[0];
    		for (unsigned int i = 1; i <= position; i++) {
    			string[i - 1] = string[i];
    		}
			position--;
    		return c;
    	}
    	return -1;
    }

    virtual int peek() {
    	if (position > 0) {
    	    char c = string[0];
    	    return c;
    	}
    	return -1;
    }

    virtual void flush() {
    	this->position = 0;
    	this->string[0] = '\0';
    }

    // Print methods
    virtual size_t write(uint8_t c) {
    	if (position < maxLength) {
    		string[position] = (char) c;
    		position++;
    		string[position] = '\0';
    		return 1;
    	} else {
    		return 0;
    	}
    }

    unsigned int length() {
        return this->position;
    }

    unsigned int getMaxLength() {
    	return this->maxLength;
    }

    char charAt(int index) {
    	return this->string[index];
    }

	size_t printf(const char *format, va_list args) {
		size_t written = vsnprintf(&string[position], maxLength - position-1, format, args);
		position+=written;
		return written;	
	}
	size_t printf(const char *format, ...) {
		va_list args;
		va_start(args, format);
		size_t len = printf(format, args);
		va_end(args);
		return len;
	}
	size_t printf(const __FlashStringHelper *format, va_list args) {
		char buf[1024];
		size_t size = sizeof buf;
		PGM_P p = reinterpret_cast<PGM_P>(format);
		char c;
		unsigned int pos=0;
		for (; pos < size-1 && (c = pgm_read_byte(p++)) != 0; pos++) {
				buf[pos]=c;
		}
		buf[pos]='\0';
		size_t len = printf(buf, args);
		return len;
	}
	
	size_t printf(const __FlashStringHelper *format, ...) {
		va_list args;
		va_start(args, format);
		size_t len = printf(format, args);
		va_end(args);
		return len;
	}

	size_t printAndReplace(const __FlashStringHelper *format, ...) {
		va_list args;
		va_start(args, format);
		size_t len = printf(format, args);
		va_end(args);
		return len;
	}

    const char* c_str() {
		return this->string;
    }

private:
    char* string;
    unsigned int maxLength;
    unsigned int position;
};

#endif // _STRING_STREAM_H_

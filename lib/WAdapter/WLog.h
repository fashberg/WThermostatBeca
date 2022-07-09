#ifndef _W_LOG_H
#define _W_LOG_H

#include <inttypes.h>
#include <stdarg.h>
#include "Arduino.h"
#include <pgmspace.h>

typedef void (*printfunction)(Print*);

#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_NOTICE  4
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_VERBOSE 6

#define CR "\n"
#define LOGGING_VERSION 1_0_3

/**
 * Logging is a helper class to output informations over
 * RS232. If you know log4j or log4net, this logging class
 * is more or less similar ;-) <br>
 * Different loglevels can be used to extend or reduce output
 * All methods are able to handle any number of output parameters.
 * All methods print out a formated string (like printf).<br>
 * To reduce output and program size, reduce loglevel.
 * 
 * Output format string can contain below wildcards. Every wildcard
 * must be start with percent sign (\%)
 * 
 * ---- Wildcards
 * 
 * %s	replace with an string (char*)
 * %c	replace with an character
 * %d	replace with an integer value
 * %l	replace with an long value
 * %x	replace and convert integer value into hex
 * %X	like %x but combine with 0x123AB
 * %b	replace and convert integer value into binary
 * %B	like %x but combine with 0b10100011
 * %t	replace and convert boolean value into "t" or "f"
 * %T	like %t but convert into "true" or "false"
 * 
 * ---- Loglevels
 * 
 * 0 - LOG_LEVEL_SILENT     no output
 * 1 - LOG_LEVEL_FATAL      fatal errors
 * 2 - LOG_LEVEL_ERROR      all errors
 * 3 - LOG_LEVEL_WARNING    errors and warnings
 * 4 - LOG_LEVEL_NOTICE     errors, warnings and notices
 * 5 - LOG_LEVEL_TRACE      errors, warnings, notices, traces
 * 6 - LOG_LEVEL_VERBOSE    all
 */

class WLog {
public:
	WLog(int levelConsole, int levelNetwork, Print *output, bool showLevelConsole = true) {
		setLevelConsole(levelConsole);
		setShowLevelConsole(showLevelConsole);
		setLevelNetwork(levelNetwork);
		_logOutput = output;
	}

	void setLevelConsole(int level) {
		_levelConsole = constrain(level, LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE);
	}
	void setLevelNetwork(int level) {
		_levelNetwork = constrain(level, LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE);
	}

	void setShowLevelConsole(bool showLevelConsole) {
		_showLevelConsole = showLevelConsole;
	}

	void setPrefix(printfunction f) {
		_prefix = f;
	}

	void setSuffix(printfunction f) {
		_suffix = f;
	}

	template<class T, typename ... Args> void fatal(T msg, Args ... args) {
		printLevel(LOG_LEVEL_FATAL, msg, args...);
	}

	template<class T, typename ... Args> void error(T msg, Args ... args) {
		printLevel(LOG_LEVEL_ERROR, msg, args...);
	}

	template<class T, typename ... Args> void warning(T msg, Args ...args) {
		printLevel(LOG_LEVEL_WARNING, msg, args...);
	}

	template<class T, typename ... Args> void notice(T msg, Args ...args) {
		printLevel(LOG_LEVEL_NOTICE, msg, args...);
	}

	template<class T, typename ... Args> void trace(T msg, Args ... args) {
		printLevel(LOG_LEVEL_TRACE, msg, args...);
	}

	template<class T, typename ... Args> void verbose(T msg, Args ... args) {
		printLevel(LOG_LEVEL_VERBOSE, msg, args...);
	}

    typedef std::function<void(int level, const char * message)> TCommandHandlerFunction;

	void setOnLogCommand(TCommandHandlerFunction LogCommand) {
    	this->onLogCommand = LogCommand;
    }

	template<class T> void printLevel(int level, T msg, ...) {
		if (level > _levelConsole && level > _levelNetwork) {
			return;
		}

		if (_prefix != NULL) {
			_prefix(_logOutput);
		}

		if (_showLevelConsole && level <=_levelConsole) {
			static const char levels[] = "FEWNTV";
			_logOutput->print(levels[level - 1]);
			_logOutput->print(": ");
		}

		va_list args;
		va_start(args, msg);
		print(level, msg, args);
		va_end(args);

		if (_suffix != NULL && level <=_levelConsole) {
			_suffix(_logOutput);
		}
	}

private:
    TCommandHandlerFunction onLogCommand;

	void print(int level, const char *format, va_list args) {
		char logbuf[256];
		size_t size = sizeof logbuf;
		vsnprintf_P(logbuf, size-1, format, args);
		if (level <=_levelConsole) _logOutput->println(logbuf);
   		if (onLogCommand && level <=_levelNetwork) {
    		onLogCommand(level, logbuf );
    	}
    }

	void print(int level, const __FlashStringHelper *format, va_list args) {
		char logbuf[256];
		size_t size = sizeof logbuf;
		PGM_P p = reinterpret_cast<PGM_P>(format);
		char c;
		unsigned int pos=0;
		for(; pos < size-1 && (c = pgm_read_byte(p++)) != 0; pos++) {
				logbuf[pos]=c;	
		}
		logbuf[pos]=0;
		print(level, logbuf, args);
	}



	int _levelConsole;
	int _levelNetwork;
	bool _showLevelConsole;
	Print *_logOutput;

	printfunction _prefix = NULL;
	printfunction _suffix = NULL;

};

#endif


#ifndef LOGDEVICE_H
#define	LOGDEVICE_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "../lib/WAdapter/WAdapter/WDevice.h"

const char* ID_LOGDEVICE PROGMEM = "logging";
const char* NAME_LOGDEVICE PROGMEM = "Logging";

const char* PROP_LOGLEVELBYTE PROGMEM = "logLevelByte";
const char* PROP_LOGLEVEL PROGMEM = "logLevel";
const char* TITL_LOGLEVEL PROGMEM = "LogLevel";

const char* ID_PAGE_LIVEWEBLOG PROGMEM = "weblog";
const char* TITLE_PAGE_LIVEWEBLOG PROGMEM = "Live WebLog";

const char* LOG_MODE_SILENT  PROGMEM = "silent";
const char* LOG_MODE_FATAL   PROGMEM = "fatal";
const char* LOG_MODE_ERROR   PROGMEM = "error";
const char* LOG_MODE_WARNING PROGMEM = "warning";
const char* LOG_MODE_NOTICE  PROGMEM = "notice";
const char* LOG_MODE_TRACE   PROGMEM = "trace";
const char* LOG_MODE_VERBOSE PROGMEM = "verbose";

const static char HTTP_WEBLOG[]         PROGMEM = R"=====(
	<textarea readonly id="t1" cols="340" rows="25" wrap="off" name="t1" style="resize:vertical;width:98%;height:318px;padding:5px;overflow:auto;background:#1f1f1f;color:#65c115"></textarea>
)=====";



const static char HTTP_WEBLOG_SCRIPT[]             PROGMEM = R"=====(<script>
	var ws = new WebSocket("ws://" + location.host + "/ws", ["log"]);
	exampleSocket.onmessage = function(event){
		eb('t1').value+=event.data;
	}
</script>
)=====";

class WLogDevice: public WDevice {
public:

    WLogDevice(WNetwork* network) 
        	: WDevice(network, ID_LOGDEVICE, NAME_LOGDEVICE, network->getIdx(), DEVICE_TYPE_LOG) {
		
		this->providingConfigPage = true;
		this->configNeedsReboot = false;
		this->mainDevice = false;
		
		/* properties */

		if (network->getSettingsOld()){
			if (network->getSettingsOld()->getNetworkSettingsVersion()==NETWORKSETTINGS_PRE_FAS114){
				network->log()->notice(F("Reading WLogSettings PRE_FAS114"));
				network->getSettingsOld()->setByte(PROP_LOGLEVELBYTE, LOG_LEVEL_WARNING);
			}
		}


		this->logLevelByte = network->getSettings()->setByte(PROP_LOGLEVELBYTE,
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_LOGLEVELBYTE) ? network->getSettingsOld()->getByte(PROP_LOGLEVELBYTE) : LOG_LEVEL_WARNING));
		this->logLevelByte->setVisibility(NONE);
    	this->addProperty(logLevelByte);
		setlogLevelByte(constrain(getlogLevelByte(),LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE ));
    	this->logLevel = new WProperty(PROP_LOGLEVEL, TITL_LOGLEVEL, STRING);
    	this->logLevel->setAtType("logLevel");
    	this->logLevel->addEnumString(LOG_MODE_SILENT);
    	this->logLevel->addEnumString(LOG_MODE_FATAL);
    	this->logLevel->addEnumString(LOG_MODE_ERROR);
    	this->logLevel->addEnumString(LOG_MODE_WARNING);
    	this->logLevel->addEnumString(LOG_MODE_NOTICE);
    	this->logLevel->addEnumString(LOG_MODE_TRACE);
    	this->logLevel->addEnumString(LOG_MODE_VERBOSE);
		this->logLevel->setVisibility(MQTT);
    	this->addProperty(logLevel);
		// first apply stored Byte value from EEPROM
		this->setlogLevel(getlogLevelByte());
		// then set Handler, because now logLevelByte follows String logLevel
		this->logLevel->setOnChange(std::bind(&WLogDevice::onlogLevelChange, this, std::placeholders::_1));

		// Pages		
		WPage * weblogPage=new WPage(ID_PAGE_LIVEWEBLOG, TITLE_PAGE_LIVEWEBLOG);
		weblogPage->setPrintPage([this,weblogPage](AsyncWebServerRequest *request, AsyncResponseStream* page) {
			this->network->log()->notice(PSTR("weblog"));
			page->print(FPSTR(HTTP_WEBLOG));
			page->print(FPSTR(HTTP_HOME_BUTTON));
			page->print(FPSTR(HTTP_WEBLOG_SCRIPT));
		});
		this->addPage(weblogPage);
    
    }

    virtual void printConfigPage(AsyncWebServerRequest *request, AsyncResponseStream* page) {
    	network->log()->notice(F("Log config page"));
    	page->printf_P(HTTP_CONFIG_PAGE_BEGIN, getId());
    	//ComboBox with model selection

    	page->printf_P(HTTP_COMBOBOX_BEGIN, F("Log Mode (Logging to MQTT Only!):"), "lm");
    	page->printf_P(HTTP_COMBOBOX_ITEM), "0", (getlogLevelByte() == LOG_LEVEL_SILENT  ? HTTP_SELECTED : "", F("Logging Disabled"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "1", (getlogLevelByte() == LOG_LEVEL_FATAL   ? HTTP_SELECTED : "", F("Fatal Messages"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "2", (getlogLevelByte() == LOG_LEVEL_ERROR   ? HTTP_SELECTED : "", F("Error Messages"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "3", (getlogLevelByte() == LOG_LEVEL_WARNING ? HTTP_SELECTED : "", F("Warning Messages"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "4", (getlogLevelByte() == LOG_LEVEL_NOTICE  ? HTTP_SELECTED : "", F("Notice Messages"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "5", (getlogLevelByte() == LOG_LEVEL_TRACE   ? HTTP_SELECTED : "", F("Trace Messages"));
    	page->printf_P(HTTP_COMBOBOX_ITEM), "6", (getlogLevelByte() == LOG_LEVEL_VERBOSE ? HTTP_SELECTED : "", F("Verbose Messages"));
    	page->print(HTTP_COMBOBOX_END);

    	page->print(HTTP_CONFIG_SAVE_BUTTON);
    }
    void saveConfigPage(AsyncWebServerRequest *request) {
        network->log()->notice(F("Log Beca config save lm=%s/%d"), request->getParam("lm")->value().c_str(), request->getParam("lm")->value().toInt());
        setlogLevelByte(constrain(request->getParam("lm")->value().toInt(),LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE ));
		this->setlogLevel(getlogLevelByte());
    }

    void loop(unsigned long now) {
        /* noop */
    }

    void handleUnknownMqttCallback(String stat_topic, String partialTopic, String payload, unsigned int length) {
		//logCommand(((String)"handleUnknownMqttCallback " + stat_topic + " / " + partialTopic + " / " + payload).c_str());
    }

	byte getlogLevelByte() {
    	return logLevelByte->getByte();
    }
	void setlogLevelByte(byte lm) {
		network->log()->notice(F("WLogDevice setlogLevelByte (%d)"), lm);
    	logLevelByte->setByte(lm);
		network->log()->setLevelNetwork(lm);
    }

    void onlogLevelChange(WProperty* property) {
		if (property->equalsString(LOG_MODE_SILENT)) setlogLevelByte(LOG_LEVEL_SILENT);
		else if (property->equalsString(LOG_MODE_FATAL)) setlogLevelByte(LOG_LEVEL_FATAL);
		else if (property->equalsString(LOG_MODE_ERROR)) setlogLevelByte(LOG_LEVEL_ERROR);
		else if (property->equalsString(LOG_MODE_WARNING)) setlogLevelByte(LOG_LEVEL_WARNING);
		else if (property->equalsString(LOG_MODE_NOTICE)) setlogLevelByte(LOG_LEVEL_NOTICE);
		else if (property->equalsString(LOG_MODE_TRACE)) setlogLevelByte(LOG_LEVEL_TRACE);
		else if (property->equalsString(LOG_MODE_VERBOSE)) setlogLevelByte(LOG_LEVEL_VERBOSE);
		else setlogLevelByte(LOG_LEVEL_SILENT);
    }

    void setlogLevel(byte lm) {
		const char * lms;
		lms=logLevelByteToString(lm);
		logLevel->setString(lms);
    }

	const char * logLevelByteToString(byte lm) {
		if (lm == LOG_LEVEL_SILENT) return LOG_MODE_SILENT;
		else if (lm == LOG_LEVEL_FATAL) return LOG_MODE_FATAL;
		else if (lm == LOG_LEVEL_ERROR) return LOG_MODE_ERROR;
		else if (lm == LOG_LEVEL_WARNING) return LOG_MODE_WARNING;
		else if (lm == LOG_LEVEL_NOTICE) return LOG_MODE_NOTICE;
		else if (lm == LOG_LEVEL_TRACE) return LOG_MODE_TRACE;
		else if (lm == LOG_LEVEL_VERBOSE) return LOG_MODE_VERBOSE;
		else return LOG_MODE_SILENT;
    }

	void sendLog(int level, const char* message) {
		String t = (String)network->getMqttTopic()+ "/" + MQTT_TELE +"/log" ;
		//network->log()->verbose(F("sendLog (%s)"), t.c_str());
		if (network->isMqttConnected()){
			network->publishMqtt(t.c_str(), ((String)logLevelByteToString(level)+": "+message).c_str());
		}
		//https://github.com/me-no-dev/EspExceptionDecoder
	};

private:
    WProperty* logLevel;
    WProperty* logLevelByte;

};


#endif
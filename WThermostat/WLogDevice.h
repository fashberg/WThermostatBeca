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
const char* TITL_LOGLEVEL PROGMEM = "logLevel";
const char* PROP_LOGLEVELWEB PROGMEM = "logLevelWeb";
const char* PROP_LOGLEVELWEBBYTE PROGMEM = "logLevelWebByte";

const char* ID_PAGE_LIVEWEBLOG PROGMEM = "weblog";
const char* TITLE_PAGE_LIVEWEBLOG PROGMEM = "Live WebLog";

const char* LOG_MODE_SILENT  PROGMEM = "silent";
const char* LOG_MODE_FATAL   PROGMEM = "fatal";
const char* LOG_MODE_ERROR   PROGMEM = "error";
const char* LOG_MODE_WARNING PROGMEM = "warning";
const char* LOG_MODE_NOTICE  PROGMEM = "notice";
const char* LOG_MODE_TRACE   PROGMEM = "trace";
const char* LOG_MODE_VERBOSE PROGMEM = "verbose";


const char* URI_WSLOG PROGMEM = "/wslog";

const static char HTTP_WEBLOG[]         PROGMEM = R"=====(
	<textarea readonly id="t1" cols="340" rows="25" wrap="off" name="t1" style="resize:vertical;width:98%;height:318px;padding:5px;overflow:auto;background:#1f1f1f;color:#65c115"></textarea>
)=====";



const static char HTTP_WEBLOG_SCRIPT[]             PROGMEM = R"=====(<script>
	var ws = null;
	function ping() {
		if (!ws) return;
		if (ws.readyState !== 1) return;
		ws.send("heartbeat");
		setTimeout(ping, 3000);
	}
	function wsinit(){
		ws = new WebSocket("ws://" + location.host + "/wslog", ["log"]);
		ws.onopen = function(){
			eb('t1').value+='Connected\n';
			eb('t1').scrollTop=99999;
			setInterval(function(){ping();}, 3000);
		}
		ws.onmessage = function(e){
			eb('t1').value+=e.data;
			eb('t1').scrollTop=99999;
		}
		ws.onclose = function(e){
			eb('t1').value+='Connection Closed! '+e.reason+'\n\n';
			eb('t1').scrollTop=99999;
			setTimeout(function(){ wsinit(); }, 3000);
		}
		ws.onerror = function(err) {
			eb('t1').value+='Error! '+err.message+'\n\n';
			eb('t1').scrollTop=99999;
			setTimeout(function(){ wsinit(); }, 3000);
			ws.close();
		}
	}
	wsinit();
</script>
)=====";

AsyncWebSocket wslog(URI_WSLOG);

class WLogDevice: public WDevice {
public:

    WLogDevice(WNetwork* network) 
        	: WDevice(network, ID_LOGDEVICE, NAME_LOGDEVICE, network->getIdx(), DEVICE_TYPE_LOG) {
		
		this->providingConfigPage = true;
		this->configNeedsReboot = false;
		this->mainDevice = false;
		this->visibility = MQTT;

		/* properties */

		if (network->getSettingsOld()){
			if (network->getSettingsOld()->getNetworkSettingsVersion()==NETWORKSETTINGS_PRE_FAS114){
				network->log()->notice(F("Reading WLogSettings PRE_FAS114"));
				network->getSettingsOld()->setByte(PROP_LOGLEVELBYTE, LOG_LEVEL_WARNING);
			}
		}

		logLevelByte=nullptr;
		logLevelWebByte=nullptr;
		this->logLevelByte = network->getSettings()->setByte(PROP_LOGLEVELBYTE,
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_LOGLEVELBYTE) ? network->getSettingsOld()->getByte(PROP_LOGLEVELBYTE) : LOG_LEVEL_WARNING));
		this->logLevelByte->setVisibility(NONE);
    	this->addProperty(logLevelByte);
		setlogLevelByte(constrain(getlogLevelByte(),LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE ));
		this->logLevelWebByte = new WProperty(PROP_LOGLEVELWEBBYTE, NULL, BYTE);
		setlogLevelWebByte(LOG_LEVEL_TRACE);
		this->logLevel = new WProperty(PROP_LOGLEVEL, TITL_LOGLEVEL, STRING, 8);
		this->logLevel->setAtType(PROP_LOGLEVEL);
		this->logLevel->addEnumString(LOG_MODE_SILENT);
		this->logLevel->addEnumString(LOG_MODE_FATAL);
		this->logLevel->addEnumString(LOG_MODE_ERROR);
		this->logLevel->addEnumString(LOG_MODE_WARNING);
		this->logLevel->addEnumString(LOG_MODE_NOTICE);
		this->logLevel->addEnumString(LOG_MODE_TRACE);
		this->logLevel->addEnumString(LOG_MODE_VERBOSE);
		this->logLevel->setVisibility(MQTT);
		this->addProperty(logLevel);
		this->logLevelWeb = new WProperty(PROP_LOGLEVEL, TITL_LOGLEVEL, STRING, 8);
		this->logLevelWeb->setAtType(PROP_LOGLEVELWEB);
		this->logLevelWeb->addEnumString(LOG_MODE_SILENT);
		this->logLevelWeb->addEnumString(LOG_MODE_FATAL);
		this->logLevelWeb->addEnumString(LOG_MODE_ERROR);
		this->logLevelWeb->addEnumString(LOG_MODE_WARNING);
		this->logLevelWeb->addEnumString(LOG_MODE_NOTICE);
		this->logLevelWeb->addEnumString(LOG_MODE_TRACE);
		this->logLevelWeb->addEnumString(LOG_MODE_VERBOSE);
		this->logLevelWeb->setVisibility(MQTT);
		this->addProperty(logLevelWeb);

		// first apply stored Byte value from EEPROM
		this->setlogLevelMqtt(getlogLevelByte());
		this->setlogLevelWeb(getlogLevelWebByte());
		// then set Handler, because now logLevelByte follows String logLevel
		this->logLevel->setOnChange(std::bind(&WLogDevice::onlogLevelMqttChange, this, std::placeholders::_1));
		this->logLevelWeb->setOnChange(std::bind(&WLogDevice::onlogLevelWebChange, this, std::placeholders::_1));

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

		page->printf_P(HTTP_COMBOBOX_BEGIN, F("MQTT Log Mode (Permanent):"), "lm");
		page->printf_P(HTTP_COMBOBOX_ITEM, "0", (getlogLevelByte() == LOG_LEVEL_SILENT  ? HTTP_SELECTED : ""), F("Logging Disabled"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "1", (getlogLevelByte() == LOG_LEVEL_FATAL   ? HTTP_SELECTED : ""), F("Fatal Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "2", (getlogLevelByte() == LOG_LEVEL_ERROR   ? HTTP_SELECTED : ""), F("Error Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "3", (getlogLevelByte() == LOG_LEVEL_WARNING ? HTTP_SELECTED : ""), F("Warning Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "4", (getlogLevelByte() == LOG_LEVEL_NOTICE  ? HTTP_SELECTED : ""), F("Notice Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "5", (getlogLevelByte() == LOG_LEVEL_TRACE   ? HTTP_SELECTED : ""), F("Trace Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "6", (getlogLevelByte() == LOG_LEVEL_VERBOSE ? HTTP_SELECTED : ""), F("Verbose Messages"));
		page->printf_P(HTTP_COMBOBOX_END);

		page->printf_P(HTTP_COMBOBOX_BEGIN, F("Web Log Mode (Is set to Trace at next reboot):"), "lw");
		page->printf_P(HTTP_COMBOBOX_ITEM, "0", (getlogLevelWebByte() == LOG_LEVEL_SILENT  ? HTTP_SELECTED : ""), F("Logging Disabled"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "1", (getlogLevelWebByte() == LOG_LEVEL_FATAL   ? HTTP_SELECTED : ""), F("Fatal Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "2", (getlogLevelWebByte() == LOG_LEVEL_ERROR   ? HTTP_SELECTED : ""), F("Error Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "3", (getlogLevelWebByte() == LOG_LEVEL_WARNING ? HTTP_SELECTED : ""), F("Warning Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "4", (getlogLevelWebByte() == LOG_LEVEL_NOTICE  ? HTTP_SELECTED : ""), F("Notice Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "5", (getlogLevelWebByte() == LOG_LEVEL_TRACE   ? HTTP_SELECTED : ""), F("Trace Messages"));
		page->printf_P(HTTP_COMBOBOX_ITEM, "6", (getlogLevelWebByte() == LOG_LEVEL_VERBOSE ? HTTP_SELECTED : ""), F("Verbose Messages"));
		page->printf_P(HTTP_COMBOBOX_END);

    	page->printf_P(HTTP_CONFIG_SAVE_BUTTON);
    }
    void saveConfigPage(AsyncWebServerRequest *request) {
		network->log()->notice(F("Log Beca config save lm=%s/%d"), getValueOrEmpty(request, "lm").c_str(), getValueOrEmpty(request, "lm").toInt());
		setlogLevelByte(constrain(getValueOrEmpty(request, "lm").toInt(),LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE ));
		this->setlogLevelMqtt(getlogLevelByte());
		network->log()->notice(F("Log Beca config save lw=%s/%d"), getValueOrEmpty(request, "lw").c_str(), getValueOrEmpty(request, "lw").toInt());
		setlogLevelWebByte(constrain(getValueOrEmpty(request, "lw").toInt(),LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE ));
		this->setlogLevelWeb(getlogLevelWebByte());
    }

    void loop(unsigned long now) {
        /* noop */
    }

    void handleUnknownMqttCallback(String stat_topic, String partialTopic, String payload, unsigned int length) {
		//logCommand(((String)"handleUnknownMqttCallback " + stat_topic + " / " + partialTopic + " / " + payload).c_str());
    }

	byte getlogLevelByte() {
		if (!logLevelByte) return LOG_LEVEL_SILENT;
		return logLevelByte->getByte();
	}
	byte getlogLevelWebByte() {
		if (!logLevelWebByte) return LOG_LEVEL_SILENT;
		return logLevelWebByte->getByte();
	}
	void setlogLevelByte(byte lm) {
		network->log()->notice(F("WLogDevice setlogLevelByte (%d)"), lm);
		logLevelByte->setByte(lm);
		setMinLogLevel();
	}
	void setlogLevelWebByte(byte lw) {
		network->log()->notice(F("WLogDevice setlogLevelWebByte (%d)"), lw);
		logLevelWebByte->setByte(lw);
		setMinLogLevel();
	}

	void setMinLogLevel(){
		unsigned int toSet=LOG_LEVEL_SILENT;
		if (getlogLevelByte() > toSet) toSet=getlogLevelByte();
		if (getlogLevelWebByte() > toSet) toSet=getlogLevelWebByte();
		network->log()->setLevelNetwork(toSet);

	}

    void onlogLevelMqttChange(WProperty* property) {
		if (property->equalsString(LOG_MODE_SILENT)) setlogLevelByte(LOG_LEVEL_SILENT);
		else if (property->equalsString(LOG_MODE_FATAL)) setlogLevelByte(LOG_LEVEL_FATAL);
		else if (property->equalsString(LOG_MODE_ERROR)) setlogLevelByte(LOG_LEVEL_ERROR);
		else if (property->equalsString(LOG_MODE_WARNING)) setlogLevelByte(LOG_LEVEL_WARNING);
		else if (property->equalsString(LOG_MODE_NOTICE)) setlogLevelByte(LOG_LEVEL_NOTICE);
		else if (property->equalsString(LOG_MODE_TRACE)) setlogLevelByte(LOG_LEVEL_TRACE);
		else if (property->equalsString(LOG_MODE_VERBOSE)) setlogLevelByte(LOG_LEVEL_VERBOSE);
		else setlogLevelByte(LOG_LEVEL_SILENT);
    }

	void onlogLevelWebChange(WProperty* property) {
		if (property->equalsString(LOG_MODE_SILENT)) setlogLevelWebByte(LOG_LEVEL_SILENT);
		else if (property->equalsString(LOG_MODE_FATAL)) setlogLevelWebByte(LOG_LEVEL_FATAL);
		else if (property->equalsString(LOG_MODE_ERROR)) setlogLevelWebByte(LOG_LEVEL_ERROR);
		else if (property->equalsString(LOG_MODE_WARNING)) setlogLevelWebByte(LOG_LEVEL_WARNING);
		else if (property->equalsString(LOG_MODE_NOTICE)) setlogLevelWebByte(LOG_LEVEL_NOTICE);
		else if (property->equalsString(LOG_MODE_TRACE)) setlogLevelWebByte(LOG_LEVEL_TRACE);
		else if (property->equalsString(LOG_MODE_VERBOSE)) setlogLevelWebByte(LOG_LEVEL_VERBOSE);
		else setlogLevelWebByte(LOG_LEVEL_SILENT);
	}

	void setlogLevelMqtt(byte lm) {
		const char * lms;
		lms=logLevelByteToString(lm);
		logLevel->setString(lms);
	}

	void setlogLevelWeb(byte lw) {
		const char * lws;
		lws=logLevelByteToString(lw);
		logLevelWeb->setString(lws);
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

		if (level>=getlogLevelWebByte()){
			for (unsigned int i=1; i<=wslog.count();i++){
				wslog.client(i)->printf(PSTR("%s: %s\n"), logLevelByteToString(level), message);
			}
		}
		if (level>=getlogLevelByte()){
			String t = (String)network->getMqttTopic()+ "/" + MQTT_TELE +"/log" ;
			//network->log()->verbose(F("sendLog (%s)"), t.c_str());
			if (network->isMqttConnected()){
				network->publishMqtt(t.c_str(), ((String)logLevelByteToString(level)+": "+message).c_str());
			}
		}
	};

	

	void wsOnEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
		if (type == WS_EVT_CONNECT){
			client->printf(PSTR("Hello WebSocket Client #%u\n"), client->id());
			client->printf(PSTR("Now you see all logs for logSeverity>=%s\n"), logLevelWeb->c_str());
			client->ping();
		} else if (type == WS_EVT_DATA){
			AwsFrameInfo * info = (AwsFrameInfo*)arg;
			if (info->final && info->index == 0 && info->len == len){
				if (info->opcode == WS_TEXT){
					//client->text("I got your text message\n");
					data[len] = 0;
					//os_printf("%s\n", (char*)data);
				} else {
					//client->binary("I got your binary message\n");
				}
			}
			client->ping();
		} else if (type == WS_EVT_PONG){
		}
	}

void webserverInitHook(AsyncWebServer *webServer) {
	wslog.onEvent([this](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
		this->wsOnEvent(server, client, type, arg, data, len);
	});
	webServer->addHandler(&wslog);
}
void webserverDeinitHook(AsyncWebServer *webServer) {
	wslog.closeAll();
}

private:
	WProperty* logLevel;
	WProperty* logLevelByte;
	WProperty* logLevelWeb;
	WProperty* logLevelWebByte;

};


#endif
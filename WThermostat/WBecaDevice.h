#ifndef BECAMCU_H
#define	BECAMCU_H 

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "../lib/WAdapter/WAdapter/WDevice.h"
#include "../lib/WAdapter/WAdapter/WPage.h"
#include "WClock.h"

const static char HTTP_CONFIG_SCHTAB_HEAD[]         PROGMEM = R"=====(
	<h2>Schedules</h2>
	<table class="settingstable">
		<thead><tr>
		<th>Period</th>
		<th>Workday/Days 1-5</th>
		<th>Saturday/Day 6</th>
		<th>Sunday/Day 7</th>
		</tr></thead>
	<tbody>
)=====";
const static char HTTP_CONFIG_SCHTAB_TD[]         PROGMEM = R"=====(
	<td>
		Time: <input type="text" name="%c%ch" value="%s" maxlength="5" lenght="5">
		Temp: <input type="text" name="%c%ct" value="%s" maxlength="4" length="5">
	</td>
)=====";
const static char HTTP_CONFIG_SCHTAB_FOOT[]         PROGMEM = R"=====(
	</tbody></table>
)=====";

// see https://www.home-assistant.io/docs/mqtt/discovery/ 
// and https://www.home-assistant.io/integrations/climate.mqtt/
// LWT = Last Will & Testament
const static char MQTT_HASS_AUTODISCOVERY_CLIMATE[]         PROGMEM = R"=====(
{
"name":"%s",
"unique_id": "%s",
"dev":{"ids":["%s"],"name":"%s","mdl":"%s","sw":"%s","mf":"WThermostatBeca"},
"~": "%s",
"avty_t":"~/tele/LWT",
"pl_avail":"Online",
"pl_not_avail":"Offline",
"act_t":"~/stat/things/thermostat/properties",
"act_tpl":"{{value_json.action}}",
"mode_cmd_t":"~/cmnd/things/thermostat/properties/mode",
"mode_stat_t":"~/stat/things/thermostat/properties",
"mode_stat_tpl":"{{value_json.mode}}",
"away_mode_cmd_t":"~/cmnd/things/thermostat/properties/ecoMode",
"away_mode_stat_t":"~/stat/things/thermostat/properties",
"away_mode_stat_tpl":"{{value_json.ecoMode}}",
"temp_cmd_t":"~/cmnd/things/thermostat/properties/targetTemperature",
"temp_stat_t":"~/stat/things/thermostat/properties",
"temp_stat_tpl":"{{value_json.targetTemperature}}",
"curr_temp_t":"~/stat/things/thermostat/properties",
"curr_temp_tpl":"{{value_json.temperature}}",
"pl_on":true,
"pl_off":false,
"min_temp":"10",
"max_temp":"35",
"temp_step":"%s",
"modes":["heat","auto","off"]
}
)=====";
const static char MQTT_HASS_AUTODISCOVERY_AIRCO[]         PROGMEM = R"=====(
{
"name":"%s",
"unique_id": "%s",
"dev":{"ids":["%s"],"name":"%s","mdl":"%s","sw":"%s","mf":"WThermostatBeca"},
"~": "%s",
"avty_t":"~/tele/LWT",
"pl_avail":"Online",
"pl_not_avail":"Offline",
"act_t":"~/stat/things/thermostat/properties",
"act_tpl":"{{value_json.action}}",
"mode_cmd_t":"~/cmnd/things/thermostat/properties/mode",
"mode_stat_t":"~/stat/things/thermostat/properties",
"mode_stat_tpl":"{{value_json.mode}}",
"away_mode_cmd_t":"~/cmnd/things/thermostat/properties/ecoMode",
"away_mode_stat_t":"~/stat/things/thermostat/properties",
"away_mode_stat_tpl":"{{value_json.ecoMode}}",
"temp_cmd_t":"~/cmnd/things/thermostat/properties/targetTemperature",
"temp_stat_t":"~/stat/things/thermostat/properties",
"temp_stat_tpl":"{{value_json.targetTemperature}}",
"curr_temp_t":"~/stat/things/thermostat/properties",
"curr_temp_tpl":"{{value_json.temperature}}",
"min_temp":"10",
"fan_mode_cmd_t":"~/cmnd/things/thermostat/properties/fanMode",
"fan_mode_stat_tpl":"~/stat/things/thermostat/properties",
"fan_mode_stat_t":"{{value_json.fanMode}}",
"pl_on":true,
"pl_off":false,
"min_temp":"10",
"max_temp":"35",
"temp_step":"%s",
"modes":["heat","cool","auto","fan_only","off"]
}
)=====";
const static char MQTT_HASS_AUTODISCOVERY_SENSOR[]         PROGMEM = R"=====(
{
"name":"%s Temperature",
"unique_id":"%s",
"device_class":"temperature",
"dev":{"ids":["%s"]},
"~":"%s",
"stat_t":"~/stat/things/thermostat/properties",
"val_tpl":"{{value_json.temperature}}",
"unit_of_measurement":"°C"
}
)=====";
const static char MQTT_HASS_AUTODISCOVERY_SENSORFLOOR[]         PROGMEM = R"=====(
{
"name":"%s Temperature Floor",
"unique_id":"%s",
"device_class":"temperature",
"dev":{"ids":["%s"]},
"~":"%s",
"stat_t":"~/stat/things/thermostat/properties",
"val_tpl":"{{value_json.floorTemperature}}",
"unit_of_measurement":"°C"
}
)=====";
const static char MQTT_HASS_AUTODISCOVERY_SENSORRSSI[]         PROGMEM = R"=====(
{
"name":"%s WiFi RSSI",
"unique_id":"%s",
"device_class":"signal_strength",
"dev":{"ids":["%s"]},
"~":"%s",
"stat_t":"~/stat/things/network/properties",
"val_tpl":"{{value_json.rssi}}",
"unit_of_measurement":"dBm"
}
)=====";


#define COUNT_DEVICE_MODELS 2
#define MODEL_BHT_002_GBLW 0
#define MODEL_BAC_002_ALW 1
#define HEARTBEAT_INTERVAL 10000
#define MINIMUM_INTERVAL 2000
#define STATE_COMPLETE 5
#define PIN_STATE_HEATING_RELAY 5
#define PIN_STATE_COOLING_RELAY 4
#define ECOMODETEMP 20.0

const unsigned char COMMAND_START[] = {0x55, 0xAA};
const char AR_COMMAND_END = '\n';
const String SCHEDULES = "schedules"; 
const String MCUCOMMAND = "mcucommand"; 
const char* SCHEDULES_MODE_OFF = "off";
const char* SCHEDULES_MODE_AUTO = "auto";
const char* SYSTEM_MODE_NONE = "none";
const char* SYSTEM_MODE_COOL = "cool";
const char* SYSTEM_MODE_HEAT = "heat";
const char* SYSTEM_MODE_FAN = "fan_only";
const char* STATE_OFF = "off";
const char* STATE_HEATING = "heating";
const char* STATE_COOLING = "cooling";
const char* FAN_MODE_NONE = "none";
const char* FAN_MODE_AUTO = "auto";
const char* FAN_MODE_LOW  = "low";
const char* FAN_MODE_MEDIUM  = "medium";
const char* FAN_MODE_HIGH = "high";
const char* MODE_OFF  = "off";
const char* MODE_AUTO = "auto";
const char* MODE_AUTOHEAT = "autoheat";
const char* MODE_AUTOCOOL = "autocool";
const char* MODE_AUTOFAN = "autofan";
const char* MODE_HEAT = "heat";
const char* MODE_COOL = "cool";
const char* MODE_FAN  = "fan_only";
const char* ACTION_OFF  = "off";
const char* ACTION_COOLING  = "cooling";
const char* ACTION_HEATING  = "heating";
const char* ACTION_IDLE  = "idle";
const char* ACTION_FAN = "fan";

const byte STORED_FLAG_BECA = 0x36;
const char SCHEDULES_PERIODS[] = "123456";
const char SCHEDULES_DAYS[] = "wau";

const byte BECABITS1_RELAIS_HEAT    =   1;
const byte BECABITS1_RELAIS_COOL    =   2;
const byte BECABITS1_TEMP_01        =   4;
const byte BECABITS1_TEMP_10        =   8;
const byte BECABITS1_SWITCHBACKOFF  =  16;
const byte BECABITS1_FLOORSENSOR    =  32;

const byte devicesWithHassAutodiscoverSupport[] = {
	MODEL_BHT_002_GBLW,
	MODEL_BAC_002_ALW
	};

//typedef 
typedef enum mcuNetworkMode
{
	MCU_NETWORKMODE_SMARTCONFIG = 0x00,
	MCU_NETWORKMODE_APCONFIG = 0x01,
	MCU_NETWORKMODE_NOTCONNECTED = 0x02,
	MCU_NETWORKMODE_CONNECTED = 0x03,
	MCU_NETWORKMODE_CONNECTEDCLOUD = 0x04,
	MCU_NETWORKMODE_POWERSAVE = 0x05
} mcuNetworkMode_t;

class WBecaDevice: public WDevice {
public:
    typedef std::function<bool()> THandlerFunction;
    typedef std::function<bool(const char*)> TCommandHandlerFunction;

    WBecaDevice(WNetwork* network, WClock* wClock)
    	: WDevice(network, "thermostat", "thermostat", network->getIdx(), DEVICE_TYPE_THERMOSTAT) {
		
    	this->receivingDataFromMcu = false;
    	this->schedulesChanged = false;
		this->providingConfigPage = true;
		this->currentSchedulePeriod = -1;
		this->targetTemperatureManualMode = 0.0;
    	this->wClock = wClock;
    	this->systemMode = nullptr;
		this->setMqttRetain(true);
		this->stateNotifyInterval=60000;
		this->onConfigurationRequest=nullptr;
		this->onPowerButtonOn=nullptr;
		this->mqttHassAutodiscoverSent=false;
		startMcuInitialize();
		/* properties */

		if (network->getSettingsOld()){
			if (network->getSettingsOld()->getNetworkSettingsVersion()==NETWORKSETTINGS_PRE_FAS114){
				network->log()->notice(F("Reading WLogSettings PRE_FAS114"));
				network->getSettingsOld()->setByte("becabits1", 0x00);
				network->getSettingsOld()->setByte("becabits2", 0x00);
				network->getSettingsOld()->setByte("thermostatModel", MODEL_BHT_002_GBLW);
				network->getSettingsOld()->setByte("schedulesDayOffset", 0);
			}
		}

		// read beca bits 
		this->becaBits1 = network->getSettings()->setByte("becabits1",
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting("becabits1") ? network->getSettingsOld()->getByte("becabits1") : 0x00));
		this->becaBits2 = network->getSettings()->setByte("becabits2",
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting("becabits2") ? network->getSettingsOld()->getByte("becabits2") : 0x00));
		// Split mqtt setting into bits - so we keep settings storage compatibility
		if (this->becaBits1->getByte() == 0xFF) this->becaBits1->setByte(BECABITS1_RELAIS_HEAT); // compatibility

		// Heating Relay and State property
		this->supportingHeatingRelay = new WProperty("supportingHeatingRelay", "supportingHeatingRelay", BOOLEAN);
		this->supportingHeatingRelay->setBoolean(this->becaBits1->getByte() & BECABITS1_RELAIS_HEAT);
		this->supportingCoolingRelay = new WProperty("supportingCoolingRelay", "supportingCoolingRelay", BOOLEAN);
		this->supportingCoolingRelay->setBoolean(this->becaBits1->getByte() & BECABITS1_RELAIS_COOL);

		if (this->supportingHeatingRelay->getBoolean() && this->supportingCoolingRelay->getBoolean()) {
			this->supportingCoolingRelay->setBoolean(false);
		}

		// switch back property
		this->switchBackToAuto = new WProperty("switchBackToAuto", "switch Back from Manual to Auto at next Schedule", BOOLEAN);
		this->switchBackToAuto->setBoolean(!(this->becaBits1->getByte() & BECABITS1_SWITCHBACKOFF));
		this->switchBackToAuto->setVisibility(ALL);
		this->switchBackToAuto->setReadOnly(false);
		this->switchBackToAuto->setMqttSendChangedValues(true);
		this->switchBackToAuto->setOnChange(std::bind(&WBecaDevice::saveSettings, this, std::placeholders::_1));
		this->addProperty(switchBackToAuto);

		// Floor Sensor 
		this->floorSensor = new WProperty("floorSensor", "floorSensor", BOOLEAN);
		this->floorSensor->setBoolean(this->becaBits1->getByte() & BECABITS1_FLOORSENSOR);

		// precicion (must be initialized before Temperature Values)
		this->temperaturePrecision = new WProperty("precision", "Precision", DOUBLE);
		if (this->becaBits1->getByte() & BECABITS1_TEMP_01){
			this->temperaturePrecision->setDouble(0.1f);
		} else if (this->becaBits1->getByte() & BECABITS1_TEMP_10){
			this->temperaturePrecision->setDouble(1.0f);
		} else {
			this->temperaturePrecision->setDouble(0.5f);
		}
		this->temperaturePrecision->setReadOnly(true);
		this->temperaturePrecision->setVisibility(ALL);


    	this->actualTemperature = new WTemperatureProperty("temperature", "Actual");
    	this->actualTemperature->setReadOnly(true);
		this->actualTemperature->setMqttSendChangedValues(true);
    	this->addProperty(actualTemperature);
    	this->targetTemperature = new WTargetTemperatureProperty("targetTemperature", "Target");//, 12.0, 28.0);
    	this->targetTemperature->setMultipleOf(getTemperaturePrecision());
    	this->targetTemperature->setOnChange(std::bind(&WBecaDevice::setTargetTemperature, this, std::placeholders::_1));
    	this->targetTemperature->setOnValueRequest([this](WProperty* p) {updateTargetTemperature();});
		this->targetTemperature->setMqttSendChangedValues(true);
    	this->addProperty(targetTemperature);
    	this->deviceOn = new WOnOffProperty("deviceOn", "Power");
    	this->deviceOn->setOnChange(std::bind(&WBecaDevice::deviceOnToMcu, this, std::placeholders::_1));
		this->deviceOn->setMqttSendChangedValues(true);
    	this->addProperty(deviceOn);
    	this->schedulesMode = new WProperty("schedulesMode", "Schedules", STRING);
    	this->schedulesMode->setAtType("ThermostatSchedulesModeProperty");
    	this->schedulesMode->addEnumString(SCHEDULES_MODE_OFF);
    	this->schedulesMode->addEnumString(SCHEDULES_MODE_AUTO);
    	this->schedulesMode->setOnChange(std::bind(&WBecaDevice::schedulesModeToMcu, this, std::placeholders::_1));
		this->schedulesMode->setMqttSendChangedValues(true);
    	this->addProperty(schedulesMode);
    	this->ecoMode = new WOnOffProperty("ecoMode", "Eco");
    	this->ecoMode->setOnChange(std::bind(&WBecaDevice::ecoModeToMcu, this, std::placeholders::_1));
    	this->ecoMode->setVisibility(MQTT);
		this->ecoMode->setMqttSendChangedValues(true);
    	this->addProperty(ecoMode);
    	this->locked = new WOnOffProperty("locked", "Lock");
    	this->locked->setOnChange(std::bind(&WBecaDevice::lockedToMcu, this, std::placeholders::_1));
    	this->locked->setVisibility(MQTT);
		this->locked->setMqttSendChangedValues(true);
    	this->addProperty(locked);
    	//Model
    	this->actualFloorTemperature = nullptr;
    	this->thermostatModel = network->getSettings()->setByte("thermostatModel",
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting("thermostatModel") ? network->getSettingsOld()->getByte("thermostatModel") : MODEL_BHT_002_GBLW));
    	if (getThermostatModel() == MODEL_BHT_002_GBLW) {
			if (this->floorSensor->getBoolean()){
				this->actualFloorTemperature = new WTemperatureProperty("floorTemperature", "Floor");
				this->actualFloorTemperature->setReadOnly(true);
				this->actualFloorTemperature->setVisibility(ALL);
				this->actualFloorTemperature->setMqttSendChangedValues(true);
				this->addProperty(actualFloorTemperature);
			}
    	} else if (getThermostatModel() == MODEL_BAC_002_ALW) {
    		this->systemMode = new WProperty("systemMode", "System Mode", STRING);
        	this->systemMode->setAtType("ThermostatModeProperty");
        	this->systemMode->addEnumString(SYSTEM_MODE_NONE);
        	this->systemMode->addEnumString(SYSTEM_MODE_COOL);
        	this->systemMode->addEnumString(SYSTEM_MODE_HEAT);
        	this->systemMode->addEnumString(SYSTEM_MODE_FAN);
			this->systemMode->setOnChange(std::bind(&WBecaDevice::systemModeToMcu, this, std::placeholders::_1));
			this->systemMode->setMqttSendChangedValues(true);
        	this->addProperty(systemMode);
    		this->fanMode = new WProperty("fanMode", "Fan", STRING);
        	this->fanMode->setAtType("FanModeProperty");
        	this->fanMode->addEnumString(FAN_MODE_NONE);
        	this->fanMode->addEnumString(FAN_MODE_LOW);
        	this->fanMode->addEnumString(FAN_MODE_MEDIUM);
        	this->fanMode->addEnumString(FAN_MODE_HIGH);
			this->fanMode->setOnChange(std::bind(&WBecaDevice::fanModeToMcu, this, std::placeholders::_1));
			this->fanMode->setMqttSendChangedValues(true);
        	this->addProperty(fanMode);
    	}
		/* 
		* New OverAllMode for easier integration
		* https://iot.mozilla.org/schemas/#ThermostatModeProperty
		* https://www.home-assistant.io/integrations/climate.mqtt/
		*/

    	this->mode = new WProperty("mode", "Mode", STRING);
    	this->mode->setAtType("ThermostatModeProperty"); 
    	this->mode->addEnumString(MODE_OFF);
		if (getThermostatModel() == MODEL_BHT_002_GBLW) {
    		this->mode->addEnumString(MODE_AUTO);
		}
		if (getThermostatModel() == MODEL_BAC_002_ALW) {
			this->mode->addEnumString(MODE_AUTOHEAT);
			this->mode->addEnumString(MODE_AUTOCOOL);
			this->mode->addEnumString(MODE_AUTOFAN);
			this->mode->addEnumString(MODE_COOL);
    		this->mode->addEnumString(MODE_FAN);
		}
		this->mode->addEnumString(MODE_HEAT);
    	this->mode->setOnChange(std::bind(&WBecaDevice::modeToMcu, this, std::placeholders::_1));
		this->mode->setOnValueRequest([this](WProperty* p) {updateModeAndAction();});
		this->mode->setMqttSendChangedValues(true);
    	this->addProperty(mode);

		this->action = new WProperty("action", "Action", STRING);
		this->action->setAtType("ThermostatactionProperty"); 
		this->action->addEnumString(ACTION_OFF);
		this->action->addEnumString(ACTION_HEATING);
		if (isSupportingHeatingRelay() || isSupportingCoolingRelay()) this->action->addEnumString(ACTION_IDLE);
		if (getThermostatModel() == MODEL_BHT_002_GBLW) {
		}
		if (getThermostatModel() == MODEL_BAC_002_ALW) {
			this->action->addEnumString(ACTION_COOLING);
			this->action->addEnumString(ACTION_FAN);
		}
		this->action->setMqttSendChangedValues(true);
		this->action->setReadOnly(true);
		this->addProperty(action);


		if (getThermostatModel() == MODEL_BHT_002_GBLW) {
			//disable Cooling Relay if enabled on heating-thermostat
			this->supportingCoolingRelay->setBoolean(false);
		}
		if (isSupportingHeatingRelay()) pinMode(PIN_STATE_HEATING_RELAY, INPUT);
    	else if (isSupportingCoolingRelay()) pinMode(PIN_STATE_COOLING_RELAY, INPUT);
		this->state = nullptr;
    	if ((isSupportingHeatingRelay()) || (isSupportingCoolingRelay())) {
    		this->state = new WProperty("state", "State", STRING);
    		this->state->setAtType("HeatingCoolingProperty");
    		this->state->setReadOnly(true);
    		this->state->addEnumString(STATE_OFF);
    		if (isSupportingHeatingRelay()) this->state->addEnumString(STATE_HEATING);
    		if (isSupportingCoolingRelay()) this->state->addEnumString(STATE_COOLING);
			this->state->setMqttSendChangedValues(true);
			this->state->setOnValueRequest([this](WProperty* p) {updateModeAndAction();});
    		this->addProperty(state);
    	}

    	//schedulesDayOffset
    	this->schedulesDayOffset = network->getSettings()->setByte("schedulesDayOffset",
			(network->getSettingsOld() && network->getSettingsOld()->existsSetting("schedulesDayOffset") ? network->getSettingsOld()->getByte("schedulesDayOffset") : 0));


		this->mcuId = new WProperty("mcuId", "mcuId", STRING);
		this->mcuId->setReadOnly(true);
		this->addProperty(mcuId);
		// Pages		
		WPage * schedulePage=new WPage("schedules", "Configure Schedules");
		schedulePage->setPrintPage([this,schedulePage](ESP8266WebServer* webServer, WStringStream* page) {
			this->network->log()->notice(PSTR("Schedules"));
			page->printAndReplace(FPSTR(HTTP_CONFIG_PAGE_BEGIN), ((String)getId()+"_"+schedulePage->getId()).c_str());
			page->print(FPSTR(HTTP_CONFIG_SCHTAB_HEAD));
			for (char *period=(char*)SCHEDULES_PERIODS; *period > 0; period++){
				page->printf(F("<tr>"));
				page->printf(F("<td>Period %c</td>"), *period);
				for (char *day=(char*)SCHEDULES_DAYS; *day > 0; day++){
					char keyH[4];
					char keyT[4];
					snprintf(keyH, 4, "%c%ch", *day, *period);
					snprintf(keyT, 4, "%c%ct", *day, *period);
					this->network->log()->verbose(PSTR("Period %s / %s"), keyH, keyT);
					page->printf(HTTP_CONFIG_SCHTAB_TD, 
					*day, *period, this->getSchedulesValue(keyH).c_str(),
					*day, *period, this->getSchedulesValue(keyT).c_str());
				}
				page->printf(F("</tr>"));
			}
			page->print(FPSTR(HTTP_CONFIG_SCHTAB_FOOT));
			page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));

		});
		schedulePage->setSubmittedPage([this](ESP8266WebServer* webServer, WStringStream* page) {
			this->network->log()->notice(PSTR("submitted"));
			schedulesChanged = false;
			for (char *period=(char*)SCHEDULES_PERIODS; *period > 0; period++){
				for (char *day=(char*)SCHEDULES_DAYS; *day > 0; day++){
					char keyH[4];
					char keyT[4];
					snprintf(keyH, 4, "%c%ch", *day, *period);
					snprintf(keyT, 4, "%c%ct", *day, *period);
					const char * valueH = webServer->arg(keyH).c_str();
					const char * valueT = webServer->arg(keyT).c_str();
					processSchedulesKeyValue(keyH, valueH);
					processSchedulesKeyValue(keyT, valueT);
				}
			}
			if (schedulesChanged) {
				this->network->log()->notice(PSTR("Some schedules changed. Write to MCU..."));
				this->schedulesToMcu();
				page->print(F("Schedules have been saved"));
			} else {
				page->print(F("Nothing has been changed"));
			}
			
		});
		this->addPage(schedulePage);
		

		// Pages reinit		
		WPage * reinitPage=new WPage("reinit", "Reinit Thermostat");
		reinitPage->setPrintPage([this,reinitPage](ESP8266WebServer* webServer, WStringStream* page) {
			this->network->log()->notice(PSTR("Reinit"));
			page->print(F("Reinitialized"));
			page->print(FPSTR(HTTP_HOME_BUTTON));
			startMcuInitialize();
		});
		reinitPage->setSubmittedPage([this](ESP8266WebServer* webServer, WStringStream* page) {
			page->print(F("VOID"));			
		});
		this->addPage(reinitPage);
		

		lastHeartBeat = lastNotify = lastScheduleNotify  = lastLongLoop = 0;
		resetAll();
		for (int i = 0; i < STATE_COMPLETE; i++) {
			receivedStates[i] = false;
		}
		this->schedulesDataPoint = 0x00;
	}

    virtual void printConfigPage(WStringStream* page) {
    	network->log()->notice(PSTR("Beca thermostat config page"));
    	page->printAndReplace(FPSTR(HTTP_CONFIG_PAGE_BEGIN), getId());

    	//ComboBox with model selection
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Thermostat model:", "tm");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (getThermostatModel() == 0 ? HTTP_SELECTED : ""), "Floor heating (BHT-002-GxLW)");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (getThermostatModel() == 1 ? HTTP_SELECTED : ""), "Heating, Cooling, Ventilation (BAC-002-ALW)");
    	page->print(FPSTR(HTTP_COMBOBOX_END));

		// Temp precision
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Temperature Precision (must match your hardware):", "tp");
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "05", (getTemperatureFactor() ==  2.0f ? HTTP_SELECTED : ""), "0.5 (default for most Devices)");
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "10", (getTemperatureFactor() ==  1.0f ? HTTP_SELECTED : ""), "1.0 (untested)");
		//page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "01", (getTemperatureFactor() == 10.0f ? HTTP_SELECTED : ""), "0.1");
		page->print(FPSTR(HTTP_COMBOBOX_END));

		//Checkbox with support for relay
		int rsMode=(this->isSupportingHeatingRelay() ? 1 :  (this->isSupportingCoolingRelay() ? 2 : 0));
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Relais connected to GPIO Inputs:", "rs");
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "_", (rsMode == 0 ? HTTP_SELECTED : ""), "No Hardware Hack");
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "h", (rsMode == 1 ? HTTP_SELECTED : ""), "Heating-Relay at GPIO 5");
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "c", (rsMode == 2 ? HTTP_SELECTED : ""), "Cooling-Relay at GPIO 5");
		page->print(FPSTR(HTTP_COMBOBOX_END));

		// FloorSensor 
		page->printAndReplace(FPSTR(HTTP_CHECKBOX_OPTION), "Floor Sensor enabled",
		"fs", "fs", (this->floorSensor->getBoolean() ? HTTP_CHECKED : ""), "", "Enabled");

		// Switch back from manual temo
		page->printAndReplace(FPSTR(HTTP_CHECKBOX_OPTION), "Switch back to Auto mode from manual at next schedule period change",
		"sb", "sb", (this->switchBackToAuto->getBoolean() ? HTTP_CHECKED : ""), "", "Enabled");

		//ComboBox with weekday
    	byte dayOffset = getSchedulesDayOffset();
		page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Workday schedules:", "ws");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (dayOffset == 0 ? HTTP_SELECTED : ""), "Workday (1-5): Mon-Fri; Weekend (6 - 7): Sat-Sun");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (dayOffset == 1 ? HTTP_SELECTED : ""), "Workday (1-5): Sun-Thu; Weekend (6 - 7): Fri-Sat");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "2", (dayOffset == 2 ? HTTP_SELECTED : ""), "Workday (1-5): Sat-Wed; Weekend (6 - 7): Thu-Fri");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "3", (dayOffset == 3 ? HTTP_SELECTED : ""), "Workday (1-5): Fri-Tue; Weekend (6 - 7): Wed-Thu");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "4", (dayOffset == 4 ? HTTP_SELECTED : ""), "Workday (1-5): Thu-Mon; Weekend (6 - 7): Tue-Wed");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "5", (dayOffset == 5 ? HTTP_SELECTED : ""), "Workday (1-5): Wed-Sun; Weekend (6 - 7): Mon-Tue");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "6", (dayOffset == 6 ? HTTP_SELECTED : ""), "Workday (1-5): Tue-Sat; Weekend (6 - 7): Sun-Mon");
    	page->print(FPSTR(HTTP_COMBOBOX_END));

		page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
		network->log()->notice(PSTR("Beca thermostat config page DONE"));
		return;
    }

    void saveConfigPage(ESP8266WebServer* webServer) {
        network->log()->notice(PSTR("Save Beca config page"));
        this->thermostatModel->setByte(webServer->arg("tm").toInt());
        this->schedulesDayOffset->setByte(webServer->arg("ws").toInt());
		byte bb1 = 0;
		byte bb2 = 0;
		if (webServer->arg("rs") == "h"){
			bb1 |= BECABITS1_RELAIS_HEAT;
		} else if (webServer->arg("rs") == "c"){
			bb1 |= BECABITS1_RELAIS_COOL;
		} else {
			// default no relaus
		}
		if (webServer->arg("tp") == "10"){
			bb1 |= BECABITS1_TEMP_10;
		} else if (webServer->arg("tp") == "01"){
			bb1 |= BECABITS1_TEMP_01;
		} else {
			// default 0.5
		}
		bb1 |= ((webServer->arg("sb") == HTTP_TRUE) ? 0 : BECABITS1_SWITCHBACKOFF); //logic reversed!
		bb1 |= ((webServer->arg("fs") == HTTP_TRUE) ? BECABITS1_FLOORSENSOR : 0);
		this->becaBits1->setByte(bb1);
		this->becaBits2->setByte(bb2); // meets r2d2
    }

    void loop(unsigned long now) {
    	if (state != nullptr) {
    		bool heating = false;
    		bool cooling = false;
    		if ((isSupportingHeatingRelay()) && (state != nullptr)) {
    			heating = digitalRead(PIN_STATE_HEATING_RELAY);
    		}
    		if ((isSupportingCoolingRelay()) && (state != nullptr)) {
    			cooling = digitalRead(PIN_STATE_COOLING_RELAY);
    		}
    		this->state->setString(heating ? STATE_HEATING : (cooling ? STATE_COOLING : STATE_OFF));
    	}
    	while (Serial.available() > 0) {
    		receiveIndex++;
    		unsigned char inChar = Serial.read();
			//logCommand(((String)"Serial Read "+String(inChar, HEX)).c_str());
    		receivedCommand[receiveIndex] = inChar;
    		if (receiveIndex < 2) {
    			//Check command start
    			if (COMMAND_START[receiveIndex] != receivedCommand[receiveIndex]) {
    				resetAll();
    			}
    		} else if (receiveIndex == 5) {
    			//length information now available
    			commandLength = receivedCommand[4] * 0x100 + receivedCommand[5];
    		} else if ((commandLength > -1)
    				&& (receiveIndex == (6 + commandLength))) {
    			//verify checksum
    			int expChecksum = 0;
    			for (int i = 0; i < receiveIndex; i++) {
    				expChecksum += receivedCommand[i];
    			}
    			expChecksum = expChecksum % 0x100;
    			if (expChecksum == receivedCommand[receiveIndex]) {
					logIncomingCommand("processSerial", LOG_LEVEL_VERBOSE);
					processSerialCommand();
    			}
    			resetAll();


    		}
    	}
		// to tasks only every second
		if (lastLongLoop + 1000 < now ){
			updateCurrentSchedulePeriod();
			lastLongLoop=now;
		}

    	//Heartbeat
    	//long now = millis();
    	if ((HEARTBEAT_INTERVAL > 0)
    			&& ((lastHeartBeat == 0)
    					|| (now - lastHeartBeat > HEARTBEAT_INTERVAL))) {
    		unsigned char heartBeatCommand[] =
    				{ 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00 };
			network->log()->trace(F("sending heartBeatCommand"));
    		commandCharsToSerial(6, heartBeatCommand);
    		//commandHexStrToSerial("55 aa 00 00 00 00");
    		lastHeartBeat = now;
    	}
    	if (receivedSchedules()) {
    		//Notify schedules
    		if ((lastScheduleNotify == 0) && (now - lastScheduleNotify > MINIMUM_INTERVAL)) {
    			sendSchedulesToMqtt();
    			lastScheduleNotify = now;
    		}
    	}
		if (!isMcuInitialized()){
			mcuInitialize();
		}
    }

    unsigned char* getCommand() {
    	return receivedCommand;
    }

    int getCommandLength() {
    	return commandLength;
    }

    String getCommandAsString(int commandLength, unsigned char * command ) {
    	String result = "";
    	if (commandLength > -1) {
    		for (int i = 0; i <  commandLength; i++) {
    			unsigned char ch = command[i];
    			result = result + (ch < 16 ? "0" : "") + String(ch, HEX);// charToHexStr(ch);
    			if (i + 1 < commandLength) {
    				result = result + " ";
    			}
    		}
    	}
    	return result;
    }

	 String getIncomingCommandAsString() {
    	return getCommandAsString(commandLength + 6, receivedCommand);
    }

    void commandHexStrToSerial(String command) {
    	command.trim();
    	command.replace(" ", "");
    	command.toLowerCase();
		unsigned char cmd[command.length()/2];
    	if ((command.length() > 1) && (command.length() % 2 == 0)) {
			int i;
    		for (i = 0; i < (command.length() / 2); i++) {
    			unsigned char chValue = getIndex(command.charAt(i * 2)) * 0x10
    					+ getIndex(command.charAt(i * 2 + 1));
    			cmd[i]=(unsigned char)chValue;
    		}
			commandCharsToSerial(i, cmd);
    	}
    }

    void commandCharsToSerial(unsigned int length, unsigned char* command) {
    	int chkSum = 0;
    	if (length > 2) {
    		for (int i = 0; i < length; i++) {
    			unsigned char chValue = command[i];
    			chkSum += chValue;
    			Serial.print((char) chValue);
    		}
    		unsigned char chValue = chkSum % 0x100;
    		Serial.print((char) chValue);
			network->log()->trace(F("commandCharsToSerial: %s, ChckSum 0x%02hhx" ),
			getCommandAsString(length, command).c_str(), chValue);
    	}
    }

    void queryAllDPs() {
    	//55 AA 00 08 00 00
    	unsigned char queryStateCommand[] = { 0x55, 0xAA, 0x00, 0x08, 0x00, 0x00 };
    	commandCharsToSerial(6, queryStateCommand);
    }

    void cancelConfiguration() {
    	reportNetworkToMcu(mcuNetworkMode::MCU_NETWORKMODE_NOTCONNECTED);
    }
	void reportNetworkToMcu(mcuNetworkMode state) {
		network->log()->trace(F("sending networkMode to Mcu: %d"), state);
		unsigned char mcuCommand[] = { 0x55, 0xaa, 0x00, 0x03, 0x00, 0x01,
				(unsigned char)state };
		commandCharsToSerial(7, mcuCommand);
		//  unsigned char configCommand[] = { 0x55, 0xAA, 0x00, 0x03, 0x00,
		//		0x01, 0x00 };
	}

    void sendActualTimeToBeca(bool localtime) {
    	//Command: Set date and time
    	//                       OK YY MM DD HH MM SS Weekday
    	//DEC:                   01 19 02 15 16 04 18 05
    	//HEX: 55 AA 00 1C 00 08 01 13 02 0F 10 04 12 05
    	//DEC:                   01 19 02 20 17 51 44 03
    	//HEX: 55 AA 00 1C 00 08 01 13 02 14 11 33 2C 03
    	unsigned long epochTime = (localtime ? wClock->getEpochTimeLocal() : wClock->getEpochTime());
    	epochTime = epochTime + (getSchedulesDayOffset() * 86400);
    	byte year = wClock->getYear(epochTime) % 100;
    	byte month = wClock->getMonth(epochTime);
    	byte dayOfMonth = wClock->getDay(epochTime);
    	byte hours = wClock->getHours(epochTime) ;
    	byte minutes = wClock->getMinutes(epochTime);
    	byte seconds = wClock->getSeconds(epochTime);
    	byte dayOfWeek = getDayOfWeek();


		network->log()->trace(F("sendActual%sTimeToBeca %d + %d days: %02d.%02d.%02d %02d:%02d:%02d (dow: %d)" ),
		(localtime ? "Local" : "GMT"), epochTime, getSchedulesDayOffset(), year, month, dayOfMonth, hours, minutes, seconds, dayOfWeek );
    	unsigned char cancelConfigCommand[] = { 0x55, 0xaa, 0x00, (localtime ?  (char)0x1c : (char)0x0c), 0x00, 0x08,
    											0x01, year, month, dayOfMonth,
    											hours, minutes, seconds, dayOfWeek};
    	commandCharsToSerial(14, cancelConfigCommand);
    }

    void bindWebServerCalls(ESP8266WebServer* webServer) {
    	String deviceBase("/things/");
    	deviceBase.concat(getId());
    	deviceBase.concat("/");
    	deviceBase.concat(SCHEDULES);
    	webServer->on(deviceBase.c_str(), HTTP_GET, std::bind(&WBecaDevice::sendSchedules, this, webServer));
    }

    void handleUnknownMqttCallback(String stat_topic, String partialTopic, String payload, unsigned int length) {
		network->log()->notice(F("handleUnknownMqttCallback %s|%s|%s" ), stat_topic.c_str(), partialTopic.c_str(), payload.c_str());
		// {"log":"handleUnknownMqttCallback home/test/stat/things/thermostat/properties / schedules / "}
    	if (partialTopic.startsWith(SCHEDULES)) {
    		partialTopic = partialTopic.substring(SCHEDULES.length() + 1);
    		if (partialTopic.equals("")) {
				if (length == 0) {
					//Send actual schedules
					network->log()->notice(PSTR("Empty payload for schedules -> send schedules..."));
				} else {
					//Set schedules
					network->log()->notice(PSTR("Payload for schedules -> set and send schedules..."));
					WJsonParser* parser = new WJsonParser();
					schedulesChanged = false;
					parser->parse(payload.c_str(), std::bind(&WBecaDevice::processSchedulesKeyValue, this,
									std::placeholders::_1, std::placeholders::_2));
					delete parser;
					if (schedulesChanged) {
						network->log()->notice(PSTR("Some schedules changed. Write to MCU..."));
						this->schedulesToMcu();
					} else {
						network->log()->notice(PSTR("No schedules changed."));
					}
				}
				sendSchedulesToMqtt();
    		} else {
    			//There are still some more topics after properties
    			network->log()->warning(PSTR("Longer topic for schedules -> not supported yet..."));

    		}
    	} else if (partialTopic.equals(MCUCOMMAND)) {
			//send to MCU
			network->log()->notice(PSTR("Received %s: %s"), MCUCOMMAND.c_str(), payload.c_str());
			commandHexStrToSerial(payload);
		} else if (partialTopic.equals("wifiap")) {
			network->log()->notice(PSTR("setDesiredModeAp"));
			network->setDesiredModeAp();
		} else if (partialTopic.equals("wififallback")) {
			network->log()->notice(PSTR("setDesiredModeAp"));
			network->setDesiredModeFallback();
		} else if (partialTopic.equals("wfistation")) {
			network->log()->notice(PSTR("setDesiredModeStation"));
			network->setDesiredModeStation();
		}
    }

	void sendSchedulesToMqtt(){
		WStringStream* response = network->getResponseStream();
		WJson json(response);
		json.beginObject();
		this->toJsonSchedules(&json, 0);// SCHEDULE_WORKDAY);
		this->toJsonSchedules(&json, 1);// SCHEDULE_SATURDAY);
		this->toJsonSchedules(&json, 2);// SCHEDULE_SUNDAY);
		json.endObject();
		String stat_topic = network->getMqttTopic() + (String)"/" + String(MQTT_STAT) + (String)"/things/" + String(getId()) + (String)"/";
		network->publishMqtt((stat_topic+SCHEDULES).c_str(), response);
	}


	int getSchedulesPeriod(const char* key) {
		if (strlen(key) == 3) {
			for (int i = 0; i < 6; i++) {
				if (SCHEDULES_PERIODS[i] == key[1]) {
					return i;
					break;
				}
			}
		} else return -1;
	}

	int getSchedulesStartAddress(const char* key, byte period) {
		if (strlen(key) == 3) {
			byte startAddr = 255;
			if (key[0] == SCHEDULES_DAYS[0]) {
				return 0;
			} else if (key[0] == SCHEDULES_DAYS[1]) {
				return 18;
			} else if (key[0] == SCHEDULES_DAYS[2]) {
				return 36;
			}
		} else return -1;
	}

	String getSchedulesValue(const char* key) {
		char buf[8];
		size_t size = sizeof buf;
		buf[0]=0;
		byte period;
		byte startAddr;
		if ((period = getSchedulesPeriod(key))<0) return String(buf);
		if ((startAddr = getSchedulesStartAddress(key, period))<0) return String(buf);
		if (key[2] == 'h') {
			snprintf(buf, size-1, "%02d:%02d", (int)schedules[startAddr + period * 3 + 1], (int)schedules[startAddr + period * 3 + 0] );
		} else if (key[2] == 't') {
			//temperature
			char str_temp[6];
			double val = (double)(schedules[startAddr + period * 3 + 2]) / 2.0f;
			dtostrf(val, 4, 1, str_temp);
			snprintf(buf, size-1, "%s", str_temp );
		}
		network->log()->verbose(PSTR("getSch %s->%s"), key, buf);
		return String(buf);
	}
    void processSchedulesKeyValue(const char* key, const char* value) {
		network->log()->verbose(PSTR("Process key '%s', value '%s'"), key, value);
		byte period;
		byte startAddr;
		if ((period = getSchedulesPeriod(key))<0) return;
		if ((startAddr = getSchedulesStartAddress(key, period))<0) return;
		network->log()->verbose(PSTR("Process period: %d, startAddr: %d"), period, startAddr);
		if (key[2] == 'h') {
			//hour
			String timeStr = String(value);
			timeStr = (timeStr.length() == 4 ? "0" + timeStr : timeStr);
			byte hh = timeStr.substring(0, 2).toInt();
			byte mm = timeStr.substring(3, 5).toInt();
			schedulesChanged = schedulesChanged || (schedules[startAddr + period * 3 + 1] != hh);
			schedules[startAddr + period * 3 + 1] = hh;
			schedulesChanged = schedulesChanged || (schedules[startAddr + period * 3 + 0] != mm);
			schedules[startAddr + period * 3 + 0] = mm;
		} else if (key[2] == 't') {
			//temperature
			byte tt = (int) (atof(value) * getTemperatureFactor());
			schedulesChanged = schedulesChanged || (schedules[startAddr + period * 3 + 2] != tt);
			schedules[startAddr + period * 3 + 2] = tt;
		}
    }

    void sendSchedules(ESP8266WebServer* webServer) {
    	WStringStream* response = network->getResponseStream();
    	WJson json(response);
    	json.beginObject();
    	this->toJsonSchedules(&json, 0);// SCHEDULE_WORKDAY);
    	this->toJsonSchedules(&json, 1);// SCHEDULE_SATURDAY);
    	this->toJsonSchedules(&json, 2);// SCHEDULE_SUNDAY);
    	json.endObject();
    	webServer->send(200, APPLICATION_JSON, response->c_str());
    }

    virtual void toJsonSchedules(WJson* json, byte schedulesDay) {
    	byte startAddr = 0;
		char dayChar = SCHEDULES_DAYS[0];
		switch (schedulesDay) {
		case 1 :
			startAddr = 18;
			dayChar = SCHEDULES_DAYS[1];
			break;
		case 2 :
			startAddr = 36;
			dayChar = SCHEDULES_DAYS[2];
			break;
		}
    	char timeStr[6];
    	timeStr[5] = '\0';
    	char* buffer = new char[4];
    	buffer[0] = dayChar;
    	buffer[3] = '\0';
    	for (int i = 0; i < 6; i++) {
    		buffer[1] = SCHEDULES_PERIODS[i];
    		sprintf(timeStr, "%02d:%02d", schedules[startAddr + i * 3 + 1], schedules[startAddr + i * 3 + 0]);
    		buffer[2] = 'h';
    		json->propertyString(buffer, timeStr);
    		buffer[2] = 't';
    		json->propertyDouble(buffer, (double) schedules[startAddr + i * 3 + 2]	/ getTemperatureFactor());
    	}
    	delete[] buffer;
    }

	void setOnConfigurationRequest(THandlerFunction onConfigurationRequest) {
		this->onConfigurationRequest = onConfigurationRequest;
	}
	
	void setOnPowerButtonOn(THandlerFunction onPowerButtonOn) {
		this->onPowerButtonOn = onPowerButtonOn;
	}

    void schedulesToMcu() {
    	if (receivedSchedules()) {
    		//Changed schedules from MQTT server, send to mcu
    		//send the changed array to MCU
    		//per unit |MM HH TT|
    		//55 AA 00 06 00 3A 65 00 00 36|
    		//00 06 28|00 08 1E|1E 0B 1E|1E 0D 1E|00 11 2C|00 16 1E|
    		//00 06 28|00 08 28|1E 0B 28|1E 0D 28|00 11 28|00 16 1E|
    		//00 06 28|00 08 28|1E 0B 28|1E 0D 28|00 11 28|00 16 1E|
    		unsigned char scheduleCommand[64];
    		scheduleCommand[0] = 0x55;
    		scheduleCommand[1] = 0xaa;
    		scheduleCommand[2] = 0x00;
    		scheduleCommand[3] = 0x06;
    		scheduleCommand[4] = 0x00;
    		scheduleCommand[5] = 0x3a;
    		scheduleCommand[6] = schedulesDataPoint;
    		scheduleCommand[7] = 0x00;
    		scheduleCommand[8] = 0x00;
    		scheduleCommand[9] = 0x36;
    		for (int i = 0; i < 54; i++) {
    			scheduleCommand[i + 10] = schedules[i];
    		}
    		commandCharsToSerial(64, scheduleCommand);
    		//notify change
    		this->notifySchedules();
    	}
    }

    String getFanMode() {
        return (fanMode != nullptr ? fanMode->c_str() : FAN_MODE_NONE);
    }

    byte getFanModeAsByte() {
    	if (fanMode != nullptr) {
    	   	if (fanMode->equalsString(FAN_MODE_AUTO)) {
    	   		return 0x00;
    	   	} else if (fanMode->equalsString(FAN_MODE_HIGH)) {
    	   		return 0x01;
    	   	} else if (fanMode->equalsString(FAN_MODE_MEDIUM)) {
    	   		return 0x02;
    	   	} else if (fanMode->equalsString(FAN_MODE_LOW)) {
    	   		return 0x03;
    	   	} else {
    	   		return 0xFF;
    	   	}
    	} else {
    		return 0xFF;
    	}
    }

    void fanModeToMcu(WProperty* property) {
    	if ((fanMode != nullptr) && (!this->receivingDataFromMcu)) {
			network->log()->notice(F("fanModeToMcu"));
    		byte dt = this->getFanModeAsByte();
    		if (dt != 0xFF) {
    			//send to device
    		    //auto:   55 aa 00 06 00 05 67 04 00 01 00
    			//low:    55 aa 00 06 00 05 67 04 00 01 03
    			//medium: 55 aa 00 06 00 05 67 04 00 01 02
    			//high:   55 aa 00 06 00 05 67 04 00 01 01
    			unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
    			                                    0x67, 0x04, 0x00, 0x01, dt};
    			commandCharsToSerial(11, deviceOnCommand);
    		}
    	}
    }

    String getSystemMode() {
    	return (systemMode != nullptr ? systemMode->c_str() : SYSTEM_MODE_NONE);
    }

    byte getSystemModeAsByte() {
    	if (systemMode != nullptr) {
    		if (systemMode->equalsString(SYSTEM_MODE_COOL)) {
    			return 0x00;
    		} else if (systemMode->equalsString(SYSTEM_MODE_HEAT)) {
    			return 0x01;
    		} else if (systemMode->equalsString(SYSTEM_MODE_FAN)) {
    			return 0x02;
    		} else {
    			return 0xFF;
    		}
    	} else {
    		return 0xFF;
    	}
    }

    void systemModeToMcu(WProperty* property) {
    	if ((systemMode != nullptr) && (!this->receivingDataFromMcu)) {
			network->log()->notice(F("systemModeToMcu"));
    		byte dt = this->getSystemModeAsByte();
    		if (dt != 0xFF) {
    			//send to device
    			//cooling:     55 AA 00 06 00 05 66 04 00 01 00
    			//heating:     55 AA 00 06 00 05 66 04 00 01 01
    			//ventilation: 55 AA 00 06 00 05 66 04 00 01 02
    			unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
    												0x66, 0x04, 0x00, 0x01, dt};
    			commandCharsToSerial(11, deviceOnCommand);
    		}
    	}
    }

	bool isMcuInitialized(){
		return this->mcuInitialized;
	}


	void startMcuInitialize(){
		this->mcuInitialized=false;
		this->mcuInitializeState=1;
	}

	void mcuInitialize(){
		if (!this->receivingDataFromMcu) {
			switch (mcuInitializeState){
			case 1:
				network->log()->notice(F("Query Product Information"));
				//send to device 
				//Query Product Information:   55 aa 00 01 00 00
				{
					unsigned char mcuCommand[] = { 0x55, 0xAA, 0x00, 0x01, 0x00, 0x00 };
					commandCharsToSerial(6, mcuCommand);
				}
				mcuInitializeState++;
				break;
			case 2:
				/* wait for answer of 1 */
				break;
			case 3:
				network->log()->notice(F("Query the MCU working mode."));
				//send to device 
				//Query Product Information:   55 aa 00 02 00 00
				{
					unsigned char mcuCommand[] = { 0x55, 0xAA, 0x00, 0x02, 0x00, 0x00 };
					commandCharsToSerial(6, mcuCommand);
				}
				mcuInitializeState++;
				mcuInitializeState=5; // FIXME HY08WE: no answer from MCU on Working Mode Query 
				break;
			case 4:
				/* wait for answer of 3 */
				break;
			case 5:
				network->log()->notice(F("Query all DPs."));
				//send to device 
				//Query Product Information:   55 aa 00 08 00 00
				queryAllDPs();
				mcuInitializeState++;
				break;
			case 6:
				/* wait for answer of 5 */
				break;

			case 7:
				/* all done */
				mcuInitialized=true;
				this->mcuInitializeState=0;
				network->log()->notice(F("mcuInitialized"));
				break;

			}
		}

	}

    bool isDeviceStateComplete() {
    	if (network->isDebug()) {
    		return true;
    	}
    	for (int i = 0; i < STATE_COMPLETE; i++) {
    		if (receivedStates[i] == false) {
    			return false;
    		}
    	}
    	return true;
    }

    byte getSchedulesDayOffset() {
    	return schedulesDayOffset->getByte();
    }

	bool sendMqttHassAutodiscover(){
		if (mqttHassAutodiscoverSent) return true;
		if (!hasDevicesWithHassAutodiscoverSupport()) return true;
		network->log()->notice(F("sendMqttHassAutodiscover"));
		// https://www.home-assistant.io/docs/mqtt/discovery/
		String topic="homeassistant/climate/";
		String unique_id = (String)network->getIdx();
		unique_id.concat("_climate"); 
		topic.concat(unique_id);
		topic.concat("/config"); 
		WStringStream* response = network->getResponseStream();
		response->flush();
		char str_temp[6];
		dtostrf(this->temperaturePrecision->getDouble(), 3, 1, str_temp);
		if (getThermostatModel() == MODEL_BHT_002_GBLW ){
			response->printf_P(MQTT_HASS_AUTODISCOVERY_CLIMATE,
				network->getIdx(),
				unique_id.c_str(),
				network->getMacAddress().c_str(),
				network->getIdx(),
				network->getApplicationName().c_str(),
				network->getFirmwareVersion().c_str(),
				network->getMqttTopic(),
				str_temp 
			);
		} else if (getThermostatModel() == MODEL_BAC_002_ALW ){
			response->printf_P(MQTT_HASS_AUTODISCOVERY_AIRCO,
				network->getIdx(),
				unique_id.c_str(),
				network->getMacAddress().c_str(),
				network->getIdx(),
				network->getApplicationName().c_str(),
				network->getFirmwareVersion().c_str(),
				network->getMqttTopic(),
				str_temp 
			);
		}
		delay(50); // some extra time
		if (!network->publishMqtt(topic.c_str(), response, true)) return false;
		response->flush();

		unique_id = (String)network->getIdx();
		unique_id.concat("_sensor"); 
		topic="homeassistant/sensor/"; 
		topic.concat(unique_id);
		topic.concat("/config");
		response->printf_P(MQTT_HASS_AUTODISCOVERY_SENSOR,
			network->getIdx(),
			unique_id.c_str(),
			network->getMacAddress().c_str(),
			network->getMqttTopic()
		);
		if (!network->publishMqtt(topic.c_str(), response, true)) return false;
		response->flush();
		delay(50); // some extra time

		if (this->floorSensor->getBoolean()){
			unique_id = (String)network->getIdx();
			unique_id.concat("_floorsensor"); 
			topic="homeassistant/sensor/"; 
			topic.concat(unique_id);
			topic.concat("/config");
			response->printf_P(MQTT_HASS_AUTODISCOVERY_SENSORFLOOR,
				network->getIdx(),
				unique_id.c_str(),
				network->getMacAddress().c_str(),
				network->getMqttTopic()
			);
			if (!network->publishMqtt(topic.c_str(), response, true)) return false;
			delay(50); // some extra time
			response->flush();
		}

		unique_id = (String)network->getIdx();
		unique_id.concat("_rssi"); 
		topic="homeassistant/sensor/"; 
		topic.concat(unique_id);
		topic.concat("/config");
		response->printf_P(MQTT_HASS_AUTODISCOVERY_SENSORRSSI,
			network->getIdx(),
			unique_id.c_str(),
			network->getMacAddress().c_str(),
			network->getMqttTopic()
		);
		if (!network->publishMqtt(topic.c_str(), response, true)) return false;
		response->flush();
		delay(50); // some extra time


		mqttHassAutodiscoverSent=true;
		return true;
	}

protected:

    byte getThermostatModel() {
    	return this->thermostatModel->getByte();
    }

    bool isSupportingHeatingRelay() {
        return this->supportingHeatingRelay->getBoolean();
    }

    bool isSupportingCoolingRelay() {
        return this->supportingCoolingRelay->getBoolean();
    }

	float getTemperaturePrecision() {
		return this->temperaturePrecision->getDouble();
	}
	float getTemperatureFactor() {
		return (float) 1 / this->temperaturePrecision->getDouble();
	}

private:
    WClock *wClock;
    int receiveIndex;
    int commandLength;
    long lastHeartBeat;
    unsigned char receivedCommand[1024];
    boolean receivingDataFromMcu;
    double targetTemperatureManualMode;
    WProperty* deviceOn;
    WProperty* state;
    WProperty* targetTemperature;
    WProperty* actualTemperature;
    WProperty* actualFloorTemperature;
	WProperty* mode;
	WProperty* action;
    WProperty* schedulesMode;
    WProperty* systemMode;
    WProperty* fanMode;
    WProperty* ecoMode;
    WProperty* locked;
	WProperty *becaBits1;
	WProperty *becaBits2;
    byte schedules[54];
    boolean receivedStates[STATE_COMPLETE];
    byte schedulesDataPoint;
    WProperty* thermostatModel;
    WProperty *supportingHeatingRelay;
    WProperty *supportingCoolingRelay;
	WProperty *temperaturePrecision;
	WProperty *switchBackToAuto;
	WProperty *floorSensor;
    WProperty* ntpServer;
    WProperty* schedulesDayOffset;
	WProperty *mcuId;
    THandlerFunction onConfigurationRequest;
	THandlerFunction onPowerButtonOn;
    unsigned long lastNotify, lastScheduleNotify, lastLongLoop;
    bool schedulesChanged;
	int currentSchedulePeriod;
	bool mqttHassAutodiscoverSent;

	bool mcuInitialized;
	int mcuInitializeState;

    int getIndex(unsigned char c) {
    	const char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
    			'9', 'a', 'b', 'c', 'd', 'e', 'f' };
    	int result = -1;
    	for (int i = 0; i < 16; i++) {
    		if (c == HEX_DIGITS[i]) {
    			result = i;
    			break;
    		}
    	}
    	return result;
    }

    byte getDayOfWeek() {
    	unsigned long epochTime = wClock->getEpochTimeLocal();
    	epochTime = epochTime + (getSchedulesDayOffset() * 86400);
    	byte dayOfWeek = wClock->getWeekDay(epochTime);
    	//make sunday a seven
    	dayOfWeek = (dayOfWeek ==0 ? 7 : dayOfWeek);
    	return dayOfWeek;
    }

    bool isWeekend() {
    	byte dayOfWeek = getDayOfWeek();
    	return ((dayOfWeek == 6) || (dayOfWeek == 7));
    }

	void logIncomingCommand(const char * message, int logLevel=LOG_LEVEL_TRACE){
		network->log()->printLevel(logLevel, F("MCU: %s;%s"), message,
		this->getIncomingCommandAsString().c_str());
	}

    void processSerialCommand() {
    	if (commandLength > -1) {
			bool knownCommand = false;
    		//unknown
    		//55 aa 00 00 00 00
    		this->receivingDataFromMcu = true;
			// header 
			if (receivedCommand[0] == 0x55 && receivedCommand[1] == 0xaa) {
				//version
				if (receivedCommand[2] == 0x00 || receivedCommand[2] == 0x01 || receivedCommand[2] == 0x03 ){
					if (receivedCommand[3] == 0x00) {
						switch (receivedCommand[6]) {
						case 0x00:
						case 0x01:
							knownCommand = true;
							//ignore, heartbeat MCU
							//55 aa 01 00 00 01 01
							//55 aa 01 00 00 01 00
							if (receivedCommand[6] == 0x00){
								network->log()->notice(F("MCU sent first heart beat") );
							}
							break;
						//default:
							//notifyUnknownCommand();
						}
					} else if (receivedCommand[3] == 0x02) {
						//ignore, MCU response to working mode
						//55 aa 01 02 00 00
						network->log()->trace(F("MCU working mode %d %d"), receivedCommand[4] , receivedCommand[5] );
						if (mcuInitializeState==4) mcuInitializeState++;
						knownCommand = true;
					} else if (receivedCommand[3] == 0x03) {
						//ignore, MCU response to wifi state
						//55 aa 01 03 00 00
						network->log()->trace(F("MCU wifi state response"));
						knownCommand = true;
					} else if (receivedCommand[3] == 0x04) {
						//Setup initialization request
						// long press DOWN-button during state off
						// long press DOWN+POWER-buttons during state on
						// in second mode no WifiModeCommands (reportNetworkToMcu) are accepted
						//received: 55 aa 01 04 00 00
						
						knownCommand = true;
						network->log()->trace(F("MCU sent wifi reset"));
						if (onConfigurationRequest) {
							//send answer: 55 aa 00 04 00 01 00
							unsigned char configCommand[] = { 0x55, 0xAA, 0x00, 0x04, 0x00,
									0x00 };
							commandCharsToSerial(6, configCommand);
							onConfigurationRequest();
						}
					} else if (receivedCommand[3] == 0x05) {
						// select AP mode
						//received: 55 aa 01 05 00 01 XX   ( 00 = smartconfig, 01 = AP mode)
						// not currently sent my MCU
						knownCommand = true;
						network->log()->trace(F("MCU requests wifi mode %d"), receivedCommand[6] );
						if (onConfigurationRequest) {
							//send answer: 55 aa 00 04 00 01 00
							unsigned char configCommand[] = { 0x55, 0xAA, 0x00, 0x05, 0x00,
									0x00, 0x04 };
							commandCharsToSerial(6, configCommand);
							onConfigurationRequest();
						}
					} else if (receivedCommand[3] == 0x07) {
						bool changed = false;
						bool newChanged = false;
						bool schedulesChanged = false;
						bool newB;
						float newValue;
						byte newByte;
						byte commandLength = receivedCommand[5];
						if (mcuInitializeState==6) mcuInitializeState++;
						//Status report from MCU
						switch (receivedCommand[6]) {
						case 0x01:
							if (commandLength == 0x05) {
								//device On/Off
								//55 aa 00 06 00 05 01 01 00 01 00|01
								newB = (receivedCommand[10] == 0x01);
								changed = ((changed) || (newChanged=(newB != deviceOn->getBoolean())));
								deviceOn->setBoolean(newB);
								receivedStates[0] = true;
								logIncomingCommand("deviceOn_x01", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
								if (newChanged && newB && onPowerButtonOn) onPowerButtonOn();
							}
							break;
						case 0x02:
							if (commandLength == 0x08) {
								//target Temperature for manual mode
								//e.g. 24.5C: 55 aa 01 07 00 08 02 02 00 04 00 00 00 31
								//                                    LENGT xx xx xx xx (longer values? (for 0.1?)) 
								newValue = (float) receivedCommand[13] / getTemperatureFactor();
								changed = ((changed) || (newChanged=!WProperty::isEqual(targetTemperatureManualMode, newValue, 0.01)));
								targetTemperatureManualMode = newValue;
								if (changed) updateTargetTemperature();
								receivedStates[1] = true;
								logIncomingCommand(((String)"targetTemperature_x02:"+(String)targetTemperatureManualMode+"/"+(String)newValue).c_str(), (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;

						case 0x03:
							if (commandLength == 0x08) {
								//actual Temperature
								//e.g. 23C: 55 aa 01 07 00 08 03 02 00 04 00 00 00 2e
								newValue = (float) (int8_t)receivedCommand[13] / getTemperatureFactor();
								changed = ((changed) || (newChanged=!actualTemperature->equalsDouble(newValue)));
								actualTemperature->setDouble(newValue);
								logIncomingCommand("actualTemperature_x03", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						case 0x04:
							if (commandLength == 0x05) {
								//manualMode?
								newB = (receivedCommand[10] == 0x01);
								changed = ((changed) || (newChanged=((newB && !schedulesMode->equalsString(SCHEDULES_MODE_OFF)) || (!newB && !schedulesMode->equalsString(SCHEDULES_MODE_AUTO)))));
								schedulesMode->setString(newB ? SCHEDULES_MODE_OFF : SCHEDULES_MODE_AUTO);
								if (newChanged){
									network->log()->trace("Manual Mode newChanged to %s", (newB ? "on" : "off"));
									updateTargetTemperature();
									updateModeAndAction();
								}
								receivedStates[2] = true;
								logIncomingCommand("manualMode_x04", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						case 0x05:
							if (commandLength == 0x05) {
								//ecoMode
								newB = (receivedCommand[10] == 0x01);
								changed = ((changed) || (newChanged=(newB != ecoMode->getBoolean())));
								ecoMode->setBoolean(newB);
								receivedStates[3] = true;
								logIncomingCommand("ecoMode_x05", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						case 0x06:
							if (commandLength == 0x05) {
								//locked
								newB = (receivedCommand[10] == 0x01);
								changed = ((changed) || (newChanged=(newB != locked->getBoolean())));
								locked->setBoolean(newB);
								receivedStates[4] = true;
								logIncomingCommand("locked_x06", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						case 0x65: //MODEL_BHT_002_GBLW
						case 0x68: //MODEL_BAC_002_ALW
							if (commandLength == 0x3A) {
								//schedules 0x65 at heater model, 0x68 at fan model, example
								//55 AA 00 06 00 3A 65 00 00 36
								//00 07 28 00 08 1E 1E 0B 1E 1E 0D 1E 00 11 2C 00 16 1E
								//00 06 28 00 08 28 1E 0B 28 1E 0D 28 00 11 28 00 16 1E
								//00 06 28 00 08 28 1E 0B 28 1E 0D 28 00 11 28 00 16 1E
								this->schedulesDataPoint = receivedCommand[6];
								//this->thermostatModel->setByte(this->schedulesDataPoint == 0x65 ? MODEL_BHT_002_GBLW : MODEL_BAC_002_ALW);
								for (int i = 0; i < 54; i++) {
									newByte = receivedCommand[i + 10];
									schedulesChanged = (newChanged=((schedulesChanged) || (newByte != schedules[i])));
									schedules[i] = newByte;
								}
								logIncomingCommand(this->thermostatModel == MODEL_BHT_002_GBLW ? "schedules_x65" : "schedules_x68", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							} else if (receivedCommand[5] == 0x05) {
								//Unknown permanently sent from MCU
								//55 aa 01 07 00 05 68 01 00 01 01
								knownCommand = true;
							}
							break;
						case 0x66:
							if (commandLength == 0x08) {
								//MODEL_BHT_002_GBLW - actualFloorTemperature
								//55 aa 01 07 00 08 66 02 00 04 00 00 00 00
								newValue = (float) (int8_t)(receivedCommand[13]) / getTemperatureFactor();
								if (actualFloorTemperature != nullptr) {
									changed = ((changed) || (newChanged=!actualFloorTemperature->equalsDouble(newValue)));
									actualFloorTemperature->setDouble(newValue);
								}
								logIncomingCommand("actualFloorTemperature_x66", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							} else if (commandLength == 0x05) {
								//MODEL_BAC_002_ALW - systemMode
								//cooling:     55 AA 00 06 00 05 66 04 00 01 00
								//heating:     55 AA 00 06 00 05 66 04 00 01 01
								//ventilation: 55 AA 00 06 00 05 66 04 00 01 02
								//this->thermostatModel->setByte(MODEL_BAC_002_ALW);
								changed = ((changed) || (newChanged=(receivedCommand[10] != this->getSystemModeAsByte())));
								if (systemMode != nullptr) {
									switch (receivedCommand[10]) {
									case 0x00 :
										systemMode->setString(SYSTEM_MODE_COOL);
										break;
									case 0x01 :
										systemMode->setString(SYSTEM_MODE_HEAT);
										break;
									case 0x02 :
										systemMode->setString(SYSTEM_MODE_FAN);
										break;
									}
								}
								logIncomingCommand("systemMode_x66", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						case 0x67:
							if (commandLength == 0x05) {
								//fanSpeed
								//auto   - 55 aa 01 07 00 05 67 04 00 01 00
								//low    - 55 aa 01 07 00 05 67 04 00 01 03
								//medium - 55 aa 01 07 00 05 67 04 00 01 02
								//high   - 55 aa 01 07 00 05 67 04 00 01 01
								changed = ((changed) || (newChanged=(receivedCommand[10] != this->getFanModeAsByte())));
								if (fanMode != nullptr) {
									switch (receivedCommand[10]) {
									case 0x00 :
										fanMode->setString(FAN_MODE_AUTO);
										break;
									case 0x03 :
										fanMode->setString(FAN_MODE_LOW);
										break;
									case 0x02 :
										fanMode->setString(FAN_MODE_MEDIUM);
										break;
									case 0x01 :
										fanMode->setString(FAN_MODE_HIGH);
										break;
									}
								}
								logIncomingCommand("fanSpeed_x67", (newChanged ? LOG_LEVEL_TRACE : LOG_LEVEL_VERBOSE));
								knownCommand = true;
							}
							break;
						}
						if (knownCommand) {							
							if (changed) {
								network->log()->notice(F("ReceivedSerial/Changed"));
								notifyState();
							} else if (schedulesChanged) {
								network->log()->notice(F("ReceivedSerial/schedulesChanged"));
								notifySchedules();
							}
						}
					} else if (receivedCommand[3] == 0x01) {
						// Product ID Answer
						// 55 aa 01 01 00 15
						// HE AD VR CM LENGT 
						network->log()->notice(F("Product ID Answer from MCU (commandLength: %d)"), commandLength);
						logIncomingCommand("productId" , LOG_LEVEL_NOTICE);
						knownCommand=true;
						if (commandLength>=5){
							unsigned int len=0;
							len = ((byte)receivedCommand[4] <<8) + (byte)receivedCommand[5];
							char buf[len+1];
							for (int i=0;i<len;i++){
								buf[i]=receivedCommand[6+i];
							}
							buf[len]=0;
							this->mcuId->setString(buf);
							if (mcuInitializeState==2) mcuInitializeState++;
							network->log()->notice(F("Product ID: '%s'"), buf);

						}
					} else if (receivedCommand[3] == 0x0C) {
						//Request for time sync from MCU : 55 aa 01 0c 00 00
						network->log()->notice(F("Request for GMT time sync from MCU"));
						this->sendActualTimeToBeca(false);
						knownCommand=true;
					} else if (receivedCommand[3] == 0x1C) {
						//Request for time sync from MCU : 55 aa 01 1c 00 00
						network->log()->notice(F("Request for local time sync from MCU"));
						this->sendActualTimeToBeca(true);
						knownCommand=true;
					}

				} //55 aa 00|01
			} // 55 aa
			this->receivingDataFromMcu = false;
			if (!knownCommand){
				logIncomingCommand("unknown", LOG_LEVEL_WARNING);
			}
    	} // command length

		
    }

    void deviceOnToMcu(WProperty* property) {
    	if (!this->receivingDataFromMcu) {
       		//55 AA 00 06 00 05 01 01 00 01 01
       		byte dt = (this->deviceOn->getBoolean() ? 0x01 : 0x00);
       		unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
       		                                    0x01, 0x01, 0x00, 0x01, dt};
       		commandCharsToSerial(11, deviceOnCommand);
       		//notifyState();
     	}
    }

    void updateTargetTemperature() {
		if (ecoMode->getBoolean()) {
			targetTemperature->setDouble(ECOMODETEMP);
		} else if ((this->currentSchedulePeriod != -1) && (schedulesMode->equalsString(SCHEDULES_MODE_AUTO))) {
			double temp = (double) schedules[this->currentSchedulePeriod + 2] / getTemperatureFactor();
			byte weekDay;
			String p = String(weekDay == 0 ? SCHEDULES_DAYS[2] : (weekDay == 6 ? SCHEDULES_DAYS[1] : SCHEDULES_DAYS[0]));
			p.concat(SCHEDULES_PERIODS[this->currentSchedulePeriod]);
			network->log()->trace((String(PSTR("We take temperature from period '%s', Schedule temperature is "))+String(temp)).c_str() , p.c_str());
			targetTemperature->setDouble(temp);
		} else {
			targetTemperature->setDouble(targetTemperatureManualMode);
		}
	}

	void updateCurrentSchedulePeriod() {
		if ((receivedSchedules()) && (wClock->isValidTime())) {
    		byte weekDay = wClock->getWeekDay();
    		weekDay += getSchedulesDayOffset();
    		weekDay = weekDay % 7;
    		int startAddr = (weekDay == 0 ? 36 : (weekDay == 6 ? 18 : 0));
    		int period = 0;
    		if (wClock->isTimeEarlierThan(schedules[startAddr + period * 3 + 1], schedules[startAddr + period * 3 + 0])) {
    			//Jump back to day before and last schedule of day
    			weekDay = weekDay - 1;
    			weekDay = weekDay % 7;
    			startAddr = (weekDay == 0 ? 36 : (weekDay == 6 ? 18 : 0));
    			period = 5;
    		} else {
    			//check the schedules in same day
    			for (int i = 1; i < 6; i++) {
    				if (i < 5) {
    					if (wClock->isTimeBetween(schedules[startAddr + i * 3 + 1], schedules[startAddr + i * 3 + 0],
    							                  schedules[startAddr + (i + 1) * 3 + 1], schedules[startAddr + (i + 1) * 3 + 0])) {
    						period = i;
    						break;
    					}
    				} else if (wClock->isTimeLaterThan(schedules[startAddr + 5 * 3 + 1], schedules[startAddr + 5 * 3 + 0])) {
    					period = 5;
    				}
    			}
    		}


			int newPeriod = startAddr + period * 3;
			if (/*(getThermostatModel() != MODEL_ET_81_W) && */ (this->switchBackToAuto->getBoolean()) &&
				(this->currentSchedulePeriod > -1) && (newPeriod != this->currentSchedulePeriod) &&
				(this->schedulesMode->equalsString(SCHEDULES_MODE_OFF))) {
				network->log()->notice(PSTR("Changed automatically back to Schedule from Manual"));
				this->schedulesMode->setString(SCHEDULES_MODE_AUTO);
			}
			this->currentSchedulePeriod = newPeriod;
    	} else {
			this->currentSchedulePeriod = -1;
    	}
    }

    void setTargetTemperature(WProperty* property) {
		if (schedulesMode->equalsString(SCHEDULES_MODE_OFF)){
			// only set targetTemperatureManualMode and targetTemperatureManualModeToMcu() if current mode is Manual
			if (!WProperty::isEqual(targetTemperatureManualMode, this->targetTemperature->getDouble(), 0.01)) {
				targetTemperatureManualMode = this->targetTemperature->getDouble();
				network->log()->trace((String(PSTR("setTargetTemperature, temp: "))+String(targetTemperatureManualMode)).c_str());
				targetTemperatureManualModeToMcu();
				schedulesMode->setString(SCHEDULES_MODE_OFF);
			} else {
				network->log()->trace((String(PSTR("setTargetTemperatureNoChange, temp: "))+String(this->targetTemperature->getDouble())).c_str());
			}
		}
    }

    void targetTemperatureManualModeToMcu() {
    	if (!this->receivingDataFromMcu) {
    		network->log()->notice((String(F("Set target Temperature (manual mode) to "))+String(targetTemperatureManualMode)).c_str());
    	    //55 AA 00 06 00 08 02 02 00 04 00 00 00 2C
			byte dt = (byte) (targetTemperatureManualMode * getTemperatureFactor());
    	    unsigned char setTemperatureCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x08,
    	    		0x02, 0x02, 0x00, 0x04,
					0x00, 0x00, 0x00, dt};
    	    commandCharsToSerial(14, setTemperatureCommand);
    	}
    }

    void schedulesModeToMcu(WProperty* property) {
    	if (!this->receivingDataFromMcu) {
        	//55 AA 00 06 00 05 04 04 00 01 01
        	byte dt = (schedulesMode->equalsString(SCHEDULES_MODE_OFF) ? 0x01 : 0x00);
        	unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
        	                                    0x04, 0x04, 0x00, 0x01, dt};
        	commandCharsToSerial(11, deviceOnCommand);
        }
    }

    void ecoModeToMcu(WProperty* property) {
       	if (!this->receivingDataFromMcu) {
       		//55 AA 00 06 00 05 05 01 00 01 01
       		byte dt = (this->ecoMode->getBoolean() ? 0x01 : 0x00);
       		unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
       		                                    0x05, 0x01, 0x00, 0x01, dt};
       		commandCharsToSerial(11, deviceOnCommand);
       		//notifyState();
       	}
    }
 	void modeToMcu(WProperty* property) {
		network->log()->trace(F("modeToMcu %s"), property->c_str());

		if (getThermostatModel() == MODEL_BHT_002_GBLW) {
			if (this->mode->equalsString(MODE_OFF)){
				this->deviceOn->setBoolean(false);
				this->schedulesMode->setString(SCHEDULES_MODE_OFF);
			} else {
				this->deviceOn->setBoolean(true);
				if (this->mode->equalsString(MODE_AUTO)){
					this->schedulesMode->setString(SCHEDULES_MODE_AUTO);
				} else if (this->mode->equalsString(MODE_HEAT)){
					this->schedulesMode->setString(SCHEDULES_MODE_OFF);
				} else {
					network->log()->warning(F("modeToMcu unknown mode %s"), property->c_str());
				}
			}
		}
		if (getThermostatModel() == MODEL_BAC_002_ALW){		
			if (this->mode->equalsString(MODE_OFF)){
				this->schedulesMode->setString(SCHEDULES_MODE_OFF);
				this->deviceOn->setBoolean(false);
			} else {
				this->deviceOn->setBoolean(true);
				this->fanMode->setString(FAN_MODE_AUTO);
				if (this->mode->equalsString(MODE_HEAT)){
					this->schedulesMode->setString(SCHEDULES_MODE_OFF);
					this->systemMode->setString(SYSTEM_MODE_HEAT);
				} else if (this->mode->equalsString(MODE_COOL)){
					this->schedulesMode->setString(SCHEDULES_MODE_OFF);
					this->systemMode->setString(SYSTEM_MODE_COOL);
				} else if (this->mode->equalsString(MODE_FAN)){
					this->schedulesMode->setString(SCHEDULES_MODE_OFF);
					this->systemMode->setString(SYSTEM_MODE_FAN);
				} else if (this->mode->equalsString(MODE_AUTOHEAT)){
					this->schedulesMode->setString(SCHEDULES_MODE_AUTO);
					this->systemMode->setString(SYSTEM_MODE_HEAT);
				} else if (this->mode->equalsString(MODE_AUTOCOOL)){
					this->schedulesMode->setString(SCHEDULES_MODE_AUTO);
					this->systemMode->setString(SYSTEM_MODE_COOL);
				} else if (this->mode->equalsString(MODE_AUTOFAN)){
					this->schedulesMode->setString(SCHEDULES_MODE_AUTO);
					this->systemMode->setString(SYSTEM_MODE_FAN);
				} else {
					network->log()->warning(F("modeToMcu unknown mode %s"), property->c_str());
				}
			}
		}
	}

    void updateModeAndAction() {
		if (!this->deviceOn->getBoolean()){
			this->mode->setString(MODE_OFF);
		} else if (this->schedulesMode->equalsString(SCHEDULES_MODE_AUTO)){
			if (getThermostatModel() == MODEL_BAC_002_ALW){
				if (this->systemMode->equalsString(SYSTEM_MODE_HEAT)){
					this->mode->setString(MODE_AUTOHEAT);
				} else if (this->systemMode->equalsString(SYSTEM_MODE_COOL)){
					this->mode->setString(MODE_AUTOCOOL);
				} else if (this->systemMode->equalsString(SYSTEM_MODE_FAN)){
					this->mode->setString(MODE_AUTOFAN);
				} 
			} else if (getThermostatModel() == MODEL_BHT_002_GBLW) {
				this->mode->setString(MODE_AUTO);
			} else {
				network->log()->error(F("Bug. Can't find mode (on+auto+?)"));
			}
		} else if (getThermostatModel() == MODEL_BAC_002_ALW) {
			if (this->systemMode->equalsString(SYSTEM_MODE_HEAT)){
				this->mode->setString(MODE_HEAT);
			} else if (this->systemMode->equalsString(SYSTEM_MODE_COOL)){
				this->mode->setString(MODE_COOL);
			} else if (this->systemMode->equalsString(SYSTEM_MODE_FAN)){
				this->mode->setString(MODE_FAN);
			}
		} else if (getThermostatModel() == MODEL_BHT_002_GBLW){
			this->mode->setString(MODE_HEAT);
		} else {
			network->log()->error(F("Bug. Can't find mode (on+noauto+?)"));
		}

		// mode to action
		if (this->mode->equalsString(MODE_OFF)) this->action->setString(ACTION_OFF);
		else if (getThermostatModel() == MODEL_BAC_002_ALW) {
			if ((isSupportingCoolingRelay() || isSupportingHeatingRelay()) && this->state->equalsString(STATE_OFF)){
				this->action->setString(ACTION_IDLE);
			} else	if (this->systemMode->equalsString(SYSTEM_MODE_HEAT)){
				this->action->setString(ACTION_HEATING);
			} else if (this->systemMode->equalsString(SYSTEM_MODE_COOL)){
				this->action->setString(ACTION_COOLING);
			} else if (this->systemMode->equalsString(SYSTEM_MODE_FAN)){
				this->action->setString(ACTION_FAN);
			} 
		} else if (getThermostatModel() == MODEL_BHT_002_GBLW){
			if ((isSupportingCoolingRelay() || isSupportingHeatingRelay()) && this->state->equalsString(STATE_OFF)){
				this->action->setString(ACTION_IDLE);
			} else this->action->setString(ACTION_HEATING);
		}
	}

    void lockedToMcu(WProperty* property) {
       	if (!this->receivingDataFromMcu) {
       		//55 AA 00 06 00 05 06 01 00 01 01
       		byte dt = (this->locked->getBoolean() ? 0x01 : 0x00);
       		unsigned char deviceOnCommand[] = { 0x55, 0xAA, 0x00, 0x06, 0x00, 0x05,
       		                                    0x06, 0x01, 0x00, 0x01, dt};
       		commandCharsToSerial(11, deviceOnCommand);
       		//notifyState();
       	}
    }

    bool receivedSchedules() {
    	return ((network->isDebug()) || (this->schedulesDataPoint != 0x00));
    }

    void notifyState() {
    	lastNotify = 0;
    }

    void notifySchedules() {
    	lastScheduleNotify = 0;
    }

    void resetAll() {
       	receiveIndex = -1;
       	commandLength = -1;
    }



	void saveSettings(WProperty* property) {
		network->log()->trace(F("saveSettings"));
		network->getSettings()->save();
	}

	bool hasDevicesWithHassAutodiscoverSupport(byte device){
		for (int i=0; i<sizeof(devicesWithHassAutodiscoverSupport); i++){
			if (devicesWithHassAutodiscoverSupport[i]==device) return true;
		}
		return false;
	}

	bool hasDevicesWithHassAutodiscoverSupport(){
		return hasDevicesWithHassAutodiscoverSupport(getThermostatModel());
	}

	bool hasInfoPage() {
		return true;
	}

	void printInfoPage(WStringStream* page) {

		htmlTableRowTitle(page, F("MCU-Initialized:"));
		page->print((this->isMcuInitialized() ? "Yes" : "No"));
		htmlTableRowEnd(page);

		htmlTableRowTitle(page, F("MCU-ID:"));
		page->print(this->mcuId->c_str());
		htmlTableRowEnd(page);


		htmlTableRowTitle(page, F("Device On:"));
		page->print((deviceOn->getBoolean() ? "Device On" : "Device Off"));
		htmlTableRowEnd(page);

		htmlTableRowTitle(page, F("Current Temperature:"));
		page->print(actualTemperature->getDouble());
		htmlTableRowEnd(page);

		if (getThermostatModel() == MODEL_BHT_002_GBLW && this->floorSensor->getBoolean()){
			htmlTableRowTitle(page, F("Current Floor-Temperature:"));
			page->print(actualFloorTemperature->getDouble());
			htmlTableRowEnd(page);
		}

		htmlTableRowTitle(page, F("Target Temperature:"));
		page->print(targetTemperature->getDouble());
		htmlTableRowEnd(page);

		htmlTableRowTitle(page, F("Current Mode:"));
		page->print(systemMode->c_str());
		htmlTableRowEnd(page);

		htmlTableRowTitle(page, F("Schedules Mode:"));
		page->print(schedulesMode->c_str());
		htmlTableRowEnd(page);

		htmlTableRowTitle(page, F("Edo Mode:"));
		page->print((schedulesMode->getBoolean() ? "Eco On" : "Eco Off"));
		htmlTableRowEnd(page);

		if (getThermostatModel() == MODEL_BAC_002_ALW){
			htmlTableRowTitle(page, F("System Mode:"));
			page->print(systemMode->c_str());
			htmlTableRowEnd(page);

			htmlTableRowTitle(page, F("Fan Mode:"));
			page->print(fanMode->c_str());
			htmlTableRowEnd(page);
		}

		htmlTableRowTitle(page, F("Current Action:"));
		page->print(action->c_str());
		htmlTableRowEnd(page);

		if ((isSupportingHeatingRelay()) || (isSupportingCoolingRelay())) {
			htmlTableRowTitle(page, F("Current State:"));
			page->print(state->c_str());
			htmlTableRowEnd(page);
		}

	}
};


#endif

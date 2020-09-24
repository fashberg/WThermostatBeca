#include <Arduino.h>

#include "../lib/WAdapter/WAdapter/WNetwork.h"
#ifndef MINIMAL
#include "WBecaDevice.h"
#include "WClock.h"
#include "WLogDevice.h"
#include "../lib/WAdapter/WAdapter/WNetworkDevice.h"
#endif
#define APPLICATION "Thermostat"
#ifndef VERSION
#define VERSION "undefined" // gets defined in commandline
#endif

#ifdef MINIMAL
#define FULLVERSION VERSION "-minimal"
#elif DEBUG
#define FULLVERSION VERSION "-debug"
#else
#define FULLVERSION VERSION
#endif

#ifdef DEBUG // use platform.io environment to activate/deactive 
#define SERIALDEBUG true  // enables logging to serial console
#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif
#define SERIALSPEED 115200
#else
#define SERIALDEBUG false
#define DEBUG_MSG(...)
#define SERIALSPEED 9600
#endif

const byte FLAG_OPTIONS_APPLICATION = 0xF0;

WNetwork *network;
#ifndef MINIMAL
WNetworkDev *networkDevice;
WLogDevice *logDevice;
WBecaDevice *becaDevice;
WClock *wClock;
#endif

void setup() {
    Serial.begin(SERIALSPEED);

    // Wifi and Mqtt connection
    network = new WNetwork(SERIALDEBUG, APPLICATION, FULLVERSION, NO_LED, FLAG_OPTIONS_APPLICATION);
#ifndef MINIMAL
    network->setOnNotify([]() {
        if (network->isSoftAPDesired()){
            becaDevice->reportNetworkToMcu(mcuNetworkMode::MCU_NETWORKMODE_SMARTCONFIG);
        } else if (network->isStation()){
            if (network->isWifiConnected()) {
                becaDevice->reportNetworkToMcu(mcuNetworkMode::MCU_NETWORKMODE_CONNECTEDCLOUD);
                if (network->isMqttConnected()) {
                    becaDevice->queryAllDPs();
                }
            } else {
                becaDevice->reportNetworkToMcu(mcuNetworkMode::MCU_NETWORKMODE_NOTCONNECTED);
            }
        }
        if (networkDevice) networkDevice->connectionChange();
    });
    network->setOnConfigurationFinished([]() {
        // Switch blinking thermostat in normal operating mode back
        network->log()->warning(F("ConfigurationFinished"));
        becaDevice->cancelConfiguration();
    });


    // networkDevice 
    network->log()->trace(F("Loading Network (%d)"), ESP.getFreeHeap());
    networkDevice = new WNetworkDev(network, APPLICATION);
    network->addDevice(networkDevice);
    network->log()->trace(F("Loading Network Done (%d)"), ESP.getFreeHeap());

    // KaClock - time sync
    network->log()->trace(F("Loading Clock (%d)"), ESP.getFreeHeap());
    wClock = new WClock(network, APPLICATION);
    network->addDevice(wClock);
    wClock->setOnTimeUpdate([]() { becaDevice->sendActualTimeToBecaRequested(true); });
    wClock->setOnError([](const char *error) {
        network->log()->error(F("Clock Error: %s"), error);
    });
    network->log()->trace(F("Loading Clock Done (%d)"), ESP.getFreeHeap());

    // Communication between ESP and Beca-Mcu
    network->log()->trace(F("Loading BecaDevice (%d)"), ESP.getFreeHeap());
    becaDevice = new WBecaDevice(network, wClock);
    becaDevice->setMqttSendChangedValues(true);
    network->addDevice(becaDevice);

    becaDevice->setOnConfigurationRequest([]() {
        network->setDesiredModeAp();
        return true;
    });
    becaDevice->setOnPowerButtonOn([]() {
        // try to go to Station mode if we should be in any other state
        // example: long press power + down for 8 seconds switches to 
        // WiFi reset mode, but blinking mode is not accepted
        network->setDesiredModeStation();
        return true;
    });
    network->log()->trace(F("Loading BecaDevice Done (%d)"), ESP.getFreeHeap());

    // add MQTTLog
    network->log()->trace(F("Loading LogDevice (%d)"), ESP.getFreeHeap());
    logDevice = new WLogDevice(network);
    network->addDevice(logDevice);
    network->log()->trace(F("Loading LogDevice Done (%d)"), ESP.getFreeHeap());

    if (network->getSettings()->settingsNeedsUpdate()){
        network->deleteSettingsOld();
        network->log()->trace(F("Writing Config (%d)"), ESP.getFreeHeap());
        #ifndef DEBUG
        network->getSettings()->save();
        #else
        network->log()->trace(F("Writing Config - not really, because debug MODE"));
        #endif
    }

    network->setOnMqttHassAutodiscover([](bool removeDiscovery) {
        // https://www.home-assistant.io/integrations/climate.mqtt/
        return becaDevice->sendMqttHassAutodiscover(removeDiscovery);
    });
#endif
    network->log()->trace(F("Starting Webserver Done (%d)"), ESP.getFreeHeap());
    network->startWebServer();
    network->log()->trace(F("Starting Webserver Done (%d)"), ESP.getFreeHeap());

}

void loop() {
    network->loop(millis());
    delay(50);
}

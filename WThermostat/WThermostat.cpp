#include <Arduino.h>

#include "../lib/WAdapter/WAdapter/WNetwork.h"
#ifndef MINIMAL
#include "WBecaDevice.h"
#include "WClock.h"
#include "WLogDevice.h"
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

WNetwork *network;
#ifndef MINIMAL
WLogDevice *logDevice;
WBecaDevice *becaDevice;
WClock *wClock;
#endif

void setup() {
    Serial.begin(SERIALSPEED);

    // Wifi and Mqtt connection
    network = new WNetwork(SERIALDEBUG, APPLICATION, FULLVERSION, NO_LED);
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
    });
    network->setOnConfigurationFinished([]() {
        // Switch blinking thermostat in normal operating mode back
        network->log()->warning(F("ConfigurationFinished"));
        becaDevice->cancelConfiguration();
    });

    // KaClock - time sync

    wClock = new WClock(network, APPLICATION);
    network->log()->trace(F("Loading ClockDevice"));
    network->addDevice(wClock);
    wClock->setOnTimeUpdate([]() { becaDevice->sendActualTimeToBeca(true); });
    wClock->setOnError([](const char *error) {
        network->log()->error(F("Clock Error: %s"), error);
    });
    network->log()->trace(F("Loading ClockDevice Done"));

    // Communication between ESP and Beca-Mcu
    network->log()->trace(F("Loading BecaDevice"));
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
    network->log()->trace(F("Loading BecaDevice Done"));

    // add MQTTLog
    network->log()->trace(F("Loading LogDevice"));
    logDevice = new WLogDevice(network);
    network->addDevice(logDevice);
    network->log()->trace(F("Loading LogDevice Done"));

    if (network->getSettings()->settingsNeedsUpdate()){
        network->deleteSettingsOld();
        network->log()->trace(F("Writing Config"));
        #ifndef DEBUG
        network->getSettings()->save();
        #else
        network->log()->trace(F("Writing Config - not really, because debug MODE"));
        #endif
    }

    network->setOnMqttHassAutodiscover([]() {
        // https://www.home-assistant.io/integrations/climate.mqtt/
        return becaDevice->sendMqttHassAutodiscover();
    });
#endif
    network->startWebServer();

}

void loop() {
    network->loop(millis());
    delay(50);
}

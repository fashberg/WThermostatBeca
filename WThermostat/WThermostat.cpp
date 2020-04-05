#include <Arduino.h>

#include "../lib/WAdapter/WAdapter/WNetwork.h"
#include "WBecaDevice.h"
#include "WClock.h"
#include "WLogDevice.h"

#define APPLICATION "Thermostat Beca"
#define VERSION "1.08b"

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
WLogDevice *logDevice;
WBecaDevice *becaDevice;
WClock *wClock;

void setup() {
    Serial.begin(SERIALSPEED);

    // Wifi and Mqtt connection
    network = new WNetwork(SERIALDEBUG, APPLICATION, VERSION, NO_LED);
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


    network->startWebServer();

}

void loop() {
    network->loop(millis());
    delay(50);
}

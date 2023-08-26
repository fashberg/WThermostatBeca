# Changelog

## Version 1.22-fas

* Fixes invalid JSON responses/escaping (fixes #153)
* Fixes MQTT-Reconnect
* Doc updates

## Version 1.21-fas

* reimplemented Away-Mode with Preset (Heating and A/C)
* fixed scheduler-Mode with Preset 'Scheduler' (A/C)
* updated docs and samples

## Version 1.20-fas

### Changes in this Release

* HA 2022.9 compatibility #153, #146. Thanks to @labodj
* Update to latest dependencies and compatibility @labodj/@fashberg
* decreased minimum temp by @zawasp  / #98
* removed away-mode @jressel01 #148

### And Build CI updates

* moved from travis-ci to github-actions @fashberg

## Version 1.19-fas

* Fix/Enhancement #54: If enabled, all single Properties are sent out every notification interval, not only if changed
* Fix #52: Documentation on SSID and renamed Wifi to WiFi
* Fix #53 and #64: Boolean Values wrong

## Version 1.18-fas

* Fixed Relay State Calculation #26 again
* Daylight Saving Bug #62 fixed

## Version 1.17-fas

* Fix Relay State Calculation #26 - thanks IATkachenko
* Fix reported targetTemperature for Schdule Mode #50

## Version 1.16-fas

* NewFeature: RelayStateCalculation - shows current state of heating/cooling
* NewFeature: WLAN Signal Level (RSSI) visible (MQTT, Info-Page, HASS)
* NewFeature: Full Support for BAC-002 with Home-Assistant autodiscovery
* HASS Climate Autodiscovery BAC-002
  * removed 'auto' from Read/Write-Property ``mode`` (not supported by device). Valid Modes: ``["heat","cool","fan_only","off"]``
  * Read/Write-Property ``holdState`` with possible values ``["scheduler", "manual", "away"]`` - switches between Scheduler, Manual and Eco-Mode
  * Fixed fanMode behaviour (auto-mode now working, setting fan does not switches to fan_only)
  * Fixed manual configuration snippet in README.md
* NewFeature: Live WebLog in Browser (WebSockets)
* NewFeature: Workaround for imprecise clock on LCD
* NewFeature: All Information on Info-Page
* Develpment: Gitpd support (<https://github.com/fashberg/WThermostatBeca#cloud-development-using-gitpod>)
* Develpment: Travis CI support (<https://travis-ci.org/github/fashberg/WThermostatBeca>)
* Develpment: platformio_override.ini documentation and sample
* Small Fixes/Enhancements
  * ecoMode temperature Fixed to 20 degrees (not changeable in MCU) #32
  * WebThings autodiscovery fix
  * IceBucketChallenge: Support for negative temperatures
  * Apple Captive Portal support
  * Case Insensitive true/false (HomeAssistant Uses "True" if boolean values are enabled) #32
  * Typos

## Version 1.15-fas

* Bugfix issue #20: Hardware Hacks not saved (thanks ilkkaolavas)
* Bugfix: no instant mqtt reconnect after network-reset mode
* New: floorSensor configurable (default off). If Off: not shown in MQTT/WebThings
  * If enabled und HASS-Autodiscovery also enabled: show additional Sensor in HASS
* New: Support changing switchBackToAuto via MQTT
  * saved in EEPROM permanently on Change
  * mosquitto_pub -h mqtt -t TOPIC/cmnd/things/thermostat/properties/switchBackToAuto  -m "true"
* Bugfix: Crash on setting non-existant property using MQTT
* Bugfix: No default clock settings after fresh install
* Improvement: Show empty Passwordfield if password is empty instead of \_\_NOCHANGE__
* by Folke Ashberg <folke@ashberg.de>

## Version 1.14-fas

* MQTT Last Will & Testament (topic/tele/LWT)
* MQTT Hass Availibility Topic Support (also with autodiscover)
* Support for switching back automatically from manual temperature to scheduled temperature
* Reading NetworkSettings support for 1.00-1.11 and 1.xx-fas Versions
* Writing NetworkSettings same way Version >=1.09 does
* Changed '_' in default hostname to '-' to be RFC952 compliant
* Changed default Hostname to "Thermostat" (to fit hostname in 16 chars)
* Show passwords fields in ClearText on Request
* by Folke Ashberg <folke@ashberg.de>

## Version 1.13-fas

* Never use thirteen (tm)

## Version 1.12-fas

* HASS Autodiscovery interval fix #10 (thanks again austwhite)
* installation documentation
* by Folke Ashberg <folke@ashberg.de>

## Version 1.11-fas

* HASS Autodiscovery Fix and Improvements
* by <https://github.com/austwhite>
* platform.io automatic firmware naming
* documentation updates
* fixed targetTemp wrong on change from scheduleMode to Manual #9 (thanks austwhite)
* by Folke Ashberg <folke@ashberg.de>

## Version 1.10-fas

* introduced -minimal version for interim-upgrade on small sketch size
* new "action" property for hass - thanks austwhite
* keep only WLAN settings from compatibility Firmware
* Fix too less page-buffor for startpage (reboot button missing)
* Uptime seconds fix
* by Folke Ashberg <folke@ashberg.de>

## Version 1.09-fas

* WebThings back working (/things) - thanks erelor
* Optional sending of changed values to its own MQTT topic with no JSON - thanks Bettman66
* by Folke Ashberg <folke@ashberg.de>

## Version 1.08-fas

* New Feature: Supports now Climate Autoconfiguration for Home Assistant (optional)
* New Feature: Switch Off (middle-button), then Longpress Down-Button, Beca switches to AP-Mode with default Password
* Automatic Wifi Fallback now optional
* Bugfix state cooling (wrong setting in EPROM, was not configurable)
* Temperature Precision 0.5/1.0
* some upstream fixes from klausahrenberg 1.03
* by Folke Ashberg <folke@ashberg.de>

## Version 1.07-fas

* Device goes to AP-Mode if Wifi is not available/fails and switches back after 5 Minutes
* enabled WLAN auto-reconnect
* current Passwords not outputted to websites
* config page opens automatically as captive Portal on android devices if in AP mode
* Clock NTP Resync fix
* uptime in clock status
* fixed several memleaks
* by Folke Ashberg <folke@ashberg.de>

## Version 1.06b

* fixed json buffer size (WebThings)
* by Folke Ashberg <folke@ashberg.de>

## Version 1.05b

* Fix for heating/cooling devices (systemMode/fanMode)
* by Folke Ashberg <folke@ashberg.de>

## Version 1.04b

* moved webthings-page to /webthings (DNS-Discovery adopted)
* root-page is now config-page
* new Logging to MQTT
  * old logMcu removed
* removed worldtimeapi and implemented own offline hanling of timezone and daylightsaving
* schedules editable on web GUI
* send changed schedules via MQTT if changed manually on device
* PLEASE CHECK all Settings after Upgrade!
* by Folke Ashberg <folke@ashberg.de>

## Version 1.03b

* fix: Scheduler: wrong MQTT returned on empty payload.
  requesting mqtt <your_topic>/cmnd/things/thermostat/schedules with empty payload suppose to return json to the <your_topic>/stat/things/thermostat/schedules
  but returned to <your_topic>/stat/things/thermostat/properties
  Thanks to nimda5 <https://github.com/klausahrenberg/WThermostatBeca/pull/81#issuecomment-598461168>
* fix: scheduler gave no MQTT response every 2nd command
* fix: responded twice on empty payload
* by Folke Ashberg <folke@ashberg.de>

## Version 1.02b

* Fix: logMcu command is now working again (&lt;TOPIC&gt;/cmnd/things/thermostat/properties/logMcu)
* Fix: manual temperature change on device now updated in realtime
* stats/things/thermostat now with MQTT-Retain-Flag
* stats/things/thermostat now every 60 seconds
* more debug and log messages with logMcu
* by Folke Ashberg <folke@ashberg.de>

## Version 1.01b

* MQTT Topics devided into 'cmnd' for receiving and 'stat' for sending
* Added new paramter 'Mode' controlling and monitoring both parameters deviceOn and schedulesMode
* Better Home Assitant Support
* by Folke Ashberg <folke@ashberg.de>

## Version 1.00

* Fix: ESP disconnected and was not reconnecting automaticly

## Version 0.99

* Serveral fixes for webinterface (e.g. Upload in Chrome was not working)
* Fix schedules. ESP was hanging in endless loop in last 2 schedules of day

## Version 0.98

* Webserver can't be disabled anymore to ensure that thermostat can configered all the time
* MQTT port can be configured now
* Serveral fixes for webinterface (Saved message, no timeout, clock settings)

## Version 0.97

* Improved behavior for targetTemperature. If target is changed via MQTT/Webthing and thermostat is in 'auto mode', the thermotat switches in 'manual mode' now. Before only the target for 'manual mode' was changed, but had no effect when thermostat was in 'auto mode'
* Fix: targetTemperature gives correct actual value now, when in 'auto mode'

## Version 0.96

* Supports Mozilla Webthings, properties like temperature, desiredTemperature, On/Off, manualMode are available in a fancy web interface which can control the device outside your home network also. MQTT still supports more properties.
* Device configuration via Web-Interface added for things like: Model, NTP-Server, Timezone API, Weekday offset
* Heating relay status supported. To make this work, a hardware modification is needed: 2x10kOhm resistors have to be soldered to IO 5 of the ESP. For detailed description with pictures, see here: <https://github.com/klausahrenberg/ThermostatBecaWifi/issues/17#issuecomment-552078026>
* ArduinoJson library removed, own json creation and parsing implemented
* ESPAsyncWebServer library removed because of memory issues
* Fix: Automatic reconnection after loss of network should work now

## Version 0.95

* Fix: Buffer for time zone sync increased again to 1024. Length of "http://worldtimeapi.org/api/ip" response had increased because of new parameters

## Version 0.94

* Fix: Schedule temperature can now set in 0.5 steps

## Version 0.93

* Fix: Property "systemMode": Changing of cooling mode at the device should trigger an mqtt message now.
* Fix: Property "schedules": Changing of schedules at device should trigger mqtt messages now.

## Version 0.92

* Fix: Property "systemMode" corrected

## Version 0.91

* Property "systemMode" for model BAC-002-ALW added; possible values: "cooling"|"heating"|"ventilation"
* Property "schedulesDayOffset" added (default: 0); details see description
* Property "thermostatModel" added; values "BHT-002-GBLW"|"BAC-002-ALW"; only readable. Model will be configered by the firmware based on the results of the MCU at device initialization
* Property "actualFloorTemperature" only in device state message included at model BHT-002-GBLW
* Property "fanSpeed" and "systemMode" only in device state message included at model BAC-002-ALW
* Removed some detail informations about the time sync from standard device state message. Details must be requered with a separate clock command
* Fix: schedules for model BAC-002-ALW should now read correctly out of the device
* Fix: wrong clock time zone calculation corrected, if DST was active

## Version 0.9

* Fix: logMcu command is now working
* Fix: webService command is now working: README.md corrected to command 'webServer'. However, now are both words working.
* Fix: locked command is now working with payload false
* Fix: changing the webServer command does invoke a state update over mtqq now

## Version 0.8

* Support for fan at some models with property "fanSpeed" wit states "auto"|"low"|"medium"|"high". If no fan is available at thermostat, "fanSpeed" will always stay at "none"
* With MQTT parameter "logMcu" true|false raw commands from MCU will forwarded to MQTT. By default disabled/false
* IP address added in json state message
* Fix: Devices with cooling/heating function do not sent actualTemperature, actualFloorTemperature and schedules at initialization - the result was, that never a device state was reported to MQTT because the information was incomplete. The state will be send now, even not all information are available yet

## Version 0.7

* increased JSON buffer in KaClock.cpp to prevent parsing failure
* definition of NTP_SERVER in ThermostatBecaWifi.ino to set a NTP server near by
* state request via mqtt
* state record includes now IP address of thermostat and if webServer is Running or not

## Version 0.6

* initial version

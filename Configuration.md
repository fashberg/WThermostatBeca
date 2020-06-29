# Configuration of Thermostat
This document describes the first configuration steps after flashing of firmware. The firmware supports MQTT messaging and 
Mozilla Webthings. Both can be running parallel.

Steps are in general:
1. Connect to the WiFi Access Point of the Thermostat
2. Connect to Thermostat GUI and Configure Network access
3. Configure MQTT (optional)
4. Connect to Thermostat GUI again in your network
5. Configure thermostat device (model selection)
6. Configure clock settings


## 1. Connect to the WiFi Access Point of the Thermostat
* The thermostat becomes an Access Point when it's started first time after flashing with no configuration.
* The AccessPoint is named `Thermostat-xxxxxx`. Default password is `12345678`
* Use your Smartphone or any other WiFi Client with webbrowser and connect to the Thermostat.
* You'll get an IP Adress by DHCP in Range 192.168.4.0/24

## 2. Connect to Thermostat GUI and Configure Network access
* After you are connected to the Thermostat's WiFi open `http://192.168.4.1` in a web browser
  * On Android Smartphones this Page opens automatically as a captive portal 
* Goto 'Configure network'
* Fill out 'Hostname/Idx' (unique id of your choice), 'Wifi SSID' (only 2.4G), 'Password' for Wifi
* If you don't want to use MQTT, press 'Save Configuration' and wait for reboot of device.
![homeassistant](docs/Setup_Network.png)  

## 2. Configure MQTT (optional) and HASS Autodiscovery
* Stay at page 'Network configuration'
* Select checkbox 'Support MQTT', web page will extend
* Fill out 'MQTT Server', 'MQTT User' (optional), 'MQTT password' (optional) and 'MQTT topic'
* If you are using Home Assistant then check "Support Autodisovery for Home Assistant using MQTT".
* For some other Smart Home Server (e.g. OpenHAB) optionally check "Send all values also as single values via MQTT" (then you don't have to parse JSON-Strings from summarized output).
* Press 'Save Configuration' and wait for reboot of device.


## 4. Connect to Thermostat GUI again in your network
* Afer you have pressed "Save configuration" the thermostat reboots and tries to connect to the configured network
* Now you have to find out the IP-Adress of the device.
  * Try to open http://&lt;hostname&gt;.&lt;domainname&gt;/ - if your router (DHCP+DNS) supports it. The Hostname is what you have configured before
  * Alternative: Connect to your router and check for the thermostat in Hostlist or check your DHCP-Server leases table
  * Alternative: After restart the thermostat sends MQTT messages to topics 'devices/thermostat', 'devices/clock' and 'devices/logging' to let you know the IP and MQTT topic of the device. The json messages are looking like:
```json
{
  "url":"http://xxx.xxx.xxx.xxx/things/thermostat",
  "ip":"xxx.xxx.xxx.xxx",
  "topic":"<your_topic>/[cmnd|stat|tele]/things/thermostat"
}
```
* Open the webpage of the thermostat in your favorite browser `http://<device_ip>/`

## 3. Configure thermostat device (model selection)
* Goto 'Configure device'
* Choose your thermostat model
* Choose, if heating or cooling relay monitor is supported, hw modification need to work, see https://github.com/klausahrenberg/WThermostatBeca/issues/17#issuecomment-552078026
* Choose if floor sensor values should be reported (enable if you have connected floor sensor)
* Choose if the thermostat should switch back from manual heating mode to schedule mode at the next scheduled period change
* Choose work day and weekend start in your region
* Press 'Save Configuration' and wait for reboot of device.

![homeassistant](docs/Setup_Thermostat.png)  


## 4. Configure clock settings
* Goto 'Configure clock'
* Modify 'NTP server' for time synchronisation
* Modify 'Time zone' for time offset synchronisation depending on your location
  * Set Timezone to values -13:30 .. 13:30 for fixed timezone
  * Set Timezone to 99 for Daylight Saving
  * DST/STD: W,M,D,h,T
  W = week (0 = last week of month, 1..4 = first .. fourth)
  M = month (1..12)
  D = day of week (1..7 1 = sunday 7 = saturday)
  h = hour (0..23)
  T = timezone (-810..810) (offset from UTC in MINUTES - 810min / 60min=13:30)
  * It's adopted from https://tasmota.github.io/docs/Commands/#timestd - but you don't need the Hemisphere Setting (first number), its calculated
* Examples:

  | Region | Timezone | DST | STD | Explanation |
  | ---------- | ---------- | ----- | ----- | ---- |
  | UTC | 0 |  |  | Just UTC
  | Pacific/Honolulu | 10 |  |  | Fixed 10 Hours Offset, no Daylight saving
  | Asia/Beijing | -8 |  | |  Fixed -8 hours Offset
  | Europe/Berlin | 99 |  0,3,0,2,120 | 0,10,0,3,60 | DST from last Sunday in March at 2 o'clock with 2h Offset from UTC and ends at last sunday in October at 3 o clock with 1 hour offset during standard time
  | Europe/London | 99 |  0,3,0,2,60 | 0,10,0,3,0 | DST from last Sunday in March at 2 o'clock with 1h Offset from UTC and ends at last sunday in October at 3 o clock with no offset during standard time
  | America/New_York | 99 | 2,3,0,2,-240 | 1,11,0,2,-300 | DST from 2nd Sunday in March at 2 o'clock with -4h Offset from UTC and ends at first sunday in November at 3 o clock with -5h offset during standard time
  | Australia/Sydney | 99 | 1,10,1,2,660 | 1,4,1,3,600 | DST from first Sunday in October at 3 o'clock with 11h Offset from UTC and ends at first sunday in April at 2 o clock with 10h offset during standard time



## 5. Troubleshooting

### Logging
If anything wents wrong set MQTT-Logging to "trace" and monitor with MQTT-Client:
```
mosquitto_sub  -h  <mqttserver> -v -t "<MQTT-TOPIC>/tele/log/#"
```

### Network Recovery
If you cannot access your device, try to switch to Access-Point mode:
* Power off the device by Pressing the button in the middle.
* Then press the "down" button (most right) for about 8 seconds.
* The Screen starts to blink and shows a WiFi-Icon.
* The thermostat is now an Access Point named `Thermostat-Beca-xxxxxx`. Default password is `12345678`
* Now you can fix network settings.
* Saving settings or pressing power button switches back to Station mode

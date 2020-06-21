## Notice
Modifying and flashing of devices is at your own risk. I'm not responsible for bricked or damaged devices. I strongly recommend a backup of original firmware before installing any other software.  
The thermostat is working independent from the Wifi-Module. That means, functionality of the thermostat itself will not and can't be changed. This firmware replaces only the communication part of the thermostat, which is handled by the ESP module. The firmware will partially work with other devices too. The Tuya devices has a serial communication standard (MCU commands) which is only different in parameters. Unknown commands will be forwarded to the MQTT server.

## Choose your way
There are 2 options to flash the firmware on device:
1. Use tuya-convert for flashing. Tested multiple times with success on all devices!
2. Flashing device manually: Unplug and open the device, wire 4 cables and connect it to a programmer for ESP8266 
3. Uploading WThermostat if you already have installed tasmota or another Free Firmware

## Download pre-built binaries

Pre-Built binaries can be downloaded at <a href="https://github.com/fashberg/WThermostatBeca/releases">releases-page</a>.

# Option 1: Use tuya-convert
This method does not require any kind of soldering or disassembly of the device.
You should be able to flash Beca Thermostats (BHT-002 and BHT-6000 also) with [tuya-convert](https://github.com/ct-Open-Source/tuya-convert).  
Follow the steps [here](https://github.com/ct-Open-Source/tuya-convert#procedure) to upload the firmware to your thermostat.  
Download the wthermostat-1.xx-fas.bin binary from https://github.com/fashberg/WThermostatBeca/releases and place it in the ```/files/``` folder before starting the flash procedure. 

Video of this procedure: https://youtu.be/fqfghJqnK_8

## Restore original Firmware: ##
The Backup file dumped by tuya-convert is 1 MB large (whole flash of ESP-Chip), which is too big to be flashed OTA (over the air). So you are not able to flash back to original Firmware without soldering!
See: https://github.com/ct-Open-Source/tuya-convert/issues/113 

# Option 2: Flashing device manually
## 1. Check your device
Compatible devices looks inside like this. On the right you can see the ESP8266 module (TYWE3S)

![thermostat inside](docs/bac-002-wifi-inside.jpg)

## 2. Connection to device for flashing
There are many ways to get the physical connection to ESP module. I soldered the connections on the device for flashing. Maybe there is a more elegant way to do that. It's quite the same, if you try to flash any other Sonoff devices to Tasmota. So get the inspiration for flashing there: https://github.com/arendst/Sonoff-Tasmota/wiki

Following connections were working for me (refer to ESP-12E pinout):
- Red: ESP-VCC and ESP-EN connected to Programmer-VCC (3.3V) 
- Black: ESP-GND and ESP-GPIO15 connected to Programmer-GND
- Green: ESP-RX connected to Programmer-TX
- Yellow: ESP-TX connected to Programmer-RX
- Blue right: ESP-GPIO0, must be connected with GND during power up
- Blue left: ESP-Reset, connect to GND to restart the ESP

![Flashing connection](docs/Flashing_Tywe3S_Detail.jpg)

## 3. Remove the power supply from thermostat during all flashing steps
Flasing will fail, if the thermostat is still powered during this operation.
## 4. Backup the original firmware
Don't skip this. In case of malfunction you need the original firmware. Tasmota has also a great tutorial for the right esptool commands: https://github.com/arendst/Sonoff-Tasmota/wiki/Esptool. So the backup command is:

```esptool.py -p <yourCOMport> -b 460800 read_flash 0x00000 0x100000 originalFirmware1M.bin```

for example:

```esptool.py -p /dev/ttyUSB0 -b 460800 read_flash 0x00000 0x100000 originalFirmware1M.bin```

## 5. Upload new firmware
Get the ESP in programming mode first.
Erase flash:

```esptool.py -p /dev/ttyUSB0 erase_flash```

After erasing the flash, get the ESP in programming mode again. 
Write firmware (1MB)

```esptool.py -p /dev/ttyUSB0 write_flash -fs 1MB 0x0 WThermostat_x.xx.bin```

# Option 3: Flashing Tasmota or other Free Firmware
If you already have tasmota installed or any other free ESP alternative you can upgrade easyly using the "Firmware Upgrade" function.
If you get "Upload Failed - Not compatible" you have to set the following option in tasmota console before you start the upgrade:
```SetOption78 1```

All tasmota settings will get lost, also the network configuration!
If you run Klaus Ahrenbergs WThermostat >= 1.09 then network settings will be kept on Firmware Upgrade.

# After the Upgrade
If you have installed WThermostat you have to configure it - see <a href="Configuration.md">Configuration.md</a>
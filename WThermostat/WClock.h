
/*********************************************************************************************\
*  TimeZone Handling based on
*        Time by Michael Margolis and Paul Stoffregen (https://github.com/PaulStoffregen/Time)
*        Timezone by Jack Christensen (https://github.com/JChristensen/Timezone)
*        Tasmota by Theo Arends (https://github.com/arendst/Tasmota)
\*********************************************************************************************/

#ifndef __WCLOCK_H__
#define __WCLOCK_H__

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#include "../lib/WAdapter/WAdapter/WDevice.h"
#include "../lib/WAdapter/WAdapter/WNetwork.h"
#include "Arduino.h"

//#define NTPTESTSTART 1585555170 // Full Hour Test
//#define NTPTESTSTART (1585447200 - 70 - (1*60*60)) // DST Test Berlin
//#define NTPTESTSTART (1603594800 - 70 - (2*60*60)) // STD Test Berlin
//#define NTPTESTSTART (1586055600 - 70 - (11*60*60)) // STD Test Sydney
//#define NTPTESTSTART (1601776800 - 70 - (10*60*60)) // DST Test Sydney

const static char* DEFAULT_NTP_SERVER PROGMEM = "pool.ntp.org";
const static char* DEFAULT_TIME_ZONE PROGMEM = "99";
const static char* DEFAULT_TIME_DST PROGMEM = "0,3,0,2,120";
const static char* DEFAULT_TIME_STD PROGMEM = "0,10,0,3,60";

const static char HTTP_TEXT_CLOCK_HOWTO[] PROGMEM = R"=====(
<div style="max-width: 400px">
See <a href="https://github.com/fashberg/WThermostatBeca/blob/master/Configuration.md#4-configure-clock-settings"
target="_blank">github.com/fashberg/WThermostatBeca/blob/master/Configuration.md</a> 
for Howto set Timezone and Daylight-Savings
</div>
)=====";

typedef struct {
    bool initialized;
    uint16_t week;   // bits 1 - 3   = 0=Last week of the month, 1=First, 2=Second, 3=Third, 4=Fourth
    uint16_t month;  // bits 4 - 7   = 1=Jan, 2=Feb, ... 12=Dec
    uint16_t dow;    // bits 8 - 10  = day of week, 1=Sun, 2=Mon, ... 7=Sat
    uint16_t hour;   // bits 11 - 15 = 0-23
    int16_t offset;
} TimeRule;

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_of_week;  // sunday is day 1
    uint8_t day_of_month;
    uint8_t month;
    char name_of_month[4];
    uint16_t day_of_year;
    uint16_t year;
    unsigned long days;
    unsigned long valid;
} TIME_T;

typedef struct {
    uint32_t utc_time = 0;
    uint32_t local_time = 0;
    uint32_t daylight_saving_time = 0;
    uint32_t standard_time = 0;
    uint32_t ntp_time = 0;
    uint32_t midnight = 0;
    uint32_t restart_time = 0;
    int32_t time_timezone = 0;
    uint8_t ntp_sync_minute = 0;
    bool midnight_now = false;
    bool user_time_entry = false;               // Override NTP by user setting
} RTC;

#define LEAP_YEAR(Y) ((( Y) > 0) && !((Y) % 4) && (((Y) % 100) || !((Y) % 400)))

static const uint8_t kDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  // API starts months from 1, this array starts from 0
static const char kMonthNames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
const uint32_t START_VALID_TIME = 1451602800;  // Time is synced and after 2016-01-01

const char* ID_CLOCK PROGMEM = "clock";
const char* NAME_CLOCK PROGMEM = "Clock";


const char* PROP_NTPSERVER PROGMEM = "ntpServer";
const char* PROP_TIMEZONE PROGMEM = "timeZone";
const char* PROP_TIMEDST PROGMEM = "timeDST";
const char* PROP_TIMESTD PROGMEM = "timeSTD";

const char* PROP_EPOCHTIME PROGMEM = "epochTime";
const char* PROP_EPOCHTIMELOCALFORMATTED PROGMEM = "epochTimeLocalFormatted";
const char* PROP_VALIDTIME PROGMEM = "validTime";
const char* PROP_OFFSET PROGMEM = "offset";
const char* PROP_UPTIME PROGMEM = "uptime";

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);

class WClock : public WDevice {
   public:
    typedef std::function<void(void)> THandlerFunction;
    typedef std::function<void(const char*)> TErrorHandlerFunction;

    //WClock(bool debug, WNetwork *network) {
    WClock(WNetwork* network, String applicationName)
        : WDevice(network, ID_CLOCK, NAME_CLOCK, network->getIdx(), DEVICE_TYPE_TEXT_DISPLAY) {
        this->mainDevice = false;
        this->visibility = MQTT;

        if (network->getSettingsOld()){
            if (network->getSettingsOld()->getNetworkSettingsVersion()==NETWORKSETTINGS_PRE_FAS114
             || network->getSettingsOld()->getApplicationSettingsVersion()==network->getSettingsOld()->getApplicationSettingsCurrent()-1){
                network->log()->notice(F("Reading WClockSettings PRE_FAS114/FLAG_OPTIONS_APPLICATION-1"));
                network->getSettingsOld()->setString(PROP_NTPSERVER, 32, DEFAULT_NTP_SERVER);
                network->getSettingsOld()->setString(PROP_TIMEZONE, 6, DEFAULT_TIME_ZONE);
                network->getSettingsOld()->setString(PROP_TIMEDST, 14, DEFAULT_TIME_DST);
                network->getSettingsOld()->setString(PROP_TIMESTD, 14, DEFAULT_TIME_STD);
            }
        }
        this->ntpServer = network->getSettings()->setString(PROP_NTPSERVER, 32,
            (network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_NTPSERVER) ? network->getSettingsOld()->getString(PROP_NTPSERVER) : DEFAULT_NTP_SERVER));
        this->ntpServer->setReadOnly(true);
        this->ntpServer->setVisibility(MQTT);
        this->addProperty(ntpServer);
        this->timeZoneConfig = network->getSettings()->setString(PROP_TIMEZONE, 6,
         (network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_TIMEZONE) ? network->getSettingsOld()->getString(PROP_TIMEZONE) : DEFAULT_TIME_ZONE));
        // upgrade hack
        String tzval=(String)this->timeZoneConfig->c_str();
        if (tzval.startsWith("http://")){
            network->log()->warning(PSTR("Changing old http Timezone to default"));
            this->timeZoneConfig->setString(DEFAULT_TIME_ZONE);
        }
        this->timeZoneConfig->setReadOnly(true);
        this->timeZoneConfig->setVisibility(MQTT);
        this->addProperty(timeZoneConfig);
        this->timeDST = network->getSettings()->setString(PROP_TIMEDST, 14,
            (network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_TIMEDST) ? network->getSettingsOld()->getString(PROP_TIMEDST) : DEFAULT_TIME_DST));
        this->timeDST->setReadOnly(true);
        this->timeDST->setVisibility(MQTT);
        this->addProperty(timeDST);
        this->timeSTD = network->getSettings()->setString(PROP_TIMESTD, 14,
            (network->getSettingsOld() && network->getSettingsOld()->existsSetting(PROP_TIMESTD) ? network->getSettingsOld()->getString(PROP_TIMESTD) : DEFAULT_TIME_STD));
        this->timeSTD->setReadOnly(true);
        this->timeSTD->setVisibility(MQTT);
        this->addProperty(timeSTD);

        parseTimeZone(this->timeZoneConfig->c_str(), &this->timezone_hours, &this->timezone_minutes);
        if (this->timezone_hours == 99) {
            parseTimeRule(this->timeDST->c_str(), &this->timeRuleDst);
            parseTimeRule(this->timeSTD->c_str(), &this->timeRuleStd);
        } else {
            resetTimeRule(&this->timeRuleStd);
            resetTimeRule(&this->timeRuleDst);
        }

        network->log()->notice(PSTR("TimeZoneAndRules: TZ:%s, DST:%s, STD:%s"),
                               getTimeZoneIntegerToString(this->timezone_hours, this->timezone_minutes).c_str(),
                               getTimeRuleToString(this->timeRuleDst).c_str(),
                               getTimeRuleToString(this->timeRuleStd).c_str()
                               );


        Rtc.utc_time = 0;
        BreakTime(Rtc.utc_time, RtcTime);

        this->epochTime = new WLongProperty(PROP_EPOCHTIME);
        this->epochTime->setReadOnly(true);
        this->epochTime->setOnValueRequest([this](WProperty* p) {
            p->setLong(getEpochTime());
        });
        this->addProperty(epochTime);
        this->epochTimeLocalFormatted = new WStringProperty(PROP_EPOCHTIMELOCALFORMATTED, nullptr, 20);
        this->epochTimeLocalFormatted->setReadOnly(true);
        this->epochTimeLocalFormatted->setOnValueRequest([this](WProperty* p) { updateLocalFormattedTime(); });
        this->addProperty(epochTimeLocalFormatted);
        this->validTime = new WOnOffProperty(PROP_VALIDTIME, nullptr);
        this->validTime->setBoolean(false);
        this->validTime->setReadOnly(true);
        this->addProperty(validTime);
        this->offset = new WIntegerProperty(PROP_OFFSET, nullptr);
        this->offset->setInteger(0);
        this->offset->setReadOnly(true);
        this->addProperty(offset);

        this->uptime = new WLongProperty(PROP_UPTIME);
        this->uptime->setReadOnly(true);
        this->uptime->setOnValueRequest([this](WProperty* p) {
            p->setLong(getUptime());
        });
        this->addProperty(uptime);
        lastTry = lastNtpSync = ntpTime =  lastRun, lastDiff = 0;

        ntpClient = NTPClient(ntpUDP, ntpServer->c_str());
    }

    void loop(unsigned long now) {
        TIME_T tmpTime;
        const int ntpRetryMinutes = 1;
        const int ntpResyncMinutes = 5;
        const int ntpInvalidateHours = 48;
        bool notify=false;
        //Invalid after 48 hours failed sync
        if (isValidTime() && !(now - lastNtpSync < (ntpInvalidateHours * 60 * 60 * 1000) )){
            network->log()->error(F("No valid NTP Time since %d hours -> invalidating"), ntpInvalidateHours);
        }
        validTime->setBoolean((lastNtpSync > 0) && (now - lastNtpSync < (ntpInvalidateHours * 60 * 60 * 1000)));

        Rtc.utc_time=getEpochTime();
        // calc retry - faster 3 minutes after boot
        unsigned long nextSync=(lastTry > lastNtpSync  
            ? lastTry+(ntpRetryMinutes * ( now < 3 * 60 * 1000 ? 20 : 60) * 1000)
            : lastNtpSync + (ntpResyncMinutes * 60 * 1000));
        if ((
                (!isValidTime() && lastTry == 0) 
                || (now > nextSync) 
            ) && (WiFi.status() == WL_CONNECTED)) {
            network->log()->trace(F("Time via NTP server '%s'"), ntpServer->c_str());
            network->logHeap(PSTR("NTP Start"));
            lastTry = now;
            if (ntpClient.update()) {
                network->logHeap(PSTR("NTP GotTime"));
                uint32_t oldutc = getEpochTime();
                ntpTime = ntpClient.getEpochTime();
                #ifdef NTPTESTSTART
                ntpTime = NTPTESTSTART + (millis()/1000);
                #endif
                if (ntpTime > START_VALID_TIME) {
                    if (validTime->getBoolean()){
                        // don't save first diff
                        lastDiff=ntpTime - oldutc;
                    }
                    Rtc.utc_time = ntpTime;
                    BreakTime(ntpTime, tmpTime);
                    RtcTime.year = tmpTime.year;
                    Rtc.daylight_saving_time = RuleToTime(this->timeRuleDst, RtcTime.year);
                    Rtc.standard_time = RuleToTime(this->timeRuleStd, RtcTime.year);
                    network->log()->notice(F("NTP time synced: (%04d-%02d-%02d %02d:%02d:%02d, Weekday: %d, Epoch: %d, Dst: %d, Std: %d, OldUTC: %d, ntpTime: %d, Diff: %d, Uptime: %d)"),
                            tmpTime.year, tmpTime.month, tmpTime.day_of_month, tmpTime.hour, tmpTime.minute, tmpTime.second, tmpTime.day_of_week,
                            ntpTime, Rtc.daylight_saving_time, Rtc.standard_time, oldutc, ntpTime, lastDiff, getUptime() );

                    validTime->setBoolean(true);
                    notify=true;
                    lastNtpSync = now;
                } else {
                    network->log()->error(F("NTP reports invalid time. (%d)"), ntpTime);
                }
            } else {
                network->log()->warning(F("NTP sync failed. "));
            }
                 
        }

        // -------------
        if (!notify && lastRun > now-1000) return;
        lastRun=now;

        //network->log()->verbose(F("Clock: epoch: %d,timezone_diff: %d, millis: %d, nextSync: %d, lastTry: %d, lastSync: %d"),
        //    getEpochTime(), Rtc.time_timezone, now, nextSync, lastTry, lastNtpSync);

        Rtc.local_time = Rtc.utc_time + Rtc.time_timezone;
        BreakTime(Rtc.local_time, RtcTime);

        if (notify || (RtcTime.minute==0 && RtcTime.second==0)){
            if (Rtc.local_time > START_VALID_TIME) {  // 2016-01-01
                network->log()->trace(F("Clock Recalc Timezone %d"),  Rtc.local_time);
                int32_t newTz=Rtc.time_timezone;
                if (this->timezone_hours!= 99) {
                    newTz = (this->timezone_hours * SECS_PER_HOUR) + (this->timezone_minutes * SECS_PER_MIN * (this->timezone_hours < 0 ? -1 : 1));

                } else {
                    int32_t dstoffset = this->timeRuleDst.offset * SECS_PER_MIN;
                    int32_t stdoffset = this->timeRuleStd.offset * SECS_PER_MIN;
                    
                    if (Rtc.standard_time < Rtc.daylight_saving_time) { 
                        // Southern hemisphere
                        network->log()->trace(F("This year switch to DST/STD: %d / %d (Southern)"),
                        Rtc.daylight_saving_time, Rtc.standard_time);
                        if ((Rtc.utc_time >= (Rtc.standard_time - dstoffset)) && (Rtc.utc_time < (Rtc.daylight_saving_time - stdoffset))) {
                            newTz = stdoffset;  // Standard Time
                        } else {
                            newTz = dstoffset;  // Daylight Saving Time
                        }
                    } else {
                        // Northern hemisphere
                        network->log()->trace(F("This year switch to DST/STD: %d / %d (Northern)"),
                        Rtc.standard_time, Rtc.daylight_saving_time);
                        if ((Rtc.utc_time >= (Rtc.daylight_saving_time - stdoffset)) && (Rtc.utc_time < (Rtc.standard_time - dstoffset))) {
                            newTz = dstoffset;  // Daylight Saving Time
                        } else {
                            newTz = stdoffset;  // Standard Time
                        }
                    }
                }
                if (Rtc.time_timezone!=newTz){
                    network->log()->notice(F("TZ Change %d -> %d"), Rtc.time_timezone, newTz);
                    Rtc.time_timezone = newTz;
                    notify=true;
                    Rtc.local_time = Rtc.utc_time + Rtc.time_timezone;
                    BreakTime(Rtc.local_time, RtcTime);
                } 
            }
        }

        /* 
        if (RtcTime.valid) {
            if (!Rtc.midnight) {
                Rtc.midnight = Rtc.local_time - (RtcTime.hour * 3600) - (RtcTime.minute * 60) - RtcTime.second;
            }
            if (!RtcTime.hour && !RtcTime.minute && !RtcTime.second) {
                Rtc.midnight = Rtc.local_time;
                Rtc.midnight_now = true;
            }
        }
        */

        if (notify){
            this->offset->setReadOnly(false);
            this->offset->setInteger(Rtc.time_timezone);
            this->offset->setReadOnly(true);
            network->log()->notice(F("NotifyNewDateTime"));
            notifyOnTimeUpdate();
        }
    }

    void setOnTimeUpdate(THandlerFunction onTimeUpdate) {
        this->onTimeUpdate = onTimeUpdate;
    }

    void setOnError(TErrorHandlerFunction onError) {
        this->onError = onError;
    }

    unsigned long getEpochTime() {
        #ifdef NTPTESTSTART
        return NTPTESTSTART + (millis()/1000);
        #endif
        return (lastNtpSync > 0 ? ntpClient.getEpochTime() : 0);
    }

    unsigned long getEpochTimeLocal() {
        #ifdef NTPTESTSTART
        return (lastNtpSync > 0 ? NTPTESTSTART + (millis()/1000) + getOffset() : 0);
        #endif
        return (lastNtpSync > 0 ? ntpClient.getEpochTime() + getOffset() : 0);
    }

    unsigned long getUptime() {
        return millis() / 1000;
    }

    byte getWeekDay() {
        return RtcTime.day_of_week -1;
    }

    byte getWeekDay(unsigned long epochTime) {
        //weekday from 0 to 6, 0 is Sunday
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        return tmpTime.day_of_week -1;
    }

    byte getHours() {
        return RtcTime.hour;
    }

    byte getHours(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        return tmpTime.hour;
    }

    byte getMinutes() {
        return RtcTime.minute;
    }

    byte getMinutes(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        return tmpTime.minute;
    }

    byte getSeconds() {
        return RtcTime.second;
    }

    byte getSeconds(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        return tmpTime.second;
    }

    int getYear() {
        return RtcTime.year;
    }

    int getYear(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        return tmpTime.year;
    }

    byte getMonth() {
        return RtcTime.month;
    }

    byte getMonth(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        //month from 1 to 12
        return tmpTime.month;
    }

    byte getDay() {
        return RtcTime.day_of_month;
    }

    byte getDay(unsigned long epochTime) {
        TIME_T tmpTime;
        BreakTime(epochTime, tmpTime);
        //day from 1 to 31
        return tmpTime.day_of_month;
    }

    bool isTimeLaterThan(byte hours, byte minutes) {
        return ((getHours() > hours) || ((getHours() == hours) && (getMinutes() >= minutes)));
    }

    bool isTimeEarlierThan(byte hours, byte minutes) {
        return ((getHours() < hours) || ((getHours() == hours) && (getMinutes() < minutes)));
    }

    bool isTimeBetween(byte hours1, byte minutes1, byte hours2, byte minutes2) {
        return ((isTimeLaterThan(hours1, minutes1)) && (isTimeEarlierThan(hours2, minutes2)));
    }

    void updateLocalFormattedTime() {
        updateFormattedTimeImpl(getEpochTimeLocal());
    }

    void updateFormattedTimeImpl(unsigned long rawTime) {
        WStringStream* stream = new WStringStream(19);
        char buffer[5];
        //year
        int _year = year(rawTime);
        itoa(_year, buffer, 10);
        stream->print(buffer);
        stream->print("-");
        //month
        uint8_t _month = month(rawTime);
        if (_month < 10) stream->print("0");
        itoa(_month, buffer, 10);
        stream->print(buffer);
        stream->print("-");
        //month
        uint8_t _day = day(rawTime);
        if (_day < 10) stream->print("0");
        itoa(_day, buffer, 10);
        stream->print(buffer);
        stream->print(" ");
        //hours
        unsigned long _hours = (rawTime % 86400L) / 3600;
        if (_hours < 10) stream->print("0");
        itoa(_hours, buffer, 10);
        stream->print(buffer);
        stream->print(":");
        //minutes
        unsigned long _minutes = (rawTime % 3600) / 60;
        if (_minutes < 10) stream->print("0");
        itoa(_minutes, buffer, 10);
        stream->print(buffer);
        stream->print(":");
        //seconds
        unsigned long _seconds = rawTime % 60;
        if (_seconds < 10) stream->print("0");
        itoa(_seconds, buffer, 10);
        stream->print(buffer);

        epochTimeLocalFormatted->setString(stream->c_str());
        delete stream;
    }

    bool isValidTime() {
        return validTime->getBoolean();
    }

    bool isClockSynced() {
        return (lastNtpSync > 0);
    }

    long getOffset() {
        return offset->getInteger();
    }

    void printConfigPage(AsyncWebServerRequest *request, AsyncResponseStream* page) {
        network->log()->notice(F("Clock config page"));
        page->printf_P(HTTP_CONFIG_PAGE_BEGIN, getId());
        page->printf_P(HTTP_TEXT_FIELD, "NTP server:", "ntp", 32, ntpServer->c_str());
        page->printf_P(HTTP_TEXT_FIELD, "Timezone:", "tzone", 6, timeZoneConfig->c_str());
        page->printf_P(HTTP_TEXT_FIELD, "TimeRule switch to Daylight Saving Time:", "tdst", 14, timeDST->c_str());
        page->printf_P(HTTP_TEXT_FIELD, "TimeRule switch Back to Standard Time:", "tstd", 14, timeSTD->c_str());
        page->printf_P(HTTP_TEXT_CLOCK_HOWTO);
        page->printf_P(HTTP_CONFIG_SAVEANDREBOOT_BUTTON);
        page->printf_P(HTTP_HOME_BUTTON);
    }

    void saveConfigPage(AsyncWebServerRequest *request) {
        network->log()->notice(F("Save clock config page"));
        this->ntpServer->setString(getValueOrEmpty(request, "ntp").c_str());
        this->timeZoneConfig->setString(getValueOrEmpty(request, "tzone").c_str());
        this->timeDST->setString(getValueOrEmpty(request, "tdst").c_str());
        this->timeSTD->setString(getValueOrEmpty(request, "tstd").c_str());
        network->log()->notice(F("Save clock config page DONE"));
        /* reboot follows */
    }

   private:
    THandlerFunction onTimeUpdate;
    TErrorHandlerFunction onError;
    unsigned long lastTry, lastNtpSync, ntpTime, lastRun;
    long lastDiff;
    WProperty* epochTime;
    WProperty* epochTimeLocalFormatted;
    WProperty* validTime;
    WProperty* uptime;
    WProperty* ntpServer;
    WProperty* timeZoneConfig;
    WProperty* timeDST;
    WProperty* timeSTD;
    WProperty* offset;

    TimeRule timeRuleStd;
    TimeRule timeRuleDst;
    int16_t timezone_hours;
    int16_t timezone_minutes;

    RTC Rtc;
    TIME_T RtcTime;



    void notifyOnTimeUpdate() {
        if (onTimeUpdate) {
            onTimeUpdate();
        }
    }

    void notifyOnError(const char* error) {
        network->log()->notice(error);
        if (onError) {
            onError(error);
        }
    }

    char* Trim(char* p) {
        while ((*p != '\0') && isblank(*p)) {
            p++;
        }  // Trim leading spaces
        char* q = p + strlen(p) - 1;
        while ((q >= p) && isblank(*q)) {
            q--;
        }  // Trim trailing spaces
        q++;
        *q = '\0';
        return p;
    }

    void resetTimeRule(TimeRule* tr) {
        tr->week = 0;
        tr->month = 0;
        tr->dow = 0;
        tr->hour = 0;
        tr->offset = 0;
        tr->initialized = false;
    }

    void parseTimeRule(const char* ts, TimeRule* tr) {
        resetTimeRule(tr);
        // TimeStd 0/1/2/3/4, 1..12, 1..7, 0..23, +/-780
        if (strlen(ts)) {
            uint32_t tpos = 0;  // Parameter index
            int value = 0;
            char* p = (char*)ts;  // Parameters like "1, 2,3 , 4 ,5, -120" or ",,,,,+240"
            char* q = p;          // Value entered flag
            while (p && (tpos < 7)) {
                if (p > q) {  // Any value entered
                    if (1 == tpos) {
                        tr->week = (value < 0) ? 0 : (value > 4) ? 4 : value;
                    }
                    if (2 == tpos) {
                        tr->month = (value < 1) ? 1 : (value > 12) ? 12 : value;
                    }
                    if (3 == tpos) {
                        tr->dow = (value < 1) ? 1 : (value > 7) ? 7 : value;
                    }
                    if (4 == tpos) {
                        tr->hour = (value < 0) ? 0 : (value > 23) ? 23 : value;
                    }
                    if (5 == tpos) {
                        tr->offset = (value < -900) ? -900 : (value > 900) ? 900 : value;
                    }
                }
                p = Trim(p);  // Skip spaces
                if (tpos && (*p == ',')) {
                    p++;
                }             // Skip separator
                p = Trim(p);  // Skip spaces
                q = p;        // Reset any value entered flag
                value = strtol(p, &p, 10);
                tpos++;  // Next parameter
            }
            tr->initialized = true;
        }
    }

    String getTimeRuleToString(TimeRule tr) {
        char buf[64];
        size_t size = sizeof buf;
        snprintf_P(buf, size, PSTR("\"Week\":%d,\"Month\":%d,\"Day\":%d,\"Hour\":%d,\"Offset\":%d"),
                   tr.week, tr.month, tr.dow, tr.hour, tr.offset);
        return (String)buf;
    }

    String getTimeZoneIntegerToString(int16_t tz_h, int16_t tz_m) {
        /*
        int8_t tz_h = tz / 60;
        int8_t tz_m = abs(tz % 60);
        */
        char buf[7];  //+13:30\0
        size_t size = sizeof buf;
        if (tz_h==99){
            snprintf_P(buf, size, PSTR("DSTSTD"));
        } else {
            snprintf_P(buf, size, PSTR("%+03d:%02d"), tz_h, tz_m);
        }
        return (String)buf;
    }

    void parseTimeZone(const char* ts, int16_t *tz_h_param, int16_t *tz_m_param){
        int16_t tz_h = 0;
        int16_t tz_m = 0;
        char* data = strdup(ts);
        if (strlen(ts)) {
            char* p = strtok(data, ":");

            tz_h = strtol(p, nullptr, 10);
            if (tz_h > 13 || tz_h < -13) {
                tz_h = 99;
            }
            if (p) {
                p = strtok(nullptr, ":");
                if (p) {
                    tz_m = strtol(p, nullptr, 10);
                    if (tz_m > 59) {
                        tz_m = 59;
                    } else if (tz_m < 0) {
                        tz_m = 0;
                    }
                }
            }
        } else {
            tz_h = 99;
            tz_m = 0;
        }
        *tz_h_param=tz_h;
        *tz_m_param=tz_m;
        //return (tz_h * 60) + (tz_h >= 0 ? tz_m : tz_m * -1);
    }

    void BreakTime(uint32_t time_input, TIME_T& tm) {
        // break the given time_input into time components
        // this is a more compact version of the C library localtime function
        uint8_t year;
        uint8_t month;
        uint8_t month_length;
        uint32_t time;
        uint32_t days;

        time = time_input;
        tm.second = time % 60;
        time /= 60;  // now it is minutes
        tm.minute = time % 60;
        time /= 60;  // now it is hours
        tm.hour = time % 24;
        time /= 24;  // now it is days
        tm.days = time;
        tm.day_of_week = ((time + 4) % 7) + 1;  // Sunday is day 1

        year = 0;
        days = 0;
        while ((unsigned)(days += (LEAP_YEAR(year+1970) ? 366 : 365)) <= time) {
            year++;
        }
        tm.year = year+1970;  // year is offset from 1970

        days -= LEAP_YEAR(year) ? 366 : 365;
        time -= days;  // now it is days in this year, starting at 0
        tm.day_of_year = time;

        for (month = 0; month < 12; month++) {
            if (1 == month) {  // february
                if (LEAP_YEAR(year)) {
                    month_length = 29;
                } else {
                    month_length = 28;
                }
            } else {
                month_length = kDaysInMonth[month];
            }

            if (time >= month_length) {
                time -= month_length;
            } else {
                break;
            }
        }
        strlcpy(tm.name_of_month, kMonthNames + (month * 3), 4);
        tm.month = month + 1;                        // jan is month 1
        tm.day_of_month = time + 1;                  // day of month
        tm.valid = (time_input > START_VALID_TIME);  // 2016-01-01
    }

        uint32_t MakeTime(TIME_T& tm) {
        // assemble time elements into time_t

        int i;
        uint32_t seconds;

        // seconds from 1970 till 1 jan 00:00:00 of the given year
        seconds = (tm.year - 1970) * (SECS_PER_DAY * 365);
        for (i = 1970; i < tm.year; i++) {
            if (LEAP_YEAR(i)) {
                seconds += SECS_PER_DAY;  // add extra days for leap years
            }
        }

        // add days for this year, months start from 1
        for (i = 1; i < tm.month; i++) {
            if ((2 == i) && LEAP_YEAR(tm.year)) {
                seconds += SECS_PER_DAY * 29;
            } else {
                seconds += SECS_PER_DAY * kDaysInMonth[i - 1];  // monthDay array starts from 0
            }
        }
        seconds += (tm.day_of_month - 1) * SECS_PER_DAY;
        seconds += tm.hour * SECS_PER_HOUR;
        seconds += tm.minute * SECS_PER_MIN;
        seconds += tm.second;
        return seconds;
    }

    uint32_t RuleToTime(TimeRule r, int yr) {
        TIME_T tm;
        uint32_t t;
        uint8_t m;
        uint8_t w;  // temp copies of r.month and r.week

        m = r.month;
        w = r.week;
        if (0 == w) {        // Last week = 0
            if (++m > 12) {  // for "Last", go to the next month
                m = 1;
                yr++;
            }
            w = 1;  // and treat as first week of next month, subtract 7 days later
        }

        tm.hour = r.hour;
        tm.minute = 0;
        tm.second = 0;
        tm.day_of_month = 1;
        tm.month = m;
        tm.year = yr;
        t = MakeTime(tm);  // First day of the month, or first day of next month for "Last" rules
        BreakTime(t, tm);
        t += (7 * (w - 1) + (r.dow - tm.day_of_week + 7) % 7) * SECS_PER_DAY;
        if (0 == r.week) {
            t -= 7 * SECS_PER_DAY;  // back up a week if this is a "Last" rule
        }
        return t;
    }

    bool hasInfoPage() {
        return true;
    }

    void printInfoPage(AsyncResponseStream* page) {
        htmlTableRowTitle(page, F("Time-Valid:"));
        page->print((isValidTime()  ? "Yes" : "No"));
        htmlTableRowEnd(page);

        htmlTableRowTitle(page, F("Current local Time:"));
        page->print(epochTimeLocalFormatted->c_str());
        htmlTableRowEnd(page);

        htmlTableRowTitle(page, F("Current UTC-Offset:"));
        page->print(offset->getInteger());
        htmlTableRowEnd(page);

        page->print(F("<tr><th>Last-NTP-Sync Try:</th><td>"));
        if (lastTry){
            page->printf("%d Seconds ago", (millis()-lastTry)/1000);
        } else {
            page->print(F("Never"));
        }        
        htmlTableRowEnd(page);

        htmlTableRowTitle(page, F("Last-NTP-Sync with Success:"));
        if (lastNtpSync){
            page->printf("%d Seconds ago", (millis()-lastNtpSync)/1000);
        } else {
            page->print(F("Never"));
        }
        htmlTableRowEnd(page);

        htmlTableRowTitle(page, F("Last-NTP-Sync Delta:"));
        page->print(lastDiff);
        page->print(" sec");
        htmlTableRowEnd(page);
    }

};

#endif

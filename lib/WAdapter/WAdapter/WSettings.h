#ifndef W_SETTINGS_H
#define W_SETTINGS_H

#include "EEPROM.h"
#include "WLog.h"
#include "WProperty.h"

const byte STORED_FLAG_OLDLOW = 0x59; //1.00 ..
const byte STORED_FLAG_OLDHIGH = 0x63; //1.02
const byte STORED_FLAG_OLD = 0xF0; //FAS
const byte FLAG_OPTIONS_NETWORK = 0x64; //1.09
const int EEPROM_SIZE = 512;
unsigned int startAddressReadOffset = 2;
unsigned int startAddressSaveOffset = 2;


class WSettingItem {
public:
	WProperty* value;
	int address;
	bool networkSetting;
	WSettingItem* next = nullptr;
};
const int NETWORKSETTINGS_UNKNOWN = 0;
const int NETWORKSETTINGS_PRE_102 = 1;
const int NETWORKSETTINGS_PRE_109 = 2;
const int NETWORKSETTINGS_PRE_FAS114 = 3;
const int NETWORKSETTINGS_CURRENT = 4;


const int APPLICATIONSETTINGS_UNKNOWN = 0;

class WSettings {
public:
	WSettings(WLog* log, byte appSettingsFlag, bool compatMode) {
		this->log = log;
		this->appSettingsFlag = appSettingsFlag;
		this->addingNetworkSettings = true;
		this->_settingsNeedsUpdate=true;
		this->_compatMode=compatMode;
		EEPROM.begin(EEPROM_SIZE);
		uint8_t epromStored = EEPROM.read(0);
		this->log->trace(F("settings: old byte 0: 0x%02x"), epromStored);
		this->_existsSettingsNetwork = false;
		this->_existsSettingsApplication = false;		
		this->_networkSettingsVersion=NETWORKSETTINGS_UNKNOWN;
		this->_applicationSettingsVersion=APPLICATIONSETTINGS_UNKNOWN;
		if (epromStored==FLAG_OPTIONS_NETWORK){
			this->log->trace(F("settings: NetworksSettings found in Current Version"));
			// klaus >=1.09 and fas >=1.14-fas
			this->_existsSettingsNetwork = true;
			this->_networkSettingsVersion=NETWORKSETTINGS_CURRENT;
			uint8_t epromStoredApplication = EEPROM.read(1);
			this->log->trace(F("settings: old byte 1: 0x%02x"), epromStoredApplication);
			this->_applicationSettingsVersion=epromStoredApplication;
			if (epromStoredApplication == this->appSettingsFlag){
				this->log->trace(F("settings: ApplicationSettings found in Current Version"));
				this->_existsSettingsApplication = true;
				this->_settingsNeedsUpdate=false;
			}			
		} else if (compatMode){
			startAddressReadOffset=1;
			if (epromStored == STORED_FLAG_OLD){
				this->_existsSettingsNetwork = true;
				this->_existsSettingsApplication = true; // we support our old config
				this->_networkSettingsVersion=NETWORKSETTINGS_PRE_FAS114;
			} else if (epromStored >= STORED_FLAG_OLDLOW && epromStored <= STORED_FLAG_OLDHIGH){
				this->_existsSettingsNetwork = true;
				if (epromStored < 0x63) this->_networkSettingsVersion=NETWORKSETTINGS_PRE_102;
				else this->_networkSettingsVersion=NETWORKSETTINGS_PRE_109;
			}
		}

		this->log->trace(F("WSettings done, networkSettingsVersion: %d, app: %d"), this->getNetworkSettingsVersion(), this->getApplicationSettingsVersion());
		EEPROM.end();
	}

	void save(WProperty* property) {
		WSettingItem* settingItem = firstSetting;
		while (settingItem != nullptr) {
			if (property == settingItem->value) {
				EEPROM.begin(EEPROM_SIZE);
				save(settingItem);
				EEPROM.commit();
				EEPROM.end();
			}
			settingItem = settingItem->next;
		}
	}

	void save() {
		EEPROM.begin(EEPROM_SIZE);
		WSettingItem* settingItem = firstSetting;
		while (settingItem != nullptr) {
			save(settingItem);
			settingItem = settingItem->next;
		}
		//1. Byte - settingsStored flag
		EEPROM.write(0, FLAG_OPTIONS_NETWORK);
		EEPROM.write(1, this->appSettingsFlag);
		EEPROM.commit();
		EEPROM.end();
	}

	bool existsSetting(String id) {
		return (getSetting(id) != nullptr);
	}

	bool existsSettingsNetwork() {
		return this->_existsSettingsNetwork;
	}
	bool existsSettingsApplication() {
		return this->_existsSettingsApplication;
	}

	bool settingsNeedsUpdate() {
		return this->_settingsNeedsUpdate;
	}

	unsigned int getNetworkSettingsVersion(){
		return this->_networkSettingsVersion;
	}
	unsigned int getApplicationSettingsVersion(){
		return this->_applicationSettingsVersion;
	}
	unsigned int getApplicationSettingsCurrent(){
		return this->appSettingsFlag;
	}

	int getCurrentSettingsAddress(){
		return (this->lastSetting != nullptr ?  this->lastSetting->address : 0);
	}

	WProperty* getSetting(String id) {
		WSettingItem* settingItem = firstSetting;
		while (settingItem != nullptr) {
			WProperty* setting = settingItem->value;
			if (id.equals(setting->getId())) {
				return setting;
			}
			settingItem = settingItem->next;
		}
		return nullptr;
	}

	bool exists(WProperty* property) {
		WSettingItem* settingItem = firstSetting;
		while (settingItem != nullptr) {
			if (settingItem->value == property) {
				return true;
			}
			settingItem = settingItem->next;
		}
		return false;
	}

	void add(WProperty* property) {
		if (!exists(property)) {
			WSettingItem* settingItem = addSetting(property);
				if (((settingItem->networkSetting) && (this->existsSettingsNetwork())) ||
				((!settingItem->networkSetting) && (this->existsSettingsApplication() || this->_compatMode))) {

				log->trace(F("Loading setting: Address=%d, Id='%s', size=%d, MEM: %d"),
					settingItem->address + startAddressReadOffset, property->getId(), property->getLength(), EEPROM_SIZE);
				if (settingItem->address + startAddressReadOffset + property->getLength() > EEPROM_SIZE){
					log->error(F("Cannot add EPROM property. Size too small, Adrress=%d, Id='%s', size=%d, MEM: %d"),
					settingItem->address + startAddressReadOffset, property->getId(), property->getLength(), EEPROM_SIZE);
					return;
				}
				EEPROM.begin(EEPROM_SIZE);
				switch (property->getType()) {
				case BOOLEAN:
					property->setBoolean(EEPROM.read(settingItem->address + startAddressReadOffset) == 0xFF);
					break;
				case DOUBLE:
					double d;
					EEPROM.get(settingItem->address + startAddressReadOffset, d);
					property->setDouble(d);
					break;
				case INTEGER: {
					byte low, high;
					low = EEPROM.read(settingItem->address + startAddressReadOffset);
					high = EEPROM.read(settingItem->address + startAddressReadOffset + 1);
					int value = (low + ((high << 8)&0xFF00));
					property->setInteger(value);
					break;
				}				
				case LONG: {
					long four = EEPROM.read(settingItem->address + startAddressReadOffset);
					long three = EEPROM.read(settingItem->address + startAddressReadOffset + 1);
					long two = EEPROM.read(settingItem->address + 2);
					long one = EEPROM.read(settingItem->address + startAddressReadOffset + startAddressReadOffset + 3);
					long value = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
					property->setLong(value);
					break;
				}			
				case UNSIGNED_LONG: {
					long four = EEPROM.read(settingItem->address + startAddressReadOffset);
					long three = EEPROM.read(settingItem->address + startAddressReadOffset + 1);
					long two = EEPROM.read(settingItem->address + 2);
					long one = EEPROM.read(settingItem->address + startAddressReadOffset + startAddressReadOffset + 3);
					long value = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
					property->setUnsignedLong(value);
					break;
				}
				case BYTE:
					property->setByte(EEPROM.read(settingItem->address + startAddressReadOffset));
					break;
				case STRING:
					const char* rs = readString(settingItem->address + startAddressReadOffset, property->getLength());
					log->trace(F("String loaded: '%s'"), rs);
					property->setString(rs);
					delete rs;
					break;
				}
				EEPROM.end();
			} else {
				log->trace(F("NOT Loading setting %s (!current)"), property->getId());
			}
			property->setSettingsNotification([this](WProperty* property) {save(property);});
		}
	}

	bool getBoolean(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getBoolean() : false);
	}

	WProperty* setBoolean(const char* id, bool value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, BOOLEAN);
			setting->setBoolean(value);
			add(setting);
		} else {
			setting->setBoolean(value);
		}
		return setting;
	}

	byte getByte(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getByte() : 0x00);
	}

	WProperty* setByte(const char* id, byte value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, BYTE);
			setting->setByte(value);
			add(setting);
		} else {
			setting->setByte(value);
		}
		return setting;
	}

	int getInteger(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getInteger() : 0);
	}

	WProperty* setInteger(const char* id, int value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, INTEGER);
			setting->setInteger(value);
			add(setting);
		} else {
			setting->setInteger(value);
		}
		return setting;
	}
	
	long getLong(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getLong() : 0);
	}

	WProperty* setLong(const char* id, long value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, LONG);
			setting->setLong(value);
			add(setting);
		} else {
			setting->setLong(value);
		}
		return setting;
	}

	unsigned long getUnsignedLong(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getUnsignedLong() : 0);
	}

	WProperty* setUnsignedLong(const char* id, unsigned long value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, UNSIGNED_LONG);
			setting->setUnsignedLong(value);
			add(setting);
		} else {
			setting->setUnsignedLong(value);
		}
		return setting;
	}

	double getDouble(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->getDouble() : 0.0);
	}

	WProperty* setDouble(const char* id, double value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WProperty(id, id, DOUBLE);
			setting->setDouble(value);
			add(setting);
		} else {
			setting->setDouble(value);
		}
		return setting;
	}

	const char* getString(const char* id) {
		WProperty* setting = getSetting(id);
		return (setting != nullptr ? setting->c_str() : "");
	}


	WProperty* setString(const char* id, const char* value) {
		return setString(id, 32, value);
	}

	WProperty* setString(const char* id, byte length, const char* value) {
		WProperty* setting = getSetting(id);
		if (setting == nullptr) {
			setting = new WStringProperty(id, id, length);
			setting->setString(value);
			add(setting);
		} else  {
			setting->setString(value);
		}
		return setting;
	}



	void copyValueFrom(const char* id, WSettings* settings2) {
		WProperty * oldProp;
		if (settings2 == nullptr) return;
		oldProp=settings2->getSetting(id);
		if (oldProp==nullptr) return;
		//WProperty* setting = getSetting(id);
	}

	bool addingNetworkSettings;
protected:
	WSettingItem* addSetting(WProperty* setting) {
		WSettingItem* settingItem = new WSettingItem();
		settingItem->value = setting;
		settingItem->networkSetting = this->addingNetworkSettings;
		if (this->lastSetting == nullptr) {
			settingItem->address = 0;
			this->firstSetting = settingItem;
			this->lastSetting = settingItem;
		} else {
			settingItem->address = this->lastSetting->address + this->lastSetting->value->getLength();
			this->lastSetting->next = settingItem;
			this->lastSetting = settingItem;
		}
		return settingItem;
	}

	void save(WSettingItem* settingItem) {
		WProperty* setting = settingItem->value;
		if (settingItem->address  + startAddressSaveOffset + setting->getLength() > EEPROM_SIZE){
			log->error(F("Cannot save to EPROM. Size too small, Adrress=%d, Id='%s', size=%d, MEM: %d"),
				settingItem->address + startAddressSaveOffset, setting->getId(), setting->getLength(), EEPROM_SIZE);
			return;
		}
		switch (setting->getType()){
		case BOOLEAN:
			//log->notice(F("Save boolean to EEPROM: id='%s'; value=%d"), setting->getId(), setting->getBoolean());
			EEPROM.write(settingItem->address + startAddressSaveOffset, (setting->getBoolean() ? 0xFF : 0x00));
			break;
		case BYTE:
			EEPROM.write(settingItem->address + startAddressSaveOffset, setting->getByte());
			break;
		case INTEGER:
			byte low, high;
			low = (setting->getInteger() & 0xFF);
			high = ((setting->getInteger()>>8) & 0xFF);
			EEPROM.write(settingItem->address + startAddressSaveOffset, low);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 1, high);
			break;
		case LONG:
			byte l1, l2, l3, l4;
			l4 = (setting->getLong() & 0xFF);
			l3 = ((setting->getLong() >> 8) & 0xFF);
			l2 = ((setting->getLong() >> 16) & 0xFF);
			l1 = ((setting->getLong() >> 24) & 0xFF);
			EEPROM.write(settingItem->address + startAddressSaveOffset, l4);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 1, l3);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 2, l2);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 3, l1);
			break;
		case UNSIGNED_LONG:
			byte ul1, ul2, ul3, ul4;
			ul4 = (setting->getUnsignedLong() & 0xFF);
			ul3 = ((setting->getUnsignedLong() >> 8) & 0xFF);
			ul2 = ((setting->getUnsignedLong() >> 16) & 0xFF);
			ul1 = ((setting->getUnsignedLong() >> 24) & 0xFF);
			EEPROM.write(settingItem->address + startAddressSaveOffset, ul4);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 1, ul3);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 2, ul2);
			EEPROM.write(settingItem->address + startAddressSaveOffset + 3, ul1);
			break;
		case DOUBLE:
			EEPROM.put(settingItem->address + startAddressSaveOffset, setting->getDouble());
			break;
		case STRING:
			writeString(settingItem->address + startAddressSaveOffset, setting->getLength(), setting->c_str());
			break;
		}

	}

private:
	WLog* log;
	byte appSettingsFlag;
	bool _existsSettingsNetwork, _existsSettingsApplication;
	bool _settingsNeedsUpdate;
	bool _compatMode;
	unsigned int _networkSettingsVersion;
	unsigned int _applicationSettingsVersion;
	WSettingItem* firstSetting = nullptr;
	WSettingItem* lastSetting = nullptr;

	const char* readString(int address, int length) {
		char* data = new char[length+ 1]; //Max 100 Bytes
		int i = 0;
		for (i = 0; i <= length; i++) {
			if (i < length) {
				byte k = EEPROM.read(address + i);
				data[i] = k;
				if (k == '\0') {
					break;
				}
			} else {
				data[i] = '\0';
			}
		}
		return data;
	}

	void writeString(int address, int length, const char* value) {
		int size = strlen(value);
		if (size > length) {
			size = length;
		}
		for (int i = 0; i < size; i++) {
			EEPROM.write(address + i, value[i]);
		}
		if (size < length) {
			EEPROM.write(address + size, '\0');
		}
	}

};

#endif

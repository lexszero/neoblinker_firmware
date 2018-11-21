#include <EEPROM.h>
#include "core.h"

Config config;

enum ConfigParamId {
#define CONFIG_PARAM(type, name, ...) __cat(ConfigParam_, name),
#define CONFIG_PARAM_STR(name, ...) CONFIG_PARAM(_, name)
	CONFIG_PARAMS
#undef CONFIG_PARAM
#undef CONFIG_PARAM_STR
	_ConfigParamCount
};

static const uint16_t CONFIG_VERSION = (_ConfigParamCount << 8);
static const uint32_t CONFIG_MAGIC =   (0xdead << 16) | CONFIG_VERSION;

void Config::load() {
	size_t offset = EEPROM_OFFSET;
	uint32_t magic;

	log("loading config");
	offset += eeprom_read(offset, &magic, sizeof(magic));
	if (magic == CONFIG_MAGIC) {
		log("magic ok, version %d", magic & 0xFF);

		#define CONFIG_PARAM(type, name, defval, desc) \
			offset += eeprom_read(offset, &config.name, sizeof(config.name)); \
			logval(desc, config.name);
		#define CONFIG_PARAM_STR(name, len, defval, desc) \
			offset += eeprom_read(offset, config.name, len); \
			logval(desc, config.name);
		CONFIG_PARAMS
		#undef CONFIG_PARAM
		#undef CONFIG_PARAM_STR
	}
	else {
		log("bad magic 0x%x, saving default config", magic);
		Config::save();
	}
}

void Config::save() {
	size_t offset = EEPROM_OFFSET;
	log("saving config");

	offset += eeprom_write(offset, &CONFIG_MAGIC, sizeof(CONFIG_MAGIC));
	#define CONFIG_PARAM(type, name, defval, desc) \
		offset += eeprom_write(offset, &config.name, sizeof(config.name));
	#define CONFIG_PARAM_STR(name, len, defval, desc) \
		offset += eeprom_write_str(offset, config.name, len);
	CONFIG_PARAMS
	#undef CONFIG_PARAM
	#undef CONFIG_PARAM_STR

	EEPROM.commit();
}

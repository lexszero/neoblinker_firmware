#include <EEPROM.h>
#include "utils.h"

size_t eeprom_read(int offset, void *buf, size_t len) {
	for (size_t i = 0; i < len; i++) {
		((uint8_t *)buf)[i] = EEPROM.read(i + offset);
	}
	return len;
}

size_t eeprom_write(int offset, const void *buf, size_t len) {
	for (size_t i = 0; i < len; i++) {
		EEPROM.write(i + offset, ((uint8_t *)buf)[i]);
	}
	return len;
}

size_t eeprom_write_str(int offset, const char *buf, size_t len) {
	int slen = buf ? strlen(buf) : 0;
	eeprom_write(offset, buf, slen);
	for (size_t i = slen; i < len; i++) {
		EEPROM.write(i + offset, 0);
	}
	return len;
}

String formatBytes(size_t bytes) {
	if (bytes < 1024) {
		return String(bytes) + "B";
	} else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	} else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	} else {
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
	}
}



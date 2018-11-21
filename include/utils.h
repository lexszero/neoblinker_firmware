#pragma once

#include <Arduino.h>

#define __cat_internal(a, ...) a ## __VA_ARGS__
#define __cat(a, ...) __cat_internal(a, __VA_ARGS__)
#define __cat3(a, b, ...) __cat(a, __cat(b, __VA_ARGS__))

#define stringify(x) __stringify(x)
#define __stringify(x) #x

#define _log(...) Serial.printf(__VA_ARGS__)
#define log(fmt, ...) _log("%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define logval(desc, val) _log("%s: ", desc); Serial.println(val);

#ifdef DEBUG
#define debug(...) log(__VA_ARGS__)
#else
#define debug(...)
#endif

uint16_t convertSpeed(uint8_t mcl_speed);
void animate(int sec);
String formatBytes(size_t bytes);

size_t eeprom_read(int offset, void *buf, size_t len);
size_t eeprom_write(int offset, const void *buf, size_t len);
size_t eeprom_write_str(int offset, const char *buf, size_t len);

#include <memory>
#include <cstring>
typedef std::unique_ptr<char, void(*)(void*)> _auto_char_ptr;
static inline _auto_char_ptr auto_char_ptr(char *ptr) {
	return _auto_char_ptr(ptr, std::free);
}

static inline _auto_char_ptr auto_strdup(const char *ptr) {
	return auto_char_ptr(strdup(ptr));
}

static inline _auto_char_ptr auto_strndup(const char *ptr, size_t len) {
	return auto_char_ptr(strndup(ptr, len));
}

namespace Core {
	struct RgbColor {
		uint8_t R, G, B;
	};
}

#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <unordered_map>
#include "utils.h"

/* Configuration */
#include "config.h"

struct Config {
#define CONFIG_PARAM_STR(name, len, defval, desc) char name[len] = defval;
#define CONFIG_PARAM(type, name, defval, desc) type name = defval;
	CONFIG_PARAMS
#undef CONFIG_PARAM_STR
#undef CONFIG_PARAM

	static void load();
	static void save();
};
extern Config config;

/* Controls */
#include "core_controls.h"
class ControlsBase {
public:
	static std::unordered_map<std::string, AbstractControl*> _all;
	ControlsBase() {}
	virtual void begin() {};
};
#include "controls.h"
extern Controls controls;

namespace Core {
	/* Basic API */
	void setup_base();
	void setup();
	void loop();

	extern void onConfigMode();

	namespace Web {
		extern ESP8266WebServer server;
		void setup();

		void returnOk(const char* resp, bool cors = true);
		void returnFail(int code = 400, const char* message = "Bad request", bool cors = true);
		void returnJson(JsonObject& json, bool cors = true);
	}

	namespace MQTT {
		extern Ticker reconnect_timer;
		extern PubSubClient mqtt;

		void setup();
		void connect();
	}

	namespace WiFi {
		void setup();
		void connect();
	}
}

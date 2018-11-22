#include <Arduino.h>
#include <FS.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include "core.h"

namespace Core {

HandlerFn onConnect, onDisconnect, onConfigMode;

static void setup_spiffs() {
	log("initializing SPIFFS");

	SPIFFS.begin();
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String fileName = dir.fileName();
		size_t fileSize = dir.fileSize();
		_log("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
	}

	FSInfo fs_info;
	SPIFFS.info(fs_info);
	_log("FS Usage: %d/%d bytes\n\n", fs_info.usedBytes, fs_info.totalBytes);
}

static void setup_ota() {
	log("starting OTA");
	ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname(config.hostname);
	ArduinoOTA.setPassword(config.password);
	ArduinoOTA.onStart([]() {
		_log("Arduino OTA: Start updating\n");
	});
	ArduinoOTA.onEnd([]() {
		_log("Arduino OTA: End\n");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		_log("Arduino OTA Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		const char *msg;
		switch (error) {
			case OTA_AUTH_ERROR:
				msg = "Auth Failed";
				break;
			case OTA_BEGIN_ERROR:
				msg = "Begin Failed";
				break;
			case OTA_CONNECT_ERROR:
				msg = "Connect Failed";
				break;
			case OTA_RECEIVE_ERROR:
				msg = "Receive Failed";
				break;
			case OTA_END_ERROR:
				msg = "End Failed";
				break;
			default:
				msg = "unknown";
				break;
		}
		_log("Arduino OTA Error[%u]: %s\n", error, msg);
	});

	ArduinoOTA.begin();
}

void setup_base() {
	Serial.begin(115200);
	Serial.println();

	EEPROM.begin(EEPROM_SIZE);
	Config::load();
}

void setup() {
	MQTT::setup();
	WiFi::setup();

	//setup_spiffs();

	Web::setup();
	setup_ota();
	log("startup done");
}

void loop() {
	Web::server.handleClient();
	if (::WiFi.status() == WL_CONNECTED) {
		if (!MQTT::mqtt.connected()) {
			MQTT::connect();
		} else {
			MQTT::mqtt.loop();
		}
	}
}

} // namespace Core

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class AbstractControl {
public:
	AbstractControl(const std::string& _name, const char *_desc = nullptr, bool _readonly = false, int _order = 0);

	const char *getName();
	const char *getDescription();

	virtual String toString() = 0;
	virtual void fromString(String& newval) = 0;

	virtual void toMqtt(char *buf);
	virtual void fromMqtt(const char *newval);

	virtual void appendJson(JsonObject& json) = 0;

	void httpSetHandlers();
	void onMqttConnect();

protected:
	const char *name;
	const char *description;
	const bool readonly;
	const int order;
	std::string mqttPath;

	uint16_t mqttPublish();
	uint16_t mqttMetaAttr(const char *meta, const char *value);
	uint16_t mqttMetaAttr(const char *meta, const String& value);
	void mqttMetaDefault(const char *type);

	virtual void mqttMeta() = 0;
};

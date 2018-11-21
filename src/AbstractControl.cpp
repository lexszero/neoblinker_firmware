#include <Arduino.h>
#include "core.h"
#include "controls.h"

static const size_t MQTT_BUF_SIZE = 16;

#include "AbstractControl.h"

using namespace Core;

AbstractControl::AbstractControl(const std::string& _name, const char *_desc, bool _readonly, int _order) :
	name(strdup(_name.c_str())),
	description(_desc),
	readonly(_readonly),
	order(_order),
	mqttPath(std::string("/devices/") + config.hostname + std::string("/controls/") + std::string(_name.c_str()))
{
	controls._all[_name] = this;
}

const char *AbstractControl::getName() {
	return name;
}

const char *AbstractControl::getDescription() {
	return description;
}

void AbstractControl::toMqtt(char *buf) {
	toString().toCharArray(buf, MQTT_BUF_SIZE);
}

void AbstractControl::fromMqtt(const char *newval) {
	auto str = String(newval);
	fromString(str);
}

void AbstractControl::onMqttConnect() {
	debug("%s: publishing meta", name);

	if (!readonly) {
		uint16_t ret = MQTT::mqtt.subscribe((mqttPath + "/on").c_str(), 0);
		if (ret) {
			log("%s: subscribed", name);
		}
		else {
			log("%s: subscribe failed", name);
		}
	}

	mqttMeta();
	mqttPublish();
}

void AbstractControl::httpSetHandlers() {
	using namespace Core::Web;
	server.on(String("/control/") + name, HTTP_GET, [this]() {
		server.sendHeader("Access-Control-Allow-Origin", "*");
		if (!readonly && server.args() == 1) {
			auto newval = server.arg(0);
			this->fromString(newval);
		}
		server.send(200, "text/plain", this->toString());
	});

	if (!readonly) {
		server.on(String("/control/") + name, HTTP_POST, [this]() {
			if (server.args() == 0)
				return server.send(400, "text/plain", "Bad request");
			auto newval = server.arg(0);
			this->fromString(newval);
			server.send(200, "text/plain", this->toString());
		});
	}
}

uint16_t AbstractControl::mqttPublish() {
	char buf[MQTT_BUF_SIZE];
	toMqtt(buf);
	debug("%s: publish value %s", name, buf);

	auto ret = MQTT::mqtt.publish(mqttPath.c_str(), buf, true);
	if (!ret) {
		log("%s: publish failed", mqttPath.c_str());
	}

	return 1;
}

uint16_t AbstractControl::mqttMetaAttr(const char *meta, const char *value) {
	debug("%s: publish meta %s = %s", name, meta, value ? value : "NULL");

	char topic[128];
	snprintf(topic, sizeof(topic), "%s/meta/%s", mqttPath.c_str(), meta);

	auto ret = MQTT::mqtt.publish(topic, (uint8_t *)value, (value ? strlen(value) : 0), true);
	if (!ret) {
		log("%s: publish failed", topic);
	}
	return 1;
}

uint16_t AbstractControl::mqttMetaAttr(const char *meta, const String& value) {
	return mqttMetaAttr(meta, value.c_str());
}

void AbstractControl::mqttMetaDefault(const char *type) {
	mqttMetaAttr("type", type);
	mqttMetaAttr("order", String(order));
	mqttMetaAttr("readonly", readonly ? "1" : NULL);
}

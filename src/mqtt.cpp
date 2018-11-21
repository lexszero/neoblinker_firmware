#include <ESP8266WiFi.h>
#include <memory>

#include "core.h"

namespace Core {
namespace MQTT {

//AsyncMqttClient mqtt;
static WiFiClient client;
PubSubClient mqtt(client);

Ticker reconnect_timer;

/*
void mqtt_connect() {
	mqtt.connect();
}

static void onMqttConnect(bool sessionPresent) {
	log("connected to MQTT");
	logval("session present", sessionPresent);

	for (auto& it : controls._all) {
		it.second->onMqttConnect();
	}
}

static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
	const char *str;
	switch (reason) {
		case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
			str = "Bad server fingerprint.";
			break;
		case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
			str = "TCP Disconnected.";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
			str = "Unacceptable protocol version.";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
			str = "MQTT Identifier rejected.";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
			str = "MQTT server unavailable.";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
			str = "MQTT malformed credentials.";
			break;
		case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
			str = "MQTT not authorized.";
			break;
		case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
			str = "Not enough space on esp8266.";
			break;
		default:
			str = "unknown";
			break;
	}
	log("disconnected from MQTT, reason: %s", str);
	if (WiFi.isConnected()) {
		mqtt_reconnect_timer.once(5, mqtt_connect);
	}
}

static void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties props,
		size_t len, size_t index, size_t total)
{
	auto msg = auto_strndup(payload, len);
	log("received %s: %s", topic, msg.get());
	char *p = strstr(topic, "controls/");
	if (!p) {
		return;
	}
	p += strlen("controls/");
	std::string ctl_name(p, strchr(p, '/') - p);
	auto ctl = controls._all.find(ctl_name);
	log("control %s", ctl_name.c_str());
	if (ctl != controls._all.end()) {
		ctl->second->fromMqtt(msg.get());
	}
	else {
		log("unknown control");
	}
}

static void onMqttSubscribe(uint16_t id, uint8_t qos) {
	log("ack, id=%d", id);
}

static void onMqttPublish(uint16_t id) {
	log("ack, id=%d", id);
}

void mqtt_setup() {
	if (strlen(config.mqtt_host) > 0 && atoi(config.mqtt_port)) {
		mqtt.onConnect(onMqttConnect);
		mqtt.onDisconnect(onMqttDisconnect);
		mqtt.onMessage(onMqttMessage);
		mqtt.onSubscribe(onMqttSubscribe);
		mqtt.onPublish(onMqttPublish);
		mqtt.setServer(config.mqtt_host, atoi(config.mqtt_port));
		if (strlen(config.mqtt_user) > 0 || strlen(config.mqtt_pass) > 0) {
			mqtt.setCredentials(config.mqtt_user, config.mqtt_pass);
		}
		mqtt.setClientId(config.hostname);
		mqtt_connect();
	}
}
*/

void connect() {
	log("mqtt connect");
	if (!mqtt.connect(config.hostname, "", "")) {
		log("connect failed");
		return;
	}
	log("connected");
	
	for (auto& it : controls._all) {
		it.second->onMqttConnect();
	}
}

static void mqtt_callback(char *topic, uint8_t *payload,  unsigned int len) {
	auto msg = auto_strndup((char *)payload, len);
	log("received %s: %s", topic, msg.get());
	char *p = strstr(topic, "controls/");
	if (!p) {
		return;
	}
	p += strlen("controls/");
	std::string ctl_name(p, strchr(p, '/') - p);
	auto ctl = controls._all.find(ctl_name);
	log("control %s", ctl_name.c_str());
	if (ctl != controls._all.end()) {
		ctl->second->fromMqtt(msg.get());
	}
	else {
		log("unknown control");
	}

}

void setup() {
	log("mqtt setup");
	mqtt.setServer(config.mqtt_host, atoi(config.mqtt_port));
	mqtt.setCallback(mqtt_callback);
}

}}	// namespace Core::MQTT

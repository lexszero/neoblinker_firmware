#include <WiFiManager.h>
#include <Ticker.h>
#include "core.h"

namespace Core {
namespace WiFi {

static WiFiEventHandler handler_connect;
static WiFiEventHandler handler_disconnect;

static Ticker reconnect_timer;

static void onConnect(const WiFiEventStationModeGotIP& event) {
	log("connected");
	if (Core::onConnect)
		Core::onConnect();
}

static void onDisconnect(const WiFiEventStationModeDisconnected& event) {
	log("disconnected");
	MQTT::reconnect_timer.detach();
	reconnect_timer.once(2, connect);
	if (Core::onDisconnect)
		Core::onDisconnect();
}

void wifiManager_configModeCallback(WiFiManager *wifiManager) {
	log("entered config mode");
	logval("SSID", wifiManager->getConfigPortalSSID());
	logval("IP", ::WiFi.softAPIP());
	if (Core::onConfigMode)
		Core::onConfigMode();
}

void wifiManager_saveConfigCallback() {
	log("should save config");
}

void connect() {
	log("reconnecting");
	::WiFi.setSleepMode(WIFI_NONE_SLEEP);
	::WiFi.mode(WIFI_STA);
	::WiFi.begin();
}

static void wifi_setup_basic() {
	::WiFi.setAutoReconnect(true);
	::WiFi.setSleepMode(WIFI_NONE_SLEEP);
	::WiFi.hostname(config.hostname);
	wifi_station_set_hostname(config.hostname);
	handler_connect = ::WiFi.onStationModeGotIP(onConnect);
	handler_disconnect = ::WiFi.onStationModeDisconnected(onDisconnect);
}

#ifdef USE_WIFIMANAGER
void setup() {
	log("initializing WiFi");

	WiFiManager wifiManager;
	wifiManager.setAPCallback(wifiManager_configModeCallback);

#define CONFIG_PARAM_STR(name, len, defval, desc) \
		WiFiManagerParameter param_##name(#name, desc, config.name, len); \
		wifiManager.addParameter(&param_##name);
#define CONFIG_PARAM(...)	/* FIXME */
	CONFIG_PARAMS
#undef CONFIG_PARAM_STR
#undef CONFIG_PARAM

	wifiManager.setSaveConfigCallback(wifiManager_saveConfigCallback);
	wifiManager.setConfigPortalTimeout(180);

	wifi_setup_basic();
	if (!wifiManager.autoConnect(config.hostname)) {
		log("failed to connect and hit timeout");
		ESP.reset();
		delay(1000);
	}

#define CONFIG_PARAM_STR(name, len, defval, desc) strcpy(config.name, param_##name.getValue());
#define CONFIG_PARAM(...)	/* FIXME */
	CONFIG_PARAMS
#undef CONFIG_PARAM_STR
#undef CONFIG_PARAM

}

#else
void setup() {
	log("connecting to %s", config.wifi_ssid);
	::WiFi.mode(WIFI_STA);
	wifi_setup_basic();
	::WiFi.begin(config.wifi_ssid, config.wifi_pass);
	while (::WiFi.status() != WL_CONNECTED) {
		_log(".");
		delay(500);
	}
}
#endif

}}	// namespace Core::WiFi

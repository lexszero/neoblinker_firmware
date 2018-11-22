#pragma once

/* Hardware setup */
#define LED_COLOR_ORDER GRB
#define LED_COUNT 32

#define HARDWARE_SONOFF

#ifdef HARDWARE_SONOFF
#define BOARD_LED 13
#define BOARD_RELAY 12
#define BOARD_BUTTON 0
#endif

/* If you don't like WiFiManager for some reason and prefer hardcoded configuration
 * instead, comment this line and adjust values down below
 */
#define USE_WIFIMANAGER

#ifdef USE_WIFIMANAGER
	#define CONFIG_PARAMS_WIFI
#else
	#define CONFIG_PARAMS_WIFI \
		CONFIG_PARAM_STR(		wifi_ssid,	64,		"my_ssid",		"WiFi SSID") \
		CONFIG_PARAM_STR(		wifi_pass,	64,		"my_password", 	"WiFi password")
#endif

#define EEPROM_SIZE 512
#define EEPROM_OFFSET 16

#define CONFIG_PARAMS \
	CONFIG_PARAM_STR(		hostname,	64,		"neoblinker",	"Hostname") \
	CONFIG_PARAMS_WIFI \
	CONFIG_PARAM_STR(		password,	64,		"admin",		"Password") \
	CONFIG_PARAM_STR(		mqtt_host,	64,		"192.168.0.10",	"MQTT host") \
	CONFIG_PARAM_STR(		mqtt_port,	6,		"1883",			"MQTT port") \
	CONFIG_PARAM_STR(		mqtt_user,	32,		"",				"MQTT username") \
	CONFIG_PARAM_STR(		mqtt_pass,	32,		"",				"MQTT password") \
	CONFIG_PARAM_STR(		mqtt_name,	64,		"NeoBlinker",	"MQTT device name") \
	CONFIG_PARAM(uint16_t,	artnet_universe,	0,				"ArtNet Universe") \
	CONFIG_PARAM(uint16_t,	artnet_offset,		0,				"ArtNet start offset")



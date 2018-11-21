#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "core.h"

namespace Core {
namespace Web {

ESP8266WebServer server(80);
static ESP8266HTTPUpdateServer updater(true);

/* FS ops handlers */
File fsUploadFile;

String getContentType(String filename) {
	if (server.hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool handleFileRead(String path) {
	logval("path", path);
	if (path.endsWith("/")) path += "index.htm";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
    server.sendHeader("Access-Control-Allow-Origin", "*");
		size_t sent = server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}

void handleFileUpload() {
	if (server.uri() != "/edit") return;
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {
		String filename = upload.filename;
		if (!filename.startsWith("/")) filename = "/" + filename;
		logval("name", filename);
		fsUploadFile = SPIFFS.open(filename, "w");
		filename = String();
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		//DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
	if (fsUploadFile)
		fsUploadFile.write(upload.buf, upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();
		logval("size", upload.totalSize);
	}
}

void handleFileDelete() {
	if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
	String path = server.arg(0);
	logval("path", path);
	if (path == "/")
		return server.send(500, "text/plain", "BAD PATH");
	if (!SPIFFS.exists(path))
		return server.send(404, "text/plain", "FileNotFound");
	SPIFFS.remove(path);
	server.send(200, "text/plain", "");
	path = String();
}

void handleFileCreate() {
	if (server.args() == 0)
		return server.send(500, "text/plain", "BAD ARGS");
	String path = server.arg(0);
	logval("path", path);
	if (path == "/")
		return server.send(500, "text/plain", "BAD PATH");
	if (SPIFFS.exists(path))
		return server.send(500, "text/plain", "FILE EXISTS");
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return server.send(500, "text/plain", "CREATE FAILED");
	server.send(200, "text/plain", "");
	path = String();
}

void handleFileList() {
	if (!server.hasArg("dir")) {
		server.send(500, "text/plain", "BAD ARGS");
		return;
	}
	
	String path = server.arg("dir");
	logval("path", path);
	Dir dir = SPIFFS.openDir(path);
	path = String();
	
	String output = "[";
	while (dir.next()) {
		File entry = dir.openFile("r");
		if (output != "[") output += ',';
		bool isDir = false;
		output += "{\"type\":\"";
		output += (isDir) ? "dir" : "file";
		output += "\",\"name\":\"";
		output += String(entry.name()).substring(1);
		output += "\"}";
		entry.close();
	}
	
	output += "]";
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "text/json", output);
}

/* Misc handlers */
void handleMinimalUpload() {
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send ( 200, "text/html",
			"<!DOCTYPE html>\
			<html>\
			<head>\
			<title>ESP8266 Upload</title>\
			<meta charset=\"utf-8\">\
			<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
			<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
			</head>\
			<body>\
			<form action=\"/edit\" method=\"post\" enctype=\"multipart/form-data\">\
			<input type=\"file\" name=\"data\">\
			<input type=\"text\" name=\"path\" value=\"/\">\
			<button>Upload</button>\
			</form>\
			</body>\
			</html>"
			);
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}
	server.send ( 404, "text/plain", message );
}

void headerCORS() {
	server.sendHeader("Access-Control-Allow-Origin", "*");
}

void returnOk(const char* resp, bool cors) {
	if (cors)
		headerCORS();
	server.send (200, "text/plain", resp);
}

void returnFail(int code, const char* msg, bool cors) {
	if (cors)
		headerCORS();
	server.send (code, "text/plain", msg);
}

void returnJson(JsonObject& json, bool cors) {
	String json_str;
	serializeJson(json, json_str);
	if (cors)
		headerCORS();
	server.send(200, "application/json", json_str);
}

static void setup_handlers_basic() {
	server.on("/list", HTTP_GET, handleFileList);
	server.on("/edit", HTTP_GET, []() {
		if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
	});
	server.on("/edit", HTTP_PUT, handleFileCreate);
	server.on("/edit", HTTP_DELETE, handleFileDelete);
	server.on("/edit", HTTP_POST, []() {
		returnOk("");
	}, handleFileUpload);
	server.on("/esp_status", HTTP_GET, []() {
		DynamicJsonDocument jsonBuffer;
		JsonObject json = jsonBuffer.to<JsonObject>();

		json["HOSTNAME"] = config.hostname;
		json["heap"] = ESP.getFreeHeap();
		json["sketch_size"] = ESP.getSketchSize();
		json["free_sketch_space"] = ESP.getFreeSketchSpace();
		json["flash_chip_size"] = ESP.getFlashChipSize();
		json["flash_chip_real_size"] = ESP.getFlashChipRealSize();
		json["flash_chip_speed"] = ESP.getFlashChipSpeed();
		json["sdk_version"] = ESP.getSdkVersion();
		json["core_version"] = ESP.getCoreVersion();
		json["cpu_freq"] = ESP.getCpuFreqMHz();
		json["chip_id"] = ESP.getFlashChipId();
		json["led_count"] = LED_COUNT;
#ifdef BOARD_BUTTON
		json["button"] = "ON";
#endif
		returnJson(json);
	});


	//called when the url is not defined here
	//use it to load content from SPIFFS
	server.onNotFound([]() {
			if (!handleFileRead(server.uri()))
			handleNotFound();
			});

	server.on("/upload", handleMinimalUpload);

	server.on("/restart", []() {
			log("/restart");
			returnOk("restarting...");
			ESP.restart();
	});

	server.on("/reset_wlan", []() {
			log("/reset_wlan");
			returnOk("Resetting WLAN and restarting...");
			WiFiManager wifiManager;
			wifiManager.resetSettings();
			ESP.restart();
			});

	server.on("/start_config_ap", []() {
			log("/start_config_ap");
			returnOk("Starting config AP ...");
			WiFiManager wifiManager;
			wifiManager.startConfigPortal(config.hostname);
			});
}

static void setup_handlers_api() {
	server.on("/status", []() {
		DynamicJsonDocument jsonBuffer;
		JsonObject json = jsonBuffer.to<JsonObject>();

		for (const auto& ctl : controls._all) {
			ctl.second->appendJson(json);
		}
		returnJson(json);
	});

	for (const auto& item : controls._all) {
		item.second->httpSetHandlers();
	}
}

void setup() {
	setup_handlers_basic();
	setup_handlers_api();
	updater.setup(&server, "/update", "admin", config.password);
	server.begin();
}

}}	// namespace Core::Web

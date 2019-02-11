#include "core.h"
#include <NeoAnimationFX.h>
#include <Artnet.h>
using namespace std::placeholders;

Ticker blink_timer;

#define LED_METHOD __cat3(NeoPBB, LED_COLOR_ORDER, D800)
LED_METHOD strip(LED_COUNT);
NeoAnimationFX<LED_METHOD> leds(strip);
static Artnet artnet;

void board_led_blink(float period) {
	blink_timer.attach(period, []() {
			#ifdef BOARD_LED
			digitalWrite(BOARD_LED, !digitalRead(BOARD_LED));
			#endif
		});
}

void board_button_handle() {
#ifdef BOARD_BUTTON
	static uint32_t pressed = 0;
	static uint32_t last_time = 0;
	auto now = millis();
	if (now - last_time < 25)
		return;
	bool state = !digitalRead(BOARD_BUTTON);
	if (state) {
		if (!pressed)
			log("pressed");
		pressed++;
	}
	else {
		if (pressed > 40)
			log("long");
		pressed = 0;
	}
	last_time = now;
#endif
}

uint16_t convertSpeed(uint8_t mcl_speed) {
	//long ws2812_speed = mcl_speed * 256;
	uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
	ws2812_speed = SPEED_MAX - ws2812_speed;
	if (ws2812_speed < SPEED_MIN) {
		ws2812_speed = SPEED_MIN;
	}
	if (ws2812_speed > SPEED_MAX) {
		ws2812_speed = SPEED_MAX;
	}
	return ws2812_speed;
}

static inline RgbColor convertColor(const Core::RgbColor& color) {
	return RgbColor { color.R, color.G, color.B };
}

void leds_init() {
	log("initializing strip with %d LEDs, color order %s", LED_COUNT, stringify(LED_COLOR_ORDER));

	leds.init();
	leds.setNumSegments(SEGMENTS_COUNT);
	#define SEGMENT_ITEM(id, name, start, stop, reverse) \
		leds.setSegment(id,  start, stop, \
				controls.segments[id].mode, \
				convertColor(controls.segments[id].colors[0]), \
				convertSpeed(controls.segments[id].speed), reverse); \
		leds.setColor(id, 1, convertColor(controls.segments[id].colors[1])); \
		leds.setColor(id, 2, convertColor(controls.segments[id].colors[2]));
		SEGMENTS
	#undef SEGMENT_ITEM
	leds.start();
}

void segments_sync(uint8_t seg, std::function<void(Controls::Segment&)> func) {
	static bool sync_in_progress;
	if (sync_in_progress || !controls.sync)
		return;
	sync_in_progress = true;
	for (int i = 0; i < SEGMENTS_COUNT; i++) {
		if (controls.segments[i].id == seg)
			continue;
		func(controls.segments[i]);
	}
	sync_in_progress = false;
}

/* Artnet */
void artnet_init() {
	artnet.begin();
	auto ip = WiFi.localIP();
	static uint8_t broadcast[4] = {
		ip[0], ip[1], ip[2], 255
	};
	artnet.setBroadcast(broadcast);
	artnet.setArtDmxCallback(
		[] (uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP) {
			static int prev_len = 0;
		
			log("DMX frame");
			// read universe and put into the right part of the display buffer
			for (int i = 0; i < length / 3; i++)
			{
				int led = i + (universe - config.artnet_universe) * (prev_len / 3);
				if (led < LED_COUNT) {
					leds.setPixelColor(led, RgbColor(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
				}
				else {
					break;
				}
			}
			prev_len = length;
			strip.Show();
		});
}

/* Controls */
Controls::Controls() :
	ControlsBase(),
#ifdef BOARD_RELAY
	relay("relay", "Relay", 0, false,
		[](bool val) {
			log("relay = %d", val);
			digitalWrite(BOARD_RELAY, val);
		}),
#endif
	artnet("artnet", "ArtNet enabled", 5, true,
		[](bool val) {
			log("artnet = %d", val);
		}),
	brightness("brightness", "Brightness", 6, 255,
		[](uint8_t val) {
			leds.setBrightness(val);
		}),
	trigger("trigger", "Trigger", 7,
		[](bool val) {
			leds.trigger();
		}),
	sync("sync", "Sync", 8, true,
		[](bool val) {
			log("sync = %d", val);
			segments_sync(0, [](Segment& seg) {
				const auto& src = controls.segments[0];
				seg.mode = src.mode;
				seg.speed = src.speed;
				for (uint8_t i = 0; i < NUM_COLORS; i++) {
					seg.colors[i] = src.colors[i];
				}
			});
		}),
	segments {
#define SEGMENT_ITEM(id, name, ...) Segment(stringify(name), id),
	SEGMENTS
#undef SEGMENT_ITEM
	},
	_dummy()
{}

static void setcolor_helper(uint8_t seg, uint8_t n, const Core::RgbColor& val) {
	auto c = convertColor(val);
	log("seg %d color %d = %x", seg, n, HtmlColor(c).Color);
	leds.setColor(seg, n, c);
	segments_sync(seg, [n, val](Controls::Segment& seg) {
		seg.colors[n] = val;
	});
}

Controls::Segment::Segment(const std::string&& prefix, uint8_t _id) :
	id(_id),
	mode(prefix + "_mode", "Animation mode", (id+1)*10+1,
			FX_MODE_BREATH,
			[this](uint8_t val) {
				log("seg %d mode = %d", id, val);
				leds.setMode(id, val);
				segments_sync(id, [val](Segment& seg) {
					seg.mode = val;
				});
			}),
	speed(prefix + "_speed", "Animation speed", (id+1)*10+2,
			128,
			[this](uint16_t val) {
				log("seg %d speed = %d", id, val);
				leds.setSpeed(id, convertSpeed(val));
				segments_sync(id, [val](Segment& seg) {
					seg.speed = val;
				});
			}),
	colors {
		ControlRGB(prefix + "_color1", "Color 1",  (id+1)*10+3,
				Core::RgbColor{255, 0, 0},
				std::bind(setcolor_helper, id, 0, _1)),
		ControlRGB(prefix + "_color2", "Color 2", (id+1)*10+4,
				Core::RgbColor{0, 255, 0},
				std::bind(setcolor_helper, id, 1, _1)),
		ControlRGB(prefix + "_color3", "Color 3", (id+1)*10+5,
				Core::RgbColor{0, 0, 255},
				std::bind(setcolor_helper, id, 2, _1))
	},
	next(prefix + "_next", "Next mode", (id+1)*10+6,
			[this](bool val) {
				log("seg %d next", id);
				auto m = leds.getMode(id);
				mode = m == MODE_COUNT ? 0 : m + 1;
			}),
	prev(prefix + "_prev", "Prev mode", (id+1)*10+7,
			[this](bool val) {
				log("seg %d prev", id);
				auto m = leds.getMode(id);
				mode = m == 0 ? MODE_COUNT : m - 1;
			})
{}

struct Controls controls;

void setup()
{
	Core::setup_base();

#ifdef BOARD_LED
	pinMode(BOARD_LED, OUTPUT);
#endif
#ifdef BOARD_BUTTON
	pinMode(BOARD_BUTTON, INPUT_PULLUP);
#endif
#ifdef BOARD_RELAY
	pinMode(BOARD_RELAY, OUTPUT);
#endif

	leds_init();
	artnet_init();

	Core::onConfigMode = []() {
		board_led_blink(0.2);
		uint16_t i;
		for (i = 0; i < leds.numPixels(); i++) {
			leds.setPixelColor(i, 0, 0, 255);
		}
		leds.show();
	};
	
	Core::onConnect = []() {
		board_led_blink(1);
	};
	
	Core::onDisconnect = []() {
		board_led_blink(0.1);
	};

	Core::setup();
}

void loop() {
	Core::loop();
	if (controls.artnet) {
		if (WiFi.status() == WL_CONNECTED)
			artnet.read();
	}
	else {
		leds.service();
	}
	board_button_handle();
}

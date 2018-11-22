#include "core.h"
#include <NeoAnimationFX.h>
#include <Artnet.h>
using namespace std::placeholders;

Ticker blink_timer;

#define LED_METHOD __cat3(NeoPBB, LED_COLOR_ORDER, D800)
LED_METHOD strip(LED_COUNT);
NeoAnimationFX<LED_METHOD> leds(strip);
static Artnet artnet;

void led_toggle() {
#ifdef BOARD_LED
	digitalWrite(BOARD_LED, !digitalRead(BOARD_LED));
#endif
}

void led_blink(float period) {
	blink_timer.attach(period, led_toggle);
}

void led_init() {
	log("initializing strip with %d LEDs, color order %s", LED_COUNT, stringify(LED_COLOR_ORDER));

	leds.init();
	controls.begin();
	leds.start();
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

void led_setcolor(uint8_t seg, uint8_t n, const Core::RgbColor& val) {
	auto c = convertColor(val);
	log("seg %d color %d = %x", seg, n, HtmlColor(c).Color);
	leds.setColor(seg, n, c);
}

/* Artnet */
static void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP)
{
	static int previousDataLength = 0;

	Serial.println("DMX frame");
	// read universe and put into the right part of the display buffer
	for (int i = 0; i < length / 3; i++)
	{
		int led = i + (universe - config.artnet_universe) * (previousDataLength / 3);
		if (led < LED_COUNT) {
			leds.setPixelColor(led, RgbColor(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
		}
		else {
			break;
		}
	}
	previousDataLength = length;
	strip.Show();
}

void artnet_init()
{
	artnet.begin();
	auto ip = WiFi.localIP();
	static uint8_t broadcast[4] = {
		ip[0], ip[1], ip[2], 255
	};
	artnet.setBroadcast(broadcast);
	artnet.setArtDmxCallback(onDmxFrame);
}

/* Controls */
Controls::Controls() :
	ControlsBase(),
	artnet("artnet", "ArtNet enabled", 0, false,
		[](bool val) {
			log("artnet = %d", val);
		}),
	brightness("brightness", "Brightness", 1, 255,
		[](uint8_t val) {
			leds.setBrightness(val);
		}),
	trigger("trigger", "Trigger", 2,
		[](bool val) {
			leds.trigger();
		}),
#define SEGMENT_ITEM(id, name, ...) SEG(name)(stringify(name), id),
	SEGMENTS
#undef SEGMENT_ITEM
	_dummy()
{}

void Controls::begin() {
	leds.setNumSegments(2);
	#define SEGMENT_ITEM(id, name, start, stop, reverse) \
		leds.setSegment(id,  start, stop, SEG(name).mode, convertColor(SEG(name).color1), SEG(name).speed, reverse); \
		leds.setColor(id, 1, convertColor(SEG(name).color2)); \
		leds.setColor(id, 2, convertColor(SEG(name).color3));
		SEGMENTS
	#undef SEGMENT
}

Controls::Segment::Segment(const std::string&& prefix, uint8_t _id) :
	id(_id),
	mode(prefix + "_mode", "Animation mode", (id+1)*10+1,
			FX_MODE_RAINBOW,
			[this](uint8_t val) {
				log("seg %d mode = %d", id, val);
				leds.setMode(id, val);
			}),
	speed(prefix + "_speed", "Animation speed", (id+1)*10+2,
			DEFAULT_SPEED,
			[this](uint16_t val) {
				log("seg %d speed = %d", id, val);
				leds.setSpeed(id, convertSpeed(val));
			}),
	color1(prefix + "_color1", "Color 1",  (id+1)*10+3,
			Core::RgbColor{255, 0, 0},
			std::bind(led_setcolor, id, 0, _1)),
	color2(prefix + "_color2", "Color 2", (id+1)*10+4,
			Core::RgbColor{0, 255, 0},
			std::bind(led_setcolor, id, 1, _1)),
	color3(prefix + "_color3", "Color 3", (id+1)*10+5,
			Core::RgbColor{0, 0, 255},
			std::bind(led_setcolor, id, 2, _1)),
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

	led_init();

	 Core::onConfigMode = []( {
		led_blink(0.2);
		uint16_t i;
		for (i = 0; i < leds.numPixels(); i++) {
			leds.setPixelColor(i, 0, 0, 255);
		}
		leds.show();
	};

	Core::onConnect = []() {
		led_blink(1);
	};

	Core::onDisconnect = []() {
		led_blink(0.1);
	};


	artnet_init();

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
}

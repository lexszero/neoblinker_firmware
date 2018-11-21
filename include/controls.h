#pragma once
#include "core.h"
#include <NeoAnimationFX.h>

#define CTL_SPEED_MAX 2000

#define SEGMENTS \
	SEGMENT_ITEM(0, all, 0, (LED_COUNT-1), false)

#define SEG(name) __cat(seg_, name)

class Controls : ControlsBase {
public:
	Controls();
	void begin();

	std::unordered_map<std::string, AbstractControl*> _all;

	ControlSwitch artnet;
	ControlRange<uint8_t, 255> brightness;
	ControlPushButton trigger;

	struct Segment {
		Segment(const std::string&& prefix, uint8_t _id);

		uint8_t id;

		ControlRange<uint8_t, MODE_COUNT> mode;
		ControlRange<uint16_t, CTL_SPEED_MAX> speed;
		ControlRGB color1, color2, color3;
		ControlPushButton next, prev;

	};

#define SEGMENT_ITEM(id, name, start, stop, reverse) Segment SEG(name);
	SEGMENTS
#undef SEGMENT_ITEM

	bool _dummy;
};

extern Controls controls;


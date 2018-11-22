#pragma once
#include "core.h"
#include <NeoAnimationFX.h>

#define SEGMENTS_COUNT 2
#define SEGMENTS \
	SEGMENT_ITEM(0, A,				0,	(LED_COUNT/2-1),	false) \
	SEGMENT_ITEM(1, B,	(LED_COUNT/2),	(LED_COUNT-1),		true)

#define SEG(name) __cat(seg_, name)

class Controls : ControlsBase {
public:
	Controls();

	std::unordered_map<std::string, AbstractControl*> _all;

#ifdef BOARD_RELAY
	ControlSwitch relay;
#endif

	ControlSwitch artnet;
	ControlRange<uint8_t, 255> brightness;
	ControlPushButton trigger;

	ControlSwitch sync;

	struct Segment {
		Segment(const std::string&& prefix, uint8_t _id);
		Segment& operator=(const Segment& seg);

		uint8_t id;

		ControlRange<uint8_t, MODE_COUNT> mode;
		ControlRange<uint8_t, 255> speed;
		ControlRGB colors[3];
		ControlPushButton next, prev;
	};

	Segment segments[SEGMENTS_COUNT];
	void segmentsSyncWith(uint8_t id);

#define SEGMENT_ITEM(id, name, start, stop, reverse) Segment& SEG(name) = segments[id];
	SEGMENTS
#undef SEGMENT_ITEM

	bool _dummy;
};

extern Controls controls;


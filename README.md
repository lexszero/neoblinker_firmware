NeoBlinker
==========

The firmware for blinking lights on ESP8266.

Building and flashing
---------------------

This project uses [PlatformIO](https://platformio.org/) for dependencies
vendoring and build system. Consult its documentation.

TLDR:

	$ pip install platformio
	$ pio run -t upload

Features
--------

* Blinks WS2812 LEDs
* Runs on any ESP8266 board (probably)
* Provides multiple ways of controlling animations:
	* MQTT interface compatible with [Wiren Board
	conventions](https://github.com/contactless/homeui/blob/master/conventions.md)
	* HTTP API
* ArtNet support for streaming raw pixels from a wide range of software

Motivation
----------

You may ask why yet another firmware is needed, there are multiple similar
projects exist. The reason is simple: they all suck. Unreadable spaghetty code,
shittons of copypaste and not a sign of a decent architecture.

Here I'm trying not just to solve the task I have on hand, but at the same
time to design and implement a bullshit-free framework for development of
networked sensors and actuators without writing too much boilerplate for
a common use cases. Ideally, the user of the framework should not care about
anything but business-logic they want to implement.

This project tries to get the best from modern C++ features and programming
techniques while still keeping things not overly complicated and providing
a clean and uncluttered way to do stuff.

TODO
----

* Write TODO

Internals
---------

Read the source, Luke.

**WARNING**: I love metaprogramming and hate copypaste. Therefore, expect
preprocessor abuse and dragons. You have been warned.

The source code is organised in two parts: core and business logic.

### Core

This is what I call "the framework". It provides WiFi management (by means of
[WiFiManager](https://github.com/tzapu/WiFiManager) library), application
configuration facility, MQTT and HTTP APIs, OTA (_FIXME: broken_) and possibly
other boilerplate not directly related to doing things you want to do (but helping
to achieve that).

There are two main entities managed by the core: **config** and **controls**.
Config contains parameters which are constant during device operation, and
controls represent actual switches, knobs and gauges you want your device to
have.

Currently the implementation of the core is quite dirty and is subject to change.
The goal is to distribute core as a completely independent library, which is
impossible to do with the way things are currently implemented (to be precise, 

### Business logic

To get impression about business logic implementation, look into the following
files:

* `include/config.h` - configurable parameters
* `include/controls.h` - definitions of controls
* `src/main.cpp` - where we are actually doing useful things

All other files are related to the core and in ideal world must not be there.

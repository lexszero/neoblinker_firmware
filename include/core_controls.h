#pragma once

#include <functional>
#include <unordered_map>
#include "AbstractControl.h"

template <typename Type>
class GenericControl : public AbstractControl {
public:
	using Setter = std::function<void(const Type&)>;

	explicit GenericControl(const std::string& _name, const char *_desc, int _order, const Type& _defval, Setter _setter = nullptr) :
		AbstractControl(
				_name,
				_desc,
				!_setter,
				_order),
		value(_defval),
		setter(_setter)
	{}

	Type& operator=(const Type& newval) {
		set(newval, true, true);
		return value;
	}

	GenericControl<Type>& operator=(const GenericControl<Type>& src) {
		set(src.value, true, true);
		return *this;
	}

	operator Type&() {
		return value;
	}

	virtual void appendJson(JsonObject& json) {
		json[name] = value;
	}

	virtual void set(const Type& newval, bool publish = true, bool call = true) {
		if (call && setter)
			setter(newval);
		value = newval;
		if (publish)
			mqttPublish();
	}
protected:
	Type value;
	Setter setter;
};

template <typename Type>
class Control : public GenericControl<Type> {
public:
	using GenericControl<Type>::GenericControl;
	using GenericControl<Type>::operator=;
	using GenericControl<Type>::operator Type&;
	using GenericControl<Type>::set;

	virtual String toString() {
		return String(this->value);
	}
};

class ControlSwitch : public Control<bool> {
	using Control<bool>::Control;

	void mqttMeta() {
		AbstractControl::mqttMetaDefault("switch");
	}

	void fromString(String& newval) {
		Control<bool>::set(newval == "1" ? true : false);
	}
};

class ControlPushButton : public Control<bool> {
public:
	explicit ControlPushButton(const std::string& _name, const char *_desc, int _order, Setter _setter = nullptr) :
		Control(_name, _desc, _order, true, _setter)
	{}

	void mqttMeta() {
		AbstractControl::mqttMetaDefault("pushbutton");
	}

	void fromString(String& newval) {
		Control<bool>::set(newval.toInt(), false);
	}
};

template <typename Type, Type Max>
class ControlRange : public Control<Type> {
public:
	using Control<Type>::Control;
	using Control<Type>::operator=;

	void mqttMeta() {
		AbstractControl::mqttMetaDefault("range");
		AbstractControl::mqttMetaAttr("max", String(Max));
	}

	void fromString(String& newval) {
		Control<Type>::set(newval.toInt());
	}
};

class ControlRGB : public GenericControl<Core::RgbColor> {
public:
	using GenericControl<Core::RgbColor>::GenericControl;
	using GenericControl<Core::RgbColor>::operator=;
	
	void mqttMeta() {
		AbstractControl::mqttMetaDefault("rgb");
	}

	void fromString(String& newval) {
		Core::RgbColor c;
		auto s1 = newval.indexOf(';');
		auto s2 = newval.indexOf(';', s1+1);
		c.R = newval.substring(0, s1).toInt();
		c.G = newval.substring(s1+1, s2).toInt();
		c.B = newval.substring(s2+1).toInt();
		GenericControl<Core::RgbColor>::set(c);
	}

	String toString() {
		auto v = this->value;
		return String(v.R) + ";" + String(v.G) + ";" + String(v.B);
	}

};

template<>
void GenericControl<Core::RgbColor>::appendJson(JsonObject& json);

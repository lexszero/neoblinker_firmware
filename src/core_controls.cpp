#include "utils.h"
#include "core_controls.h"

template<>
void GenericControl<Core::RgbColor>::appendJson(JsonObject& json) {
	json[name] = toString();
}

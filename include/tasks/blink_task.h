#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Starts the FreeRTOS task that toggles the built-in LED.
void startBlinkTask(UBaseType_t priority);

}  // namespace tasks

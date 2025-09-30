#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Starts the FreeRTOS task that manages the I2C LCD display.
// This task displays "Hello World" and updates periodically.
void startDisplayTask(UBaseType_t priority);

}  // namespace tasks
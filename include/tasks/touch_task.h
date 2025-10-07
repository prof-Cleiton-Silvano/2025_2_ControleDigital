#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Starts a FreeRTOS task that reads an ESP32 touch-capable pin and sends
// 'H' (high/touched) or 'L' (low/not touched) to the display at row 0, col 3.
void startTouchTask(UBaseType_t priority);

}  // namespace tasks

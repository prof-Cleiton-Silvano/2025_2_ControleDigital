#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Commands that can be sent to the display task.
enum class DisplayCmd : uint8_t { WriteChar = 0, Clear = 1 };

// Message structure sent to the display task.
// - For WriteChar: set col,row and c
// - For Clear: other fields ignored
struct DisplayMessage {
	DisplayCmd cmd;
	uint8_t col;
	uint8_t row;
	char c;
};

// Enqueue a message to the display task. Returns true on success.
bool sendDisplayMessage(const DisplayMessage& msg, TickType_t ticksToWait = portMAX_DELAY);

// Starts the FreeRTOS task that manages the I2C LCD display.
void startDisplayTask(UBaseType_t priority);

}  // namespace tasks
#pragma once

#include <stdint.h>

namespace hal {

// Initializes board-specific hardware used across tasks.
void initBoard();

// Controls the built-in LED state.
void setBuiltinLed(bool enabled);

// Provides the Arduino pin index for the built-in LED.
uint8_t builtinLedPin();

}  // namespace hal

#pragma once

#include <stdint.h>

namespace hal {

// Initializes board-specific hardware used across tasks.
void initBoard();

// Controls the built-in LED state.
void setBuiltinLed(bool enabled);

// Provides the Arduino pin index for the built-in LED.
uint8_t builtinLedPin();

// TB6600 Stepper Motor Driver Control
void setStepperPulse(bool state);        // Controls STEP/PUL pin
void setStepperDirection(bool clockwise); // Controls DIR pin (true = CW, false = CCW)
void setStepperEnable(bool enabled);      // Controls ENA pin (true = motor ON, false = motor OFF)

// TB6600 pin getters
uint8_t stepperPulsePin();
uint8_t stepperDirectionPin();
uint8_t stepperEnablePin();

}  // namespace hal

#pragma once

#include <stdint.h>

namespace hal {

// ============================================================================
// PIN DEFINITIONS - Centralized I/O Pin Configuration
// ============================================================================

// Built-in LED
constexpr uint8_t kBuiltinLedPin = 2;  // ESP32 built-in LED

// User Buttons (active LOW with internal pull-up)
constexpr uint8_t kUserBtn1Pin = 32;
constexpr uint8_t kUserBtn2Pin = 33;
constexpr uint8_t kUserBtn3Pin = 25;

// Limit Switches (active HIGH with pull-down)
constexpr uint8_t kLimitBtn1Pin = 26;
constexpr uint8_t kLimitBtn2Pin = 27;

// TB6600 Stepper Motor Driver Pins
constexpr uint8_t kStepperEnablePin = 18;     // ENA - LOW=enabled, HIGH=disabled
constexpr uint8_t kStepperDirectionPin = 17;  // DIR - direction control
constexpr uint8_t kStepperPulsePin = 16;      // PUL/STEP - step pulse

// ============================================================================
// HARDWARE INITIALIZATION AND CONTROL
// ============================================================================

// Initializes board-specific hardware used across tasks.
void initBoard();

// Controls the built-in LED state.
void setBuiltinLed(bool enabled);

// User button readers (returns true when pressed)
bool readUserBtn1();
bool readUserBtn2();
bool readUserBtn3();

// Limit switch readers (returns true when activated)
bool readLimitBtn1();
bool readLimitBtn2();

}  // namespace hal

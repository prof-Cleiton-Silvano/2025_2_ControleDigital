#include <Arduino.h>

#include "hal/board.h"

// Define LED_BUILTIN if not defined by the environment
#ifndef LED_BUILTIN
#define LED_BUILTIN 13  // Change 13 to your board's built-in LED pin if different
#endif

namespace hal {
namespace {
constexpr uint8_t kBuiltinLedPin = LED_BUILTIN;
}  // namespace

void initBoard() {
  pinMode(kBuiltinLedPin, OUTPUT);
  digitalWrite(kBuiltinLedPin, LOW);
}

void setBuiltinLed(bool enabled) {
  digitalWrite(kBuiltinLedPin, enabled ? HIGH : LOW);
}

uint8_t builtinLedPin() {
  return kBuiltinLedPin;
}

}  // namespace hal

#include <Arduino.h>

#include "hal/board.h"

// Define LED_BUILTIN if not defined by the environment
#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // ESP32 built-in LED is typically on pin 2
#endif

// Define LED_BUILTIN if not defined by the environment
#ifndef SW_01
#define SW_01 GPIO_NUM_10  // Change 13 to your board's built-in LED pin if different
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

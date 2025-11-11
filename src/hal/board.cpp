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

// Pinos dos botões de usuário e fim de curso
constexpr uint8_t kUserBtn1Pin = 32;
constexpr uint8_t kUserBtn2Pin = 33;
constexpr uint8_t kUserBtn3Pin = 25;
constexpr uint8_t kLimitBtn1Pin = 26;
constexpr uint8_t kLimitBtn2Pin = 27;

// Pinos do driver TB6600 para motor de passo
constexpr uint8_t kStepperEnablePin = 18;     // ENA - GPIO 23 (LOW = enabled, HIGH = disabled)
constexpr uint8_t kStepperDirectionPin = 17;  // DIR - GPIO 19
constexpr uint8_t kStepperPulsePin = 16;      // PUL/STEP - GPIO 18
}  // namespace

void initBoard() {
  pinMode(kBuiltinLedPin, OUTPUT);
  digitalWrite(kBuiltinLedPin, LOW);

  // Inicializa botões de usuário como pull-up
  pinMode(kUserBtn1Pin, INPUT_PULLUP);
  pinMode(kUserBtn2Pin, INPUT_PULLUP);
  pinMode(kUserBtn3Pin, INPUT_PULLUP);

  // Inicializa botões de fim de curso como pull-down
  pinMode(kLimitBtn1Pin, INPUT_PULLDOWN);
  pinMode(kLimitBtn2Pin, INPUT_PULLDOWN);

  // Inicializa pinos do motor de passo (TB6600)
  pinMode(kStepperPulsePin, OUTPUT);
  pinMode(kStepperDirectionPin, OUTPUT);
  pinMode(kStepperEnablePin, OUTPUT);
  digitalWrite(kStepperPulsePin, LOW);
  digitalWrite(kStepperDirectionPin, LOW);
  digitalWrite(kStepperEnablePin, HIGH);  // Motor desabilitado inicialmente (HIGH = disabled)
}

void setBuiltinLed(bool enabled) {
  digitalWrite(kBuiltinLedPin, enabled ? HIGH : LOW);
}

// Funções para ler estado dos botões
bool readUserBtn1() { return digitalRead(kUserBtn1Pin) == LOW; }
bool readUserBtn2() { return digitalRead(kUserBtn2Pin) == LOW; }
bool readUserBtn3() { return digitalRead(kUserBtn3Pin) == LOW; }
bool readLimitBtn1() { return digitalRead(kLimitBtn1Pin) == HIGH; }
bool readLimitBtn2() { return digitalRead(kLimitBtn2Pin) == HIGH; }

uint8_t builtinLedPin() {
  return kBuiltinLedPin;
}

uint8_t userBtn1Pin() { return kUserBtn1Pin; }
uint8_t userBtn2Pin() { return kUserBtn2Pin; }
uint8_t userBtn3Pin() { return kUserBtn3Pin; }
uint8_t limitBtn1Pin() { return kLimitBtn1Pin; }
uint8_t limitBtn2Pin() { return kLimitBtn2Pin; }

// TB6600 Stepper Motor Driver Control Functions
void setStepperPulse(bool state) {
  digitalWrite(kStepperPulsePin, state ? HIGH : LOW);
}

void setStepperDirection(bool clockwise) {
  digitalWrite(kStepperDirectionPin, clockwise ? HIGH : LOW);
}

void setStepperEnable(bool enabled) {
  // TB6600: LOW = enabled, HIGH = disabled
  digitalWrite(kStepperEnablePin, enabled ? LOW : HIGH);
}

uint8_t stepperPulsePin() { return kStepperPulsePin; }
uint8_t stepperDirectionPin() { return kStepperDirectionPin; }
uint8_t stepperEnablePin() { return kStepperEnablePin; }

}  // namespace hal

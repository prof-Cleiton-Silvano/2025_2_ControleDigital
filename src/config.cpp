/**
 * @file config.cpp
 * @brief Implementação das funções auxiliares de hardware
 */

#include <Arduino.h>
#include "config.h"

void initHardwarePins() {
  // Configura LED built-in como saída
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  digitalWrite(PIN_LED_BUILTIN, LOW);

  // Configura botões de usuário com pull-up interno (ativo em LOW)
  pinMode(PIN_USER_BTN_1, INPUT_PULLUP);
  pinMode(PIN_USER_BTN_2, INPUT_PULLUP);
  pinMode(PIN_USER_BTN_3, INPUT_PULLUP);

  // Configura sensores de fim de curso com pull-down (ativo em HIGH)
  pinMode(PIN_LIMIT_SWITCH_1, INPUT_PULLDOWN);
  pinMode(PIN_LIMIT_SWITCH_2, INPUT_PULLDOWN);

  // Configura pinos do driver TB6600
  pinMode(PIN_STEPPER_PULSE, OUTPUT);
  pinMode(PIN_STEPPER_DIRECTION, OUTPUT);
  pinMode(PIN_STEPPER_ENABLE, OUTPUT);
  
  // Estado inicial: motor desabilitado
  digitalWrite(PIN_STEPPER_PULSE, LOW);
  digitalWrite(PIN_STEPPER_DIRECTION, LOW);
  digitalWrite(PIN_STEPPER_ENABLE, HIGH);  // HIGH = desabilitado no TB6600
}

bool readLimitSwitch1() {
  // Retorna true se o sensor está ativo (HIGH)
  return digitalRead(PIN_LIMIT_SWITCH_1) == HIGH;
}

bool readLimitSwitch2() {
  // Retorna true se o sensor está ativo (HIGH)
  return digitalRead(PIN_LIMIT_SWITCH_2) == HIGH;
}

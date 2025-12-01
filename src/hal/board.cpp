#include <Arduino.h>

#include "hal/board.h"

namespace hal {

void initBoard() {
  // Inicializa LED built-in
  pinMode(kBuiltinLedPin, OUTPUT);
  digitalWrite(kBuiltinLedPin, LOW);

  // Inicializa botões de usuário como pull-up (ativo em LOW)
  pinMode(kUserBtn1Pin, INPUT_PULLUP);
  pinMode(kUserBtn2Pin, INPUT_PULLUP);
  pinMode(kUserBtn3Pin, INPUT_PULLUP);

  // Inicializa botões de fim de curso como pull-down (ativo em HIGH)
  pinMode(kLimitBtn1Pin, INPUT_PULLDOWN);
  pinMode(kLimitBtn2Pin, INPUT_PULLDOWN);

  // Inicializa pinos do driver TB6600 do motor de passo
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

// Leitura de botões de usuário (retorna true quando pressionado)
bool readUserBtn1() { return digitalRead(kUserBtn1Pin) == LOW; }
bool readUserBtn2() { return digitalRead(kUserBtn2Pin) == LOW; }
bool readUserBtn3() { return digitalRead(kUserBtn3Pin) == LOW; }

// Leitura de fim de curso (retorna true quando ativado)
bool readLimitBtn1() { return digitalRead(kLimitBtn1Pin) == HIGH; }
bool readLimitBtn2() { return digitalRead(kLimitBtn2Pin) == HIGH; }

}  // namespace hal

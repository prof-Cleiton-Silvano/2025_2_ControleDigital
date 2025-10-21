#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Mensagem de entrada do sensor de toque para o controlador
// Representa a referência (setpoint) ou entrada do sistema de controle
struct TouchInputMessage {
  int32_t touchValue;      // Valor bruto do sensor capacitivo (0-100)
  uint8_t touchZone;       // Zona de toque identificada (0=nenhum, 1=leve, 2=médio, 3=forte)
  TickType_t timestamp;    // Timestamp para análise temporal
};

// Envia mensagem de toque para o controlador
// ticksToWait: tempo de espera se a fila estiver cheia
bool sendTouchInputMessage(const TouchInputMessage& msg, TickType_t ticksToWait = portMAX_DELAY);

// Inicia a task de controle digital
// Esta task implementa a função de transferência do sistema de controle
// Prioridade intermediária entre sensor (entrada) e atuador (saída)
void startControlTask(UBaseType_t priority);

}  // namespace tasks

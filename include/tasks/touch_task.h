#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Inicia a task de leitura do sensor de toque capacitivo ESP32.
// Esta task monitora continuamente o sensor e classifica o toque em zonas
// baseadas em thresholds (limiares), enviando mensagens para o controlador.
// 
// O sensor funciona como ENTRADA do sistema de controle digital.
void startTouchTask(UBaseType_t priority);

}  // namespace tasks

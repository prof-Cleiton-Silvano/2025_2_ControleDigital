#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Starts the task that periodically envia comandos para o motor de passo.
void startStepperCommandTask(UBaseType_t priority);

}  // namespace tasks

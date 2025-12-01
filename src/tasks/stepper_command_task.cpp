#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/stepper_command_task.h"
#include "tasks/stepper_task.h"

namespace tasks {
namespace {

// Configurações de movimento
constexpr int32_t kStepsPerCommand = 500;       // Número de passos por movimento
constexpr float kSpeedStepsPerSec = 500.0f;     // Velocidade: 500 passos/segundo
constexpr float kAccelStepsPerSecSec = 200.0f;  // Aceleração: 200 passos/segundo²
constexpr TickType_t kDirectionSwapDelay = pdMS_TO_TICKS(5000);  // 5 segundos entre movimentos

void stepperCommandTask(void* /*params*/) {
  bool moveForward = true;
  StepperMessage msg{};
  
  for (;;) {
    // Configura mensagem para movimento relativo
    msg.targetPosition = moveForward ? kStepsPerCommand : -kStepsPerCommand;
    msg.speedInStepsPerSec = kSpeedStepsPerSec;
    msg.accelInStepsPerSecSec = kAccelStepsPerSecSec;
    msg.isRelative = true;  // Movimento relativo à posição atual
    
    // Envia comando para o stepper task
    sendStepperMessage(msg, portMAX_DELAY);

    // Aguarda antes de inverter direção
    vTaskDelay(kDirectionSwapDelay);

    // Inverte direção para próximo movimento
    moveForward = !moveForward;
  }
}

}  // namespace

void startStepperCommandTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 2048;
  xTaskCreate(
      stepperCommandTask,
      "stepper_cmd",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

}  // namespace tasks

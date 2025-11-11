#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/stepper_command_task.h"
#include "tasks/stepper_task.h"

namespace tasks {
namespace {

constexpr uint32_t kStepsPerCommand = 400;
constexpr uint32_t kPulseIntervalUs = 1000UL;  // 1000 ms entre pulsos
constexpr TickType_t kDirectionSwapDelay = pdMS_TO_TICKS(5000);

void stepperCommandTask(void* /*params*/) {
  StepperDirection direction = StepperDirection::Clockwise;
  StepperMessage msg{}; // TODO: Verificar overflow de fila
  
  for (;;) {
    // Monta mensagem esperada pelo stepper_task e envia para a fila
    msg.steps = kStepsPerCommand;
    msg.intervalUs = kPulseIntervalUs;
    msg.direction = direction;
    sendStepperMessage(msg, portMAX_DELAY);

    vTaskDelay(kDirectionSwapDelay);

    direction = (direction == StepperDirection::Clockwise)
                    ? StepperDirection::CounterClockwise
                    : StepperDirection::Clockwise;
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

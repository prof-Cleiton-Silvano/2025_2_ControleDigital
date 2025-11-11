#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/stepper_command_task.h"
#include "tasks/stepper_task.h"

namespace tasks {
namespace {

constexpr uint32_t kStepsPerCommand = 500;
constexpr uint32_t kPulseIntervalUs = 800UL;  // 1000 ms entre pulsos
constexpr TickType_t kDirectionSwapDelay = pdMS_TO_TICKS(5000);
constexpr uint32_t kRampSteps = 75;
constexpr uint32_t kRampIntervalMultiplier = 2;

void stepperCommandTask(void* /*params*/) {
  StepperDirection direction = StepperDirection::Clockwise;
  StepperMessage msg{};
  
  for (;;) {
    // Monta mensagem esperada pelo stepper_task e envia para a fila
    msg.steps = kStepsPerCommand;
    msg.intervalUs = kPulseIntervalUs;
    msg.direction = direction;
    msg.accelRampSteps = kRampSteps;
    msg.decelRampSteps = kRampSteps;
    msg.accelStartIntervalUs = kPulseIntervalUs * kRampIntervalMultiplier;
    msg.decelEndIntervalUs = kPulseIntervalUs * kRampIntervalMultiplier;
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

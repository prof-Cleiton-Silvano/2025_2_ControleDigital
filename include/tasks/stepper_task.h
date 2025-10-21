#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Direction for stepper motor movement
enum class StepperDirection : uint8_t {
  Clockwise = 0,
  CounterClockwise = 1
};

// Message structure sent to the stepper task
struct StepperMessage {
  uint32_t steps;                    // Number of steps to move
  uint32_t intervalUs;               // Interval between steps in microseconds
  StepperDirection direction;        // Direction of movement
};

// Enqueue a message to the stepper task. Returns true on success.
// ticksToWait: time to wait if queue is full (default: wait forever)
bool sendStepperMessage(const StepperMessage& msg, TickType_t ticksToWait = portMAX_DELAY);

// Starts the FreeRTOS task that controls the stepper motor via TB6600 driver.
// Use high priority (e.g., tskIDLE_PRIORITY + 3) for precise timing.
void startStepperTask(UBaseType_t priority);

}  // namespace tasks

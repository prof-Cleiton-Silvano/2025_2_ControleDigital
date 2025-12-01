#pragma once

#include <freertos/FreeRTOS.h>

namespace tasks {

// Message structure sent to the stepper task
struct StepperMessage {
  int32_t targetPosition;      // Target position in steps (absolute or relative)
  float speedInStepsPerSec;    // Movement speed in steps per second
  float accelInStepsPerSecSec; // Acceleration in steps per second^2
  bool isRelative;             // true = relative move, false = absolute move
};

// Enqueue a message to the stepper task. Returns true on success.
// ticksToWait: time to wait if queue is full (default: wait forever)
bool sendStepperMessage(const StepperMessage& msg, TickType_t ticksToWait = portMAX_DELAY);

// Starts the FreeRTOS task that controls the stepper motor via TB6600 driver using ESP-FlexyStepper.
// Use high priority (e.g., tskIDLE_PRIORITY + 3) for precise timing.
void startStepperTask(UBaseType_t priority);

// Get the current position of the stepper motor
int32_t getStepperPosition();

// Emergency stop the stepper motor
void emergencyStopStepper();

// Enable/disable the stepper motor
void setStepperEnabled(bool enabled);

}  // namespace tasks

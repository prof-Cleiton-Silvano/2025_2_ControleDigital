#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "tasks/stepper_task.h"
#include "hal/board.h"

namespace tasks {
namespace {

// Queue for stepper motor commands
QueueHandle_t xStepperQueue = nullptr;

// Maximum number of queued commands
constexpr size_t kStepperQueueLength = 8;

// Minimum safe interval to prevent motor stalling (microseconds)
constexpr uint32_t kMinIntervalUs = 50;

// Stepper task implementation - processes movement commands from queue
void stepperTask(void* /*params*/) {
  // Create the queue if not already created
  if (xStepperQueue == nullptr) {
    xStepperQueue = xQueueCreate(kStepperQueueLength, sizeof(StepperMessage));
  }

  // Motor starts disabled
  hal::setStepperEnable(false);

  StepperMessage msg;
  for (;;) {
    // Wait for a command from the queue
    if (xStepperQueue != nullptr && xQueueReceive(xStepperQueue, &msg, portMAX_DELAY) == pdTRUE) {
      // Validate interval to prevent motor damage
      uint32_t safeInterval = msg.intervalUs;
      if (safeInterval < kMinIntervalUs) {
        safeInterval = kMinIntervalUs;
      }

      // Set direction
      bool clockwise = (msg.direction == StepperDirection::Clockwise);
      hal::setStepperDirection(clockwise);

      // Enable motor
      hal::setStepperEnable(true);

      // Small delay to allow driver to stabilize after enable
      delayMicroseconds(10);

      // Execute steps with precise timing
      for (uint32_t i = 0; i < msg.steps; i++) {
        // Generate pulse: HIGH
        hal::setStepperPulse(true);
        delayMicroseconds(5);  // Minimum pulse width for TB6600 (2.5us typical)

        // Generate pulse: LOW
        hal::setStepperPulse(false);

        // Wait for the specified interval (minus pulse time)
        if (safeInterval > 5) {
          delayMicroseconds(safeInterval - 5);
        }

        // Allow other high-priority tasks to run periodically
        // Check every 100 steps to maintain responsiveness
        if (i % 100 == 0) {
          taskYIELD();
        }
      }

      // Disable motor after movement completes (optional - saves power)
      // Comment out the next line if you want the motor to hold position
      // hal::setStepperEnable(false);
    }
  }
}

}  // namespace

void startStepperTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 2048;
  xTaskCreate(
      stepperTask,
      "stepper_motor",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

bool sendStepperMessage(const StepperMessage& msg, TickType_t ticksToWait) {
  if (xStepperQueue == nullptr) {
    // Try to create queue lazily if task hasn't initialized it yet
    xStepperQueue = xQueueCreate(kStepperQueueLength, sizeof(StepperMessage));
    if (xStepperQueue == nullptr) return false;
  }
  return xQueueSend(xStepperQueue, &msg, ticksToWait) == pdTRUE;
}

}  // namespace tasks

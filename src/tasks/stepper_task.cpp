#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <ESP_FlexyStepper.h>

#include "tasks/stepper_task.h"
#include "hal/board.h"

namespace tasks {
namespace {

// Queue for stepper motor commands
QueueHandle_t xStepperQueue = nullptr;

// Maximum number of queued commands
constexpr size_t kStepperQueueLength = 8;

// ESP-FlexyStepper instance
ESP_FlexyStepper stepper;

// Stepper task implementation - processes movement commands from queue
void stepperTask(void* /*params*/) {
  // Create the queue if not already created
  if (xStepperQueue == nullptr) {
    xStepperQueue = xQueueCreate(kStepperQueueLength, sizeof(StepperMessage));
  }

  // Configure stepper motor using ESP-FlexyStepper
  stepper.connectToPins(hal::kStepperPulsePin, hal::kStepperDirectionPin);
  
  // Configure enable pin (TB6600: LOW = enabled, HIGH = disabled)
  pinMode(hal::kStepperEnablePin, OUTPUT);
  digitalWrite(hal::kStepperEnablePin, HIGH);  // Start disabled
  
  // Set default speed and acceleration (can be overridden by messages)
  stepper.setSpeedInStepsPerSecond(500);
  stepper.setAccelerationInStepsPerSecondPerSecond(200);
  
  StepperMessage msg;
  for (;;) {
    // Process stepper service (handles movement timing)
    stepper.processMovement();
    
    // Check for new commands (non-blocking)
    if (xStepperQueue != nullptr && xQueueReceive(xStepperQueue, &msg, 0) == pdTRUE) {
      // Enable motor before movement
      digitalWrite(hal::kStepperEnablePin, LOW);
      
      // Configure speed and acceleration
      stepper.setSpeedInStepsPerSecond(msg.speedInStepsPerSec);
      stepper.setAccelerationInStepsPerSecondPerSecond(msg.accelInStepsPerSecSec);
      
      // Execute movement (relative or absolute)
      if (msg.isRelative) {
        stepper.setTargetPositionRelativeInSteps(msg.targetPosition);
      } else {
        stepper.setTargetPositionInSteps(msg.targetPosition);
      }
      
      // Wait for movement to complete
      while (!stepper.motionComplete()) {
        stepper.processMovement();
        taskYIELD();  // Allow other tasks to run
      }
      
      // Optional: disable motor after movement to save power
      // digitalWrite(hal::kStepperEnablePin, HIGH);
    }
    
    // Small delay to prevent tight loop and allow other tasks
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

}  // namespace

void startStepperTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 4096;  // Increased for FlexyStepper
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

int32_t getStepperPosition() {
  return stepper.getCurrentPositionInSteps();
}

void emergencyStopStepper() {
  stepper.emergencyStop();
}

void setStepperEnabled(bool enabled) {
  // TB6600: LOW = enabled, HIGH = disabled
  digitalWrite(hal::kStepperEnablePin, enabled ? LOW : HIGH);
}

}  // namespace tasks

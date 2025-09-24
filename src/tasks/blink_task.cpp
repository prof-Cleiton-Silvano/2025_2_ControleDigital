#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hal/board.h"
#include "tasks/blink_task.h"

namespace tasks {
namespace {

// Periodic LED toggle implemented as a dedicated task.
void blinkTask(void* /*params*/) {
  constexpr TickType_t kBlinkDelayTicks = pdMS_TO_TICKS(500);
  bool ledOn = false;

  for (;;) {
    ledOn = !ledOn;
    hal::setBuiltinLed(ledOn);
    vTaskDelay(kBlinkDelayTicks);
  }
}

}  // namespace

void startBlinkTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 2048;
  xTaskCreate(
      blinkTask,
      "blink_led",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

}  // namespace tasks

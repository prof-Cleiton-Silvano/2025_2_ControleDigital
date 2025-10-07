#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/touch_task.h"
#include "tasks/display_task.h"

namespace tasks {
namespace {

// Use a touch-capable pin on ESP32. Adjust if your board differs.
// T0 usually maps to GPIO4 on many ESP32 boards.
constexpr uint8_t kTouchPin = T0; // Arduino constant for touch 0
constexpr TickType_t kPollDelay = pdMS_TO_TICKS(100);

void touchTask(void* /*params*/) {
  // No special initialization required for touchRead, but ensure pinMode is not interfering
  // We'll just periodically call touchRead and interpret the result.
  long lastValue = -1;

  for (;;) {
    long v = touchRead(kTouchPin);
    // Simple thresholding: treat non-zero as touched; you may tune the threshold
    bool touched = (v >= 0 && v < 40); // lower values typically mean stronger touch; tune as needed

    // Only send on change to reduce traffic
    if (lastValue == -1 || (touched && lastValue != 1) || (!touched && lastValue != 0)) {
      tasks::DisplayMessage msg{};
      msg.cmd = tasks::DisplayCmd::WriteChar;
      msg.col = 3;
      msg.row = 0;
      msg.c = touched ? 'H' : 'L';
      tasks::sendDisplayMessage(msg, 0);
    }

    lastValue = touched ? 1 : 0;
    vTaskDelay(kPollDelay);
  }
}

}  // namespace

void startTouchTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 2048;
  xTaskCreate(
      touchTask,
      "touch_task",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

}  // namespace tasks

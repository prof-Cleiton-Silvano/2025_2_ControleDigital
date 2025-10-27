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

// Debounce/tolerance between touch triggers
constexpr TickType_t kTouchDebounce = pdMS_TO_TICKS(300);

// Stepper movement parameters
constexpr uint32_t kStepsPerTouch = 200;         // passos por toque
constexpr uint32_t kSlowIntervalUs = 1500;       // intervalo lento entre pulsos (us)
constexpr uint32_t kFastIntervalUs = 300;        // intervalo rápido entre pulsos (us)

// Limiar de toque: valores menores indicam toque mais forte no ESP32
constexpr long kTouchThreshold = 40;  // ajuste se necessário

void touchTask(void* /*params*/) {
  // No special initialization required for touchRead, but ensure pinMode is not interfering
  // We'll just periodically call touchRead and interpret the result.
  long lastValue = -1;
  bool lastTouched = false;
  TickType_t lastTriggerTick = 0;

  // Menu state: simple list of options. Tap -> next option, Hold -> select
  const char* menuItems[] = {"Art", "Clock"};
  constexpr size_t kMenuCount = sizeof(menuItems) / sizeof(menuItems[0]);
  size_t menuIndex = 0;

  // Threshold to consider a hold as "select" (ms)
  constexpr TickType_t kSelectHoldMs = pdMS_TO_TICKS(1500);

  // Helper: write a C-string to display at given row starting at col 0
  auto writeStringToDisplay = [&](uint8_t row, const char* s) {
    // Create a 16-char buffer padded with spaces to fully overwrite the line.
    char line[17];
    for (int i = 0; i < 16; ++i) line[i] = ' ';
    line[16] = '\0';
    // Copy up to 16 characters
    for (int i = 0; i < 16 && s[i] != '\0'; ++i) line[i] = s[i];

    // Send each character with a short timeout so we don't block too long
    // but also avoid dropping characters when queue is momentarily full.
    const TickType_t kCharTimeout = pdMS_TO_TICKS(20);
    for (uint8_t c = 0; c < 16; ++c) {
      tasks::DisplayMessage msg{};
      msg.cmd = tasks::DisplayCmd::WriteChar;
      msg.col = c;
      msg.row = row;
      msg.c = line[c];
      // small timeout; queue length was increased to reduce drops
      tasks::sendDisplayMessage(msg, kCharTimeout);
    }
  };

  // Art mode: simple ASCII animation on 16x2 LCD. Runs until user touches the sensor.
  auto runArtMode = [&]() {
    // Wait until touch released (so we don't immediately exit if finger still on pad)
    while (true) {
      long v = touchRead(kTouchPin);
      bool touchedNow = (v >= 0 && v < kTouchThreshold);
      if (!touchedNow) break;
      vTaskDelay(pdMS_TO_TICKS(50));
    }

      // Two-frame art (each string must be <=16 chars for 16x2 LCD)
      const char* frames[][2] = {
          {"*      *      ", "   *      *    "},
          {"  *    *      ", "    *    *     "},
      };
    constexpr size_t kFrameCount = sizeof(frames) / sizeof(frames[0]);

    size_t idx = 0;
    for (;;) {
      writeStringToDisplay(0, frames[idx][0]);
      writeStringToDisplay(1, frames[idx][1]);

      // brief delay and check for touch to exit
      for (int t = 0; t < 4; ++t) {
        long v = touchRead(kTouchPin);
        bool touchedNow = (v >= 0 && v < kTouchThreshold);
        if (touchedNow) {
          // exit art mode
          writeStringToDisplay(0, "Exiting Art...");
          vTaskDelay(pdMS_TO_TICKS(1000));
          return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
      }

      idx = (idx + 1) % kFrameCount;
    }
  };

  // Clock mode: show running HH:MM:SS based on millis() until user touches to exit
  auto runClockMode = [&]() {
    // Wait until touch released first
    while (true) {
      long v = touchRead(kTouchPin);
      bool touchedNow = (v >= 0 && v < kTouchThreshold);
      if (!touchedNow) break;
      vTaskDelay(pdMS_TO_TICKS(50));
    }

    unsigned long startMs = millis();
    for (;;) {
      unsigned long elapsed = (millis() - startMs) / 1000; // seconds
      unsigned long hh = (elapsed / 3600) % 24;
      unsigned long mm = (elapsed / 60) % 60;
      unsigned long ss = elapsed % 60;

      char line0[17];
      char line1[17];
      snprintf(line0, sizeof(line0), "Clock Mode       ");
      snprintf(line1, sizeof(line1), "%02lu:%02lu:%02lu        ", hh, mm, ss);
      writeStringToDisplay(0, line0);
      writeStringToDisplay(1, line1);

      // check touch to exit
      for (int t = 0; t < 10; ++t) {
        long v = touchRead(kTouchPin);
        bool touchedNow = (v >= 0 && v < kTouchThreshold);
        if (touchedNow) {
          writeStringToDisplay(0, "Exiting Clock...");
          vTaskDelay(pdMS_TO_TICKS(1000));
          return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
      }
    }
  };

  // Initial display of menu
  // Clear screen once and show initial menu
  tasks::DisplayMessage clr{};
  clr.cmd = tasks::DisplayCmd::Clear;
  tasks::sendDisplayMessage(clr, pdMS_TO_TICKS(200));
  writeStringToDisplay(0, "Menu:");
  writeStringToDisplay(1, menuItems[menuIndex]);

  TickType_t touchStartTick = 0;
  for (;;) {
    long v = touchRead(kTouchPin);
    bool touched = (v >= 0 && v < kTouchThreshold);

    TickType_t now = xTaskGetTickCount();

    // Edge: touch started
    if (touched && !lastTouched) {
      touchStartTick = now;
    }

    // Edge: touch released
    if (!touched && lastTouched) {
      TickType_t held = now - touchStartTick;
      if (held >= kSelectHoldMs) {
        // Select current menu item
        const char* selected = menuItems[menuIndex];
        char buf[17];
        snprintf(buf, sizeof(buf), "Selected:%s", selected);
        writeStringToDisplay(0, buf);
        vTaskDelay(pdMS_TO_TICKS(400));

        // Dispatch action per menu index (Art=0, Clock=1)
        if (menuIndex == 0) {
          runArtMode();
        } else if (menuIndex == 1) {
          runClockMode();
        } else {
          // Shouldn't happen but provide feedback
          writeStringToDisplay(0, "Invalid option");
          vTaskDelay(pdMS_TO_TICKS(400));
        }

        // After action, show menu again
        writeStringToDisplay(0, "Menu:");
        writeStringToDisplay(1, menuItems[menuIndex]);
      } else {
        // Short tap -> advance menu
        menuIndex = (menuIndex + 1) % kMenuCount;
        writeStringToDisplay(1, menuItems[menuIndex]);
      }
    }

    lastTouched = touched;
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

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/touch_task.h"
#include "tasks/display_task.h"
#include "tasks/stepper_task.h"

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

  // Alternância: primeiro toque -> lento horário; próximo -> rápido anti-horário
  bool nextSlowClockwise = true;

  for (;;) {
    long v = touchRead(kTouchPin);
    bool touched = (v >= 0 && v < kTouchThreshold);

    // Atualiza display em mudanças (opcional)
    if (lastValue == -1 || (touched && lastValue != 1) || (!touched && lastValue != 0)) {
      tasks::DisplayMessage dmsg{};
      dmsg.cmd = tasks::DisplayCmd::WriteChar;
      dmsg.col = 3;
      dmsg.row = 0;
      dmsg.c = touched ? 'H' : 'L';
      tasks::sendDisplayMessage(dmsg, 0);
    }

    // Detecta borda de subida (novo toque) com debounce
    TickType_t now = xTaskGetTickCount();
    if (touched && !lastTouched && (now - lastTriggerTick) >= kTouchDebounce) {
      tasks::StepperMessage smsg{};
      smsg.steps = kStepsPerTouch;
      if (nextSlowClockwise) {
        smsg.intervalUs = kSlowIntervalUs;
        smsg.direction = tasks::StepperDirection::Clockwise;
      } else {
        smsg.intervalUs = kFastIntervalUs;
        smsg.direction = tasks::StepperDirection::CounterClockwise;
      }

      // Envia comando ao motor de passo
      tasks::sendStepperMessage(smsg, 0);

      // Alterna para a próxima vez
      nextSlowClockwise = !nextSlowClockwise;
      lastTriggerTick = now;
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

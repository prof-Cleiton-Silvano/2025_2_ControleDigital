#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hal/board.h"
#include "tasks/blink_task.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <freertos/semphr.h>
#include "tasks/display_task.h"

namespace tasks {
namespace {

// LCD config (igual ao display_task.cpp)
constexpr uint8_t kLcdAddress = 0x27;
constexpr uint8_t kLcdColumns = 16;
constexpr uint8_t kLcdRows = 2;
constexpr uint8_t kSdaPin = 21;
constexpr uint8_t kSclPin = 22;
const char* palavra = "Controle Digital";
constexpr size_t palavra_len = 16; // "Controle Digital" tem 16 caracteres

void blinkTask(void* /*params*/) {
  constexpr TickType_t kBlinkDelayTicks = pdMS_TO_TICKS(1000);
  bool ledOn = false;
  size_t letra_idx = 0;

  // Inicializa LCD
  // Apenas aguarda a display task inicializar e gerenciar o LCD
  vTaskDelay(pdMS_TO_TICKS(500)); // Pequena espera para permitir init do display
  
  for (;;) {
    // Blink LED
    ledOn = !ledOn;
    hal::setBuiltinLed(ledOn);
    // (Display update disabled to allow menu to control the LCD)

    // letra_idx++;
    // if (letra_idx >= palavra_len) {
    //   vTaskDelay(kBlinkDelayTicks);
    //   // pede para display limpar
    //   tasks::DisplayMessage clr{};
    //   clr.cmd = tasks::DisplayCmd::Clear;
    //   tasks::sendDisplayMessage(clr, 0);
    //   letra_idx = 0;
    // }

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

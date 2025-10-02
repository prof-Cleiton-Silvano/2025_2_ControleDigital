#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hal/board.h"
#include "tasks/blink_task.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <freertos/semphr.h>

namespace tasks {
namespace {

// LCD config (igual ao display_task.cpp)
constexpr uint8_t kLcdAddress = 0x27;
constexpr uint8_t kLcdColumns = 16;
constexpr uint8_t kLcdRows = 2;
constexpr uint8_t kSdaPin = 21;
constexpr uint8_t kSclPin = 22;
LiquidCrystal_I2C lcd(kLcdAddress, kLcdColumns, kLcdRows);

SemaphoreHandle_t xLcdMutex = nullptr;

const char* palavra = "Controle Digital";
constexpr size_t palavra_len = 16; // "Controle Digital" tem 16 caracteres

void blinkTask(void* /*params*/) {
  constexpr TickType_t kBlinkDelayTicks = pdMS_TO_TICKS(500);
  bool ledOn = false;
  size_t letra_idx = 0;

  // Inicializa LCD
  Wire.begin(kSdaPin, kSclPin);
  lcd.init();
  lcd.backlight();
    lcd.clear();
    vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda LCD estabilizar

  // Cria o mutex se ainda não existe
  if (xLcdMutex == nullptr) {
    xLcdMutex = xSemaphoreCreateMutex();
  }

  for (;;) {
    ledOn = !ledOn;
    hal::setBuiltinLed(ledOn);


    // Protege acesso ao display com mutex
    if (xLcdMutex != nullptr && xSemaphoreTake(xLcdMutex, portMAX_DELAY) == pdTRUE) {
      lcd.setCursor(letra_idx, 0);
      lcd.print(palavra[letra_idx]);
      xSemaphoreGive(xLcdMutex);
    }

    letra_idx++;
    if (letra_idx >= palavra_len) {
      vTaskDelay(kBlinkDelayTicks);
      if (xLcdMutex != nullptr && xSemaphoreTake(xLcdMutex, portMAX_DELAY) == pdTRUE) {
        lcd.clear();
        xSemaphoreGive(xLcdMutex);
      }
      letra_idx = 0;
    }

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

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <LiquidCrystal_I2C.h>

#include "tasks/display_task.h"

namespace tasks {
namespace {

// I2C LCD configuration
constexpr uint8_t kLcdAddress = 0x27;  // Common I2C address for LCD modules
constexpr uint8_t kLcdColumns = 16;
constexpr uint8_t kLcdRows = 2;
constexpr uint8_t kSdaPin = 21;        // ESP32 default SDA pin
constexpr uint8_t kSclPin = 22;        // ESP32 default SCL pin

// Global LCD object
LiquidCrystal_I2C lcd(kLcdAddress, kLcdColumns, kLcdRows);

// Display task implementation - shows "Hello World" and a counter
void displayTask(void* /*params*/) {
  // Inicializa I2C e LCD
  Wire.begin(kSdaPin, kSclPin);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Não exibe nada, deixa controle para blink_task.cpp
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000)); // Mantém a tarefa viva
  }
}

}  // namespace

void startDisplayTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 4096;  // Larger stack for LCD operations
  xTaskCreate(
      displayTask,
      "lcd_display",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

}  // namespace tasks
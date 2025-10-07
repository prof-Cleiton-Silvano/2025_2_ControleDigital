#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/queue.h>
#include <Wire.h>

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

// Queue for display messages
QueueHandle_t xDisplayQueue = nullptr;

// Maximum number of queued messages
constexpr size_t kDisplayQueueLength = 16;

// Display task implementation - shows "Hello World" and a counter
void displayTask(void* /*params*/) {
  // Inicializa I2C e LCD
  Wire.begin(kSdaPin, kSclPin);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Cria a fila se ainda nao criada
  if (xDisplayQueue == nullptr) {
    xDisplayQueue = xQueueCreate(kDisplayQueueLength, sizeof(DisplayMessage));
  }

  DisplayMessage msg;
  for (;;) {
    // Espera por mensagens e processa
    if (xDisplayQueue != nullptr && xQueueReceive(xDisplayQueue, &msg, portMAX_DELAY) == pdTRUE) {
      switch (msg.cmd) {
        case DisplayCmd::WriteChar:
          if (msg.col < kLcdColumns && msg.row < kLcdRows) {
            lcd.setCursor(msg.col, msg.row);
            char buf[2] = {msg.c, '\0'};
            lcd.print(buf);
          }
          break;
        case DisplayCmd::Clear:
          lcd.clear();
          break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Pequena espera para evitar uso excessivo de CPU
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

bool sendDisplayMessage(const DisplayMessage& msg, TickType_t ticksToWait) {
  if (xDisplayQueue == nullptr) {
    // Try to create queue lazily if task hasn't initialized it yet
    xDisplayQueue = xQueueCreate(kDisplayQueueLength, sizeof(DisplayMessage));
    if (xDisplayQueue == nullptr) return false;
  }
  return xQueueSend(xDisplayQueue, &msg, ticksToWait) == pdTRUE;
}

}  // namespace tasks
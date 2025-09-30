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
  // Initialize I2C and LCD
  Wire.begin(kSdaPin, kSclPin);
  lcd.init();
  lcd.backlight();
  
  // Initial display setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Controle Digital");
  
  uint32_t counter = 0;
  constexpr TickType_t kUpdateDelayTicks = pdMS_TO_TICKS(1000);  // Update every 2 seconds
  
  for (;;) {
    // Update counter on second line
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(counter);
    lcd.print("    ");  // Clear any remaining characters
    
    counter++;
    vTaskDelay(kUpdateDelayTicks);
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
#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include "hal/board.h"
#include "tasks/blink_task.h"

// Application entry point: configure hardware and spawn the initial tasks.
void setup() {
  hal::initBoard();

  const UBaseType_t blinkPriority = tskIDLE_PRIORITY + 1;  // Low priority task.
  tasks::startBlinkTask(blinkPriority);
}

void loop() {
  vTaskDelay(portMAX_DELAY);  // Nothing left for the Arduino loop.
}

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include "hal/board.h"
#include "tasks/blink_task.h"
#include "tasks/display_task.h"
#include "tasks/touch_task.h"
#include "tasks/control_task.h"
#include "tasks/stepper_task.h"

// Application entry point: configure hardware and spawn the initial tasks.
void setup() {
  hal::initBoard();

  const UBaseType_t blinkPriority = tskIDLE_PRIORITY + 1;    // Low priority task.
  const UBaseType_t displayPriority = tskIDLE_PRIORITY + 1;  // Low priority task.
  const UBaseType_t touchPriority = tskIDLE_PRIORITY + 1;    // Low priority - sensor input.
  const UBaseType_t controlPriority = tskIDLE_PRIORITY + 2;  // Medium priority - control algorithm.
  const UBaseType_t stepperPriority = tskIDLE_PRIORITY + 3;  // High priority - actuator output.

  // Start display first so it initializes the LCD and the message queue.
  // tasks::startDisplayTask(displayPriority);
  tasks::startBlinkTask(blinkPriority);
  
  // Sistema de controle digital: Sensor → Controlador → Atuador
  // tasks::startTouchTask(touchPriority);      // ENTRADA: lê sensor capacitivo
  // tasks::startControlTask(csontrolPriority);  // PROCESSAMENTO: implementa função de transferência
  tasks::startStepperTask(stepperPriority);  // SAÍDA: controla motor de passo
}

void loop() {
  vTaskDelay(portMAX_DELAY);  // Nothing left for the Arduino loop.
}

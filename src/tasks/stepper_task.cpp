/**
 * @file stepper_task.cpp
 * @brief Task de controle do motor de passo com monitoramento de fim de curso
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ESP_FlexyStepper.h>
#include "config.h"
#include "tasks/stepper_task.h"

static ESP_FlexyStepper stepper;

// Verifica se algum fim de curso está ativo
static bool checkLimitSwitches() {
  return readLimitSwitch1() || readLimitSwitch2();
}

// Para o motor imediatamente
static void stopMotor() {
  stepper.emergencyStop();
  digitalWrite(PIN_STEPPER_ENABLE, HIGH);
}

// Executa movimento e monitora fim de curso
// Retorna true se completou, false se interrompido
static bool executeMove(int32_t steps) {
  digitalWrite(PIN_STEPPER_ENABLE, LOW);
  stepper.setTargetPositionRelativeInSteps(steps);
  
  while (!stepper.motionComplete()) {
    stepper.processMovement();
    
    if (checkLimitSwitches()) {
      stopMotor();
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  
  return true;
}

// Loop principal - Movimento dummy alternado
static void stepperTask(void* params) {
  (void)params;
  
  stepper.connectToPins(PIN_STEPPER_PULSE, PIN_STEPPER_DIRECTION);
  stepper.setSpeedInStepsPerSecond(STEPPER_SPEED);
  stepper.setAccelerationInStepsPerSecondPerSecond(STEPPER_ACCEL);
  digitalWrite(PIN_STEPPER_ENABLE, HIGH);
  
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  for (;;) {
    // Define passos conforme configuração: curso grande ou até fim de curso
    const int32_t kHuge = 1000000; // grande o suficiente para atingir o fim de curso
    const int32_t forward = STEPPER_TRAVEL_UNTIL_LIMIT ? kHuge : STEPPER_TRAVEL_STEPS;
    const int32_t backward = STEPPER_TRAVEL_UNTIL_LIMIT ? -kHuge : -STEPPER_TRAVEL_STEPS;

    // Move para frente
    if (!executeMove(forward)) {
      while (checkLimitSwitches()) {
        vTaskDelay(pdMS_TO_TICKS(100));
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(STEPPER_MOVE_DELAY_MS));
    
    // Move para trás
    if (!executeMove(backward)) {
      while (checkLimitSwitches()) {
        vTaskDelay(pdMS_TO_TICKS(100));
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(STEPPER_MOVE_DELAY_MS));
  }
}

void startStepperTask(UBaseType_t priority) {
  xTaskCreate(stepperTask, "Stepper_Motor", 4096, nullptr, priority, nullptr);
}

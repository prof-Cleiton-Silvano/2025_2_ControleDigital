/**
 * @file sensor_task.cpp
 * @brief Implementação da task de leitura do sensor óptico
 * 
 * Monitora o sensor EE-SH3 Photomicrosensor e detecta mudanças de estado.
 * Conta quantas vezes o feixe foi bloqueado.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"
#include "tasks/sensor_task.h"

// Variáveis globais protegidas (acessadas apenas pela task)
static bool sensorState = false;
static uint32_t blockCount = 0;
static bool lastState = false;

// Loop principal da task do sensor
static void sensorTask(void* params) {
  (void)params;
  
  // Aguarda inicialização
  vTaskDelay(pdMS_TO_TICKS(100));
  
  // Lê estado inicial
  lastState = readPhotoSensor();
  sensorState = lastState;
  
  Serial.println("Sensor Task: Iniciada");
  Serial.printf("Estado inicial: %s\n", sensorState ? "Desbloqueado" : "Bloqueado");
  
  for (;;) {
    // Lê estado atual do sensor
    bool currentState = readPhotoSensor();
    
    // Detecta mudança de estado
    if (currentState != lastState) {
      sensorState = currentState;
      
      // Detecta transição de desbloqueado para bloqueado
      if (!currentState && lastState) {
        blockCount++;
        Serial.printf("Sensor bloqueado! Contagem: %lu\n", blockCount);
      }
      // Detecta transição de bloqueado para desbloqueado
      else if (currentState && !lastState) {
        Serial.println("Sensor desbloqueado!");
      }
      
      lastState = currentState;
    }
    
    // Aguarda intervalo de leitura
    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

void startSensorTask(UBaseType_t priority) {
  xTaskCreate(sensorTask, "Photo_Sensor", 2048, nullptr, priority, nullptr);
}

bool getSensorState() {
  return sensorState;
}

uint32_t getSensorBlockCount() {
  return blockCount;
}

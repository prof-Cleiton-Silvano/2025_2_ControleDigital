/**
 * @file main.cpp
 * @brief Programa principal - Sistema de Controle de Motor de Passo
 * 
 * Sistema baseado em FreeRTOS com duas tasks principais:
 * 1. Blink Task - LED de diagnóstico (baixa prioridade)
 * 2. Stepper Task - Controle do motor com fim de curso (alta prioridade)
 * 
 * Hardware: ESP32 Dev Kit V1 + Driver TB6600 + Motor de Passo
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"
#include "tasks/blink_task.h"
#include "tasks/stepper_task.h"

/**
 * @brief Função de inicialização do Arduino/ESP32
 * 
 * Configura o hardware e cria as tasks do FreeRTOS.
 * Após esta função, o scheduler do FreeRTOS assume o controle.
 */
void setup() {
  // Inicializa porta serial para debug (opcional)
  Serial.begin(115200);
  Serial.println("=== Sistema de Controle de Motor de Passo ===");
  Serial.println("Inicializando hardware...");
  
  // Configura todos os pinos de I/O
  initHardwarePins();
  
  Serial.println("Criando tasks FreeRTOS...");
  
  // Cria task de diagnóstico (LED piscante) - Baixa prioridade
  startBlinkTask(PRIORITY_BLINK_TASK);
  
  // Cria task de controle do motor - Alta prioridade
  startStepperTask(PRIORITY_STEPPER_TASK);
  
  Serial.println("Sistema iniciado com sucesso!");
  Serial.println("Tasks ativas:");
  Serial.println("  - LED Blink (prioridade 1)");
  Serial.println("  - Stepper Motor (prioridade 3)");
}

/**
 * @brief Loop principal do Arduino (não utilizado)
 * 
 * Com FreeRTOS, o loop() não é usado. O scheduler gerencia as tasks.
 * Mantemos um delay infinito para liberar CPU.
 */
void loop() {
  // Suspende indefinidamente - FreeRTOS gerencia tudo
  vTaskDelay(portMAX_DELAY);
}

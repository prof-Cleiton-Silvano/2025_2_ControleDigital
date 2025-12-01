/**
 * @file blink_task.cpp
 * @brief Implementação da task de diagnóstico do LED
 * 
 * Task simples que alterna o estado do LED built-in a cada 1 segundo.
 * Serve como indicador visual de que o sistema está funcionando.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"
#include "tasks/blink_task.h"

/**
 * @brief Loop principal da task de blink
 * 
 * Alterna o LED built-in entre ligado e desligado a cada segundo.
 * Usa vTaskDelay para não consumir CPU desnecessariamente.
 */
static void blinkTask(void* params) {
  (void)params;  // Parâmetro não utilizado
  
  bool ledState = false;
  
  for (;;) {
    // Alterna estado do LED
    ledState = !ledState;
    digitalWrite(PIN_LED_BUILTIN, ledState ? HIGH : LOW);
    
    // Aguarda 1 segundo antes de alternar novamente
    vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL_MS));
  }
}

void startBlinkTask(UBaseType_t priority) {
  // Cria a task com stack de 1KB (suficiente para operação simples)
  xTaskCreate(
    blinkTask,           // Função da task
    "LED_Blink",         // Nome para debug
    1024,                // Stack size (palavras de 32 bits)
    nullptr,             // Parâmetro (não usado)
    priority,            // Prioridade
    nullptr              // Handle (não precisamos)
  );
}

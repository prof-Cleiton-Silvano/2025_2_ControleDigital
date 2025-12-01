/**
 * @file blink_task.h
 * @brief Task de diagnóstico - LED piscante
 * 
 * Task de baixa prioridade que pisca o LED built-in a cada 1 segundo.
 * Útil para verificar se o sistema está operacional.
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Inicia a task do LED piscante
 * @param priority Prioridade da task (usar valor baixo)
 */
void startBlinkTask(UBaseType_t priority);

/**
 * @file sensor_task.h
 * @brief Task de leitura do sensor óptico EE-SH3
 * 
 * Task responsável por monitorar o sensor óptico transmissivo
 * e detectar interrupções no feixe de luz.
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Inicia a task de leitura do sensor óptico
 * @param priority Prioridade da task
 */
void startSensorTask(UBaseType_t priority);

/**
 * @brief Obtém o último estado lido do sensor
 * @return true se o feixe está desbloqueado, false se bloqueado
 */
bool getSensorState();

/**
 * @brief Obtém o contador de bloqueios detectados
 * @return Número de vezes que o sensor foi bloqueado desde o início
 */
uint32_t getSensorBlockCount();

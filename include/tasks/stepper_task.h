/**
 * @file stepper_task.h
 * @brief Task de controle do motor de passo
 * 
 * Task responsável por controlar o motor de passo usando a biblioteca
 * ESP-FlexyStepper. Monitora os sensores de fim de curso e executa
 * movimentos sequenciais.
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Inicia a task de controle do motor de passo
 * @param priority Prioridade da task (usar valor alto para tempo real)
 */
void startStepperTask(UBaseType_t priority);

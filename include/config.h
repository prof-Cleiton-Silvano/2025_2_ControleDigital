/**
 * @file config.h
 * @brief Configurações centralizadas do projeto - Pinos e Constantes
 * 
 * Este arquivo contém todas as definições de pinos de I/O e constantes
 * utilizadas no projeto. Centralizando essas definições, facilitamos
 * a manutenção e configuração do hardware.
 * 
 * Placa: ESP32 Dev Kit V1
 */

#pragma once

#include <stdint.h>

// ============================================================================
// DEFINIÇÕES DE PINOS - ESP32 Dev Kit V1
// ============================================================================

// LED Built-in
// Conectado ao LED integrado na placa ESP32
constexpr uint8_t PIN_LED_BUILTIN = 2;

// Botões de Usuário (com pull-up interno, ativo em LOW)
// Conectar botões entre o pino e GND
constexpr uint8_t PIN_USER_BTN_1 = 32;
constexpr uint8_t PIN_USER_BTN_2 = 33;
constexpr uint8_t PIN_USER_BTN_3 = 25;

// Sensores de Fim de Curso (com pull-down, ativo em HIGH)
// Conectar sensores entre o pino e 3.3V
constexpr uint8_t PIN_LIMIT_SWITCH_1 = 26;  // Fim de curso 1
constexpr uint8_t PIN_LIMIT_SWITCH_2 = 27;  // Fim de curso 2

// Driver de Motor de Passo TB6600
// Conectar aos pinos correspondentes do driver TB6600
constexpr uint8_t PIN_STEPPER_ENABLE = 18;     // ENA- (LOW=ativo, HIGH=desabilitado)
constexpr uint8_t PIN_STEPPER_DIRECTION = 17;  // DIR- (HIGH=horário, LOW=anti-horário)
constexpr uint8_t PIN_STEPPER_PULSE = 16;      // PUL- (pulso de step)

// ============================================================================
// CONSTANTES DO SISTEMA
// ============================================================================

// Prioridades das Tasks FreeRTOS
// Quanto maior o número, maior a prioridade
constexpr uint8_t PRIORITY_BLINK_TASK = 1;    // Baixa prioridade (diagnóstico)
constexpr uint8_t PRIORITY_STEPPER_TASK = 3;  // Alta prioridade (tempo real)

// Configurações de Temporização
constexpr uint16_t BLINK_INTERVAL_MS = 1000;   // Intervalo do LED (1 segundo)
constexpr uint16_t STEPPER_CHECK_MS = 10;      // Período de verificação do stepper

// Configurações do Motor de Passo
constexpr int32_t STEPPER_DUMMY_STEPS = 400;   // Passos do movimento dummy
constexpr float STEPPER_SPEED = 500.0f;        // Velocidade (passos/segundo)
constexpr float STEPPER_ACCEL = 300.0f;        // Aceleração (passos/segundo²)
constexpr uint16_t STEPPER_MOVE_DELAY_MS = 2000;  // Delay entre movimentos (2 segundos)

// ============================================================================
// FUNÇÕES AUXILIARES DE HARDWARE
// ============================================================================

/**
 * @brief Inicializa todos os pinos de I/O do projeto
 * 
 * Configura todos os pinos como entrada ou saída e define seus estados iniciais.
 * Deve ser chamada uma vez no início do programa, antes de criar as tasks.
 */
void initHardwarePins();

/**
 * @brief Lê o estado do sensor de fim de curso 1
 * @return true se o sensor está ativo (pressionado), false caso contrário
 */
bool readLimitSwitch1();

/**
 * @brief Lê o estado do sensor de fim de curso 2
 * @return true se o sensor está ativo (pressionado), false caso contrário
 */
bool readLimitSwitch2();

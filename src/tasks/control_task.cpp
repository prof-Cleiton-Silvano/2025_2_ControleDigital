#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "tasks/control_task.h"
#include "tasks/stepper_task.h"
#include "tasks/display_task.h"

namespace tasks {
namespace {

// ============================================================================
// CONFIGURAÇÃO DO SISTEMA DE CONTROLE DIGITAL
// ============================================================================

// Fila de mensagens de entrada (sensor de toque)
QueueHandle_t xTouchInputQueue = nullptr;

// Tamanho da fila de entrada - permite buffer de múltiplas leituras
constexpr size_t kTouchInputQueueLength = 10;

// Período de amostragem do controlador (em ticks de FreeRTOS)
// Ts = tempo de amostragem = 100ms
// Este é o período do sistema discreto (digital)
constexpr TickType_t kControlPeriod = pdMS_TO_TICKS(100);


// ============================================================================
// PARÂMETROS DA FUNÇÃO DE TRANSFERÊNCIA
// ============================================================================
// 
// Em sistemas de controle digital, a função de transferência G(z) relaciona
// a entrada (sensor) com a saída (atuador) no domínio z (transformada Z).
// 
// Para um controlador simples proporcional-derivativo (PD):
//   U(z) = Kp * E(z) + Kd * (E(z) - E(z^-1)) / Ts
// 
// Onde:
//   U(z) = sinal de controle (número de passos do motor)
//   E(z) = erro atual (diferença entre setpoint e valor medido)
//   E(z^-1) = erro anterior (um período de amostragem atrás)
//   Kp = ganho proporcional
//   Kd = ganho derivativo
//   Ts = período de amostragem
// ============================================================================

// Ganho proporcional - Define quanto a saída responde ao erro atual
// Valores maiores = resposta mais agressiva
constexpr float kProportionalGain = 8.0f;  // Kp

// Ganho derivativo - Define quanto a saída responde à taxa de variação do erro
// Ajuda a reduzir overshoot e melhorar estabilidade
constexpr float kDerivativeGain = 2.0f;    // Kd

// Valor mínimo de comando para vencer atrito estático do motor
// Zona morta (deadband) - comandos menores são ignorados
constexpr uint32_t kMinSteps = 10;

// Valor máximo de passos por comando (saturação)
// Limita a saída para evitar movimentos bruscos
constexpr uint32_t kMaxSteps = 1000;


// ============================================================================
// MAPEAMENTO ENTRADA → SAÍDA
// ============================================================================
//
// Função de transferência implementada em código:
// Converte zona de toque (entrada digital) em número de passos (saída digital)
//
// Este mapeamento representa a relação estática do sistema:
//   Zona 0 (sem toque)     → 0 passos      → sem movimento
//   Zona 1 (toque leve)    → 50 passos     → movimento suave
//   Zona 2 (toque médio)   → 200 passos    → movimento moderado
//   Zona 3 (toque forte)   → 500 passos    → movimento rápido
//
// Em termos de controle digital, esta é uma LUT (Look-Up Table) que
// implementa uma função não-linear: y[k] = f(u[k])
// ============================================================================

// Tabela de mapeamento: zona de toque → número de passos base
constexpr uint32_t kZoneToStepsMap[] = {
    0,      // Zona 0: sem toque → sem movimento
    50,     // Zona 1: toque leve → 50 passos
    200,    // Zona 2: toque médio → 200 passos
    500     // Zona 3: toque forte → 500 passos
};

// Velocidades de rotação por zona (intervalo entre pulsos em microsegundos)
// Intervalos menores = velocidade maior
constexpr uint32_t kZoneToSpeedMap[] = {
    2000,   // Zona 0: N/A (não usado)
    1500,   // Zona 1: lento (1500μs entre pulsos)
    800,    // Zona 2: médio (800μs entre pulsos)
    300     // Zona 3: rápido (300μs entre pulsos)
};


// ============================================================================
// VARIÁVEIS DE ESTADO DO CONTROLADOR
// ============================================================================
// 
// Em sistemas de controle digital, mantemos um histórico de estados
// para implementar a equação de diferenças do controlador.
// 
// A equação de diferenças é a versão discreta da função de transferência:
//   y[k] = f(u[k], u[k-1], y[k-1], ...)
// 
// Onde:
//   k = instante de tempo atual
//   k-1 = instante de tempo anterior
// ============================================================================

// Estado anterior do controlador
struct ControlState {
  int32_t lastError;           // Erro anterior: e[k-1]
  uint8_t lastZone;            // Zona anterior: zona[k-1]
  bool alternateDirection;     // Direção alternada (para demonstração)
};

ControlState gControlState = {0, 0, false};


// ============================================================================
// FUNÇÃO DE PROCESSAMENTO DO CONTROLADOR
// ============================================================================
//
// Esta é a implementação da função de transferência discreta (digital).
// Ela recebe a entrada (mensagem de toque) e calcula a saída (comando do motor).
//
// Fluxo do processamento:
// 1. Lê entrada do sensor (via fila de mensagens)
// 2. Calcula erro: e[k] = setpoint - medição
// 3. Aplica lei de controle: u[k] = f(e[k], e[k-1], ...)
// 4. Satura/limita a saída
// 5. Envia comando ao atuador (motor de passo)
// 6. Atualiza estados para próxima iteração
// ============================================================================

void processControlLaw(const TouchInputMessage& input) {
  
  // -------------------------------------------------------------------------
  // ETAPA 1: EXTRAÇÃO DA ZONA DE TOQUE (entrada do sistema)
  // -------------------------------------------------------------------------
  // A zona representa a intensidade do toque (0 a 3)
  // Esta é nossa entrada discreta: u[k]
  uint8_t zone = input.touchZone;
  
  // -------------------------------------------------------------------------
  // ETAPA 2: MAPEAMENTO ENTRADA → PASSOS BASE (função de transferência)
  // -------------------------------------------------------------------------
  // Aqui aplicamos a tabela de conversão (LUT)
  // Esta é a relação estática: passos_base = G_static(zona)
  // 
  // Matematicamente: y_base[k] = f(u[k])
  uint32_t baseSteps = 0;
  uint32_t speedInterval = 1000;
  
  if (zone < sizeof(kZoneToStepsMap) / sizeof(kZoneToStepsMap[0])) {
    baseSteps = kZoneToStepsMap[zone];      // Número de passos pela zona
    speedInterval = kZoneToSpeedMap[zone];  // Velocidade pela zona
  }
  
  // -------------------------------------------------------------------------
  // ETAPA 3: CÁLCULO DO ERRO (componente proporcional)
  // -------------------------------------------------------------------------
  // O erro é a diferença entre a zona desejada e a zona anterior
  // Isso permite um comportamento mais suave e controlado
  // 
  // Erro atual: e[k] = referência[k] - medição[k]
  // Para este sistema simplificado: e[k] = zona[k] - zona[k-1]
  int32_t currentError = static_cast<int32_t>(zone) - static_cast<int32_t>(gControlState.lastZone);
  
  // -------------------------------------------------------------------------
  // ETAPA 4: COMPONENTE DERIVATIVA (taxa de variação)
  // -------------------------------------------------------------------------
  // A derivada discreta aproxima: d/dt ≈ (e[k] - e[k-1]) / Ts
  // Isso nos dá informação sobre a tendência do erro
  // 
  // Derivada do erro: de[k] = e[k] - e[k-1]
  int32_t errorDerivative = currentError - gControlState.lastError;
  
  // -------------------------------------------------------------------------
  // ETAPA 5: LEI DE CONTROLE PD (Proporcional-Derivativo)
  // -------------------------------------------------------------------------
  // Combinamos os componentes proporcional e derivativo
  // 
  // Sinal de controle: u[k] = Kp * e[k] + Kd * de[k]
  // 
  // O componente proporcional (Kp * e[k]) corrige o erro atual
  // O componente derivativo (Kd * de[k]) antecipa mudanças futuras
  float controlSignal = (kProportionalGain * currentError) + 
                        (kDerivativeGain * errorDerivative);
  
  // -------------------------------------------------------------------------
  // ETAPA 6: COMBINAÇÃO COM O MAPEAMENTO BASE
  // -------------------------------------------------------------------------
  // Somamos o sinal de controle aos passos base da zona
  // Isso permite ajuste fino além do mapeamento estático
  // 
  // Saída total: y[k] = y_base[k] + u[k]
  int32_t totalSteps = baseSteps + static_cast<int32_t>(controlSignal);
  
  // -------------------------------------------------------------------------
  // ETAPA 7: SATURAÇÃO E ZONA MORTA
  // -------------------------------------------------------------------------
  // Limitamos a saída para valores fisicamente realizáveis
  // 
  // Zona morta: se |y[k]| < threshold, então y[k] = 0
  // Saturação superior: se y[k] > max, então y[k] = max
  // Saturação inferior: se y[k] < 0, então y[k] = 0
  uint32_t commandSteps = 0;
  
  if (totalSteps < static_cast<int32_t>(kMinSteps)) {
    // Zona morta - movimento muito pequeno é ignorado
    commandSteps = 0;
  } else if (totalSteps > static_cast<int32_t>(kMaxSteps)) {
    // Saturação superior - limita movimento máximo
    commandSteps = kMaxSteps;
  } else {
    // Faixa válida - usa o valor calculado
    commandSteps = static_cast<uint32_t>(totalSteps);
  }
  
  // -------------------------------------------------------------------------
  // ETAPA 8: DETERMINAR DIREÇÃO DO MOVIMENTO
  // -------------------------------------------------------------------------
  // Alternamos a direção para demonstrar controle bidirecional
  // Em um sistema real, isso seria determinado pelo sinal do erro
  int32_t directionMultiplier = gControlState.alternateDirection ? -1 : 1;
  
  // -------------------------------------------------------------------------
  // ETAPA 9: ENVIAR COMANDO AO ATUADOR (saída do sistema)
  // -------------------------------------------------------------------------
  // Se há movimento a ser realizado, enviamos o comando ao motor
  if (commandSteps > 0) {
    // Converte velocidade de intervalo (μs) para steps/segundo
    // Fórmula: steps/sec = 1.000.000 / intervalUs
    float speedInStepsPerSec = (speedInterval > 0) ? (1000000.0f / speedInterval) : 500.0f;
    
    // Monta a mensagem de controle do motor (usando FlexyStepper)
    StepperMessage motorCmd{};
    motorCmd.targetPosition = commandSteps * directionMultiplier;  // Passos com direção
    motorCmd.speedInStepsPerSec = speedInStepsPerSec;              // Velocidade calculada
    motorCmd.accelInStepsPerSecSec = 200.0f;                       // Aceleração padrão
    motorCmd.isRelative = true;                                    // Movimento relativo
    
    // Envia comando para a fila do motor (sistema de atuação)
    sendStepperMessage(motorCmd, 0);
    
    // Feedback visual no display (opcional)
    DisplayMessage displayMsg;
    displayMsg.cmd = DisplayCmd::WriteChar;
    displayMsg.col = 5;
    displayMsg.row = 0;
    displayMsg.c = (directionMultiplier > 0) ? 'R' : 'L';
    sendDisplayMessage(displayMsg, 0);
  }
  
  // -------------------------------------------------------------------------
  // ETAPA 10: ATUALIZAR ESTADOS PARA PRÓXIMA ITERAÇÃO
  // -------------------------------------------------------------------------
  // Armazenamos os valores atuais para uso no próximo ciclo
  // Isso implementa o "atraso" (z^-1) da função de transferência discreta
  // 
  // Estado[k] → Estado[k-1] para o próximo ciclo
  gControlState.lastError = currentError;
  gControlState.lastZone = zone;
  gControlState.alternateDirection = !gControlState.alternateDirection;
}


// ============================================================================
// TASK PRINCIPAL DO CONTROLADOR
// ============================================================================
//
// Esta é a task de tempo real que executa o loop de controle digital.
// Ela opera em um período fixo (Ts) para manter a consistência temporal.
//
// Estrutura típica de um controlador digital:
// 1. Aguardar período de amostragem
// 2. Ler entrada(s) do sistema
// 3. Executar algoritmo de controle
// 4. Enviar saída(s) ao atuador
// 5. Repetir
// ============================================================================

void controlTask(void* /*params*/) {
  // Cria a fila de entrada se ainda não existir
  if (xTouchInputQueue == nullptr) {
    xTouchInputQueue = xQueueCreate(kTouchInputQueueLength, sizeof(TouchInputMessage));
  }
  
  // Variável para armazenar a última vez que o controle foi executado
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  // Loop infinito do controlador (execução periódica)
  for (;;) {
    // -----------------------------------------------------------------------
    // SINCRONIZAÇÃO TEMPORAL (período de amostragem)
    // -----------------------------------------------------------------------
    // Aguarda até o próximo período de controle (Ts)
    // Isso garante execução determinística e periódica
    vTaskDelayUntil(&lastWakeTime, kControlPeriod);
    
    // -----------------------------------------------------------------------
    // LEITURA DA ENTRADA (sensor → controlador)
    // -----------------------------------------------------------------------
    TouchInputMessage inputMsg;
    
    // Tenta ler mensagem da fila (sem bloqueio)
    // Se houver mensagem disponível, processa; senão, aguarda próximo ciclo
    if (xTouchInputQueue != nullptr && 
        xQueueReceive(xTouchInputQueue, &inputMsg, 0) == pdTRUE) {
      
      // ---------------------------------------------------------------------
      // EXECUÇÃO DA LEI DE CONTROLE
      // ---------------------------------------------------------------------
      // Processa a mensagem de entrada e calcula a saída
      // Esta função implementa a função de transferência G(z)
      processControlLaw(inputMsg);
    }
    
    // Se não houver mensagem, o controlador permanece em estado de espera
    // mantendo os últimos valores de estado (e[k-1], y[k-1], etc.)
  }
}

}  // namespace


// ============================================================================
// INTERFACE PÚBLICA
// ============================================================================

void startControlTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 3072;  // Stack maior para cálculos
  xTaskCreate(
      controlTask,
      "control_task",    // Nome descritivo da task
      kStackDepthWords,
      nullptr,
      priority,          // Prioridade intermediária (entre sensor e atuador)
      nullptr);
}

bool sendTouchInputMessage(const TouchInputMessage& msg, TickType_t ticksToWait) {
  // Cria a fila se necessário (inicialização lazy)
  if (xTouchInputQueue == nullptr) {
    xTouchInputQueue = xQueueCreate(kTouchInputQueueLength, sizeof(TouchInputMessage));
    if (xTouchInputQueue == nullptr) return false;
  }
  
  // Envia mensagem para a fila do controlador
  return xQueueSend(xTouchInputQueue, &msg, ticksToWait) == pdTRUE;
}

}  // namespace tasks

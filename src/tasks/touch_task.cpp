#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/touch_task.h"
#include "tasks/display_task.h"
#include "tasks/control_task.h"

namespace tasks {
namespace {

// ============================================================================
// CONFIGURAÇÃO DO SENSOR DE TOQUE (ENTRADA DO SISTEMA)
// ============================================================================

// Pino capacitivo do ESP32 para leitura de toque
// T0 geralmente mapeia para GPIO4 na maioria das placas ESP32
constexpr uint8_t kTouchPin = T0;

// Período de amostragem do sensor (100ms)
// Define a taxa na qual lemos o sensor capacitivo
constexpr TickType_t kPollDelay = pdMS_TO_TICKS(100);

// Tempo de debounce para evitar múltiplas leituras do mesmo toque
// Filtra transições rápidas e ruído
constexpr TickType_t kTouchDebounce = pdMS_TO_TICKS(300);


// ============================================================================
// THRESHOLDS (LIMIARES) DE CLASSIFICAÇÃO DO TOQUE
// ============================================================================
// 
// O sensor capacitivo ESP32 retorna valores MENORES para toques MAIS FORTES.
// Quando não tocado, o valor é alto (~70-100).
// Quando tocado levemente, o valor diminui (~30-50).
// Quando tocado fortemente, o valor diminui mais (~5-20).
//
// Classificamos o toque em 4 zonas baseadas em thresholds:
//
//   Valor > 50        → Zona 0: SEM TOQUE
//   30 < Valor ≤ 50   → Zona 1: TOQUE LEVE
//   15 < Valor ≤ 30   → Zona 2: TOQUE MÉDIO  
//   Valor ≤ 15        → Zona 3: TOQUE FORTE
//
// Estes thresholds devem ser ajustados experimentalmente para cada sensor.
// ============================================================================

// Limiar para detectar ausência de toque
constexpr long kNoTouchThreshold = 50;

// Limiar entre toque leve e médio
constexpr long kLightTouchThreshold = 30;

// Limiar entre toque médio e forte
constexpr long kMediumTouchThreshold = 15;


// ============================================================================
// FUNÇÃO DE CLASSIFICAÇÃO DO TOQUE
// ============================================================================
//
// Converte o valor bruto do sensor em uma zona discreta (0 a 3).
// Esta é uma função de quantização: valor analógico → valor digital.
//
// Entrada: valor bruto do sensor capacitivo (0-100)
// Saída: zona classificada (0=nenhum, 1=leve, 2=médio, 3=forte)
//
// Em controle digital, isso é chamado de ADC conceitual:
// converte sinal contínuo em níveis discretos.
// ============================================================================
uint8_t classifyTouchZone(long touchValue) {
  if (touchValue > kNoTouchThreshold) {
    // Zona 0: SEM TOQUE - valor alto indica ausência de contato
    return 0;
  } else if (touchValue > kLightTouchThreshold) {
    // Zona 1: TOQUE LEVE - valor moderado indica contato suave
    return 1;
  } else if (touchValue > kMediumTouchThreshold) {
    // Zona 2: TOQUE MÉDIO - valor baixo indica contato moderado
    return 2;
  } else {
    // Zona 3: TOQUE FORTE - valor muito baixo indica contato forte
    return 3;
  }
}


// ============================================================================
// TASK DE LEITURA E PROCESSAMENTO DO SENSOR
// ============================================================================
//
// Esta task atua como o SENSOR no diagrama de blocos do sistema de controle:
//
//    [SENSOR] → [CONTROLADOR] → [ATUADOR]
//       ↑            |              ↓
//       └────────────┴──────────────┘
//           (realimentação opcional)
//
// Responsabilidades:
// 1. Ler periodicamente o sensor capacitivo (amostragem)
// 2. Classificar o valor em zonas usando thresholds
// 3. Detectar mudanças significativas (histerese/debounce)
// 4. Enviar mensagens para o controlador quando houver evento relevante
// ============================================================================

void touchTask(void* /*params*/) {
  // Variáveis de estado para rastreamento
  uint8_t lastZone = 0;              // Última zona detectada
  long lastTouchValue = 0;           // Último valor bruto lido
  TickType_t lastMessageTime = 0;    // Timestamp da última mensagem enviada
  
  // Loop infinito de leitura do sensor
  for (;;) {
    // -------------------------------------------------------------------------
    // ETAPA 1: AMOSTRAGEM DO SENSOR
    // -------------------------------------------------------------------------
    // Lê o valor bruto do sensor capacitivo
    // No ESP32, valores menores = toque mais forte
    long touchValue = touchRead(kTouchPin);
    
    // -------------------------------------------------------------------------
    // ETAPA 2: CLASSIFICAÇÃO BASEADA EM THRESHOLDS
    // -------------------------------------------------------------------------
    // Converte valor analógico em zona discreta (0-3)
    // Isso implementa uma quantização multi-nível
    uint8_t currentZone = classifyTouchZone(touchValue);
    
    // -------------------------------------------------------------------------
    // ETAPA 3: FEEDBACK VISUAL NO DISPLAY
    // -------------------------------------------------------------------------
    // Atualiza o display quando a zona muda
    // Mostra o nível de toque ao usuário
    if (currentZone != lastZone) {
      DisplayMessage displayMsg;
      displayMsg.cmd = DisplayCmd::WriteChar;
      displayMsg.col = 3;
      displayMsg.row = 0;
      
      // Mapeia zona para caractere visual:
      // '0' = sem toque, '1' = leve, '2' = médio, '3' = forte
      displayMsg.c = '0' + currentZone;
      
      sendDisplayMessage(displayMsg, 0);
    }
    
    // -------------------------------------------------------------------------
    // ETAPA 4: DETECÇÃO DE EVENTO COM DEBOUNCE
    // -------------------------------------------------------------------------
    // Envia mensagem ao controlador apenas quando:
    // 1. A zona mudou (transição de estado)
    // 2. Passou tempo suficiente desde a última mensagem (debounce)
    // 3. A zona atual não é zero (há toque detectado)
    //
    // Isso implementa um filtro temporal para evitar ruído e múltiplas
    // detecções do mesmo evento.
    TickType_t currentTime = xTaskGetTickCount();
    
    bool zoneChanged = (currentZone != lastZone);
    bool debounceElapsed = ((currentTime - lastMessageTime) >= kTouchDebounce);
    bool touchActive = (currentZone > 0);
    
    if (zoneChanged && debounceElapsed && touchActive) {
      // -----------------------------------------------------------------------
      // ETAPA 5: PREPARAR E ENVIAR MENSAGEM AO CONTROLADOR
      // -----------------------------------------------------------------------
      // Monta a mensagem de entrada para o sistema de controle
      TouchInputMessage msg;
      msg.touchValue = static_cast<int32_t>(touchValue);  // Valor bruto
      msg.touchZone = currentZone;                        // Zona classificada
      msg.timestamp = currentTime;                        // Timestamp
      
      // Envia mensagem ao controlador (sem bloqueio)
      // O controlador processará esta entrada e calculará a resposta
      sendTouchInputMessage(msg, 0);
      
      // Atualiza timestamp da última mensagem
      lastMessageTime = currentTime;
    }
    
    // -------------------------------------------------------------------------
    // ETAPA 6: ATUALIZAR ESTADO PARA PRÓXIMA ITERAÇÃO
    // -------------------------------------------------------------------------
    lastZone = currentZone;
    lastTouchValue = touchValue;
    
    // Aguarda próximo período de amostragem
    vTaskDelay(kPollDelay);
  }
}

}  // namespace

void startTouchTask(UBaseType_t priority) {
  constexpr uint32_t kStackDepthWords = 2048;
  xTaskCreate(
      touchTask,
      "touch_task",
      kStackDepthWords,
      nullptr,
      priority,
      nullptr);
}

}  // namespace tasks

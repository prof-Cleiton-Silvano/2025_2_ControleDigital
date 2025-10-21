# Sistema de Controle Digital

## Visão Geral

Este projeto implementa um **sistema de controle digital em tempo real** usando FreeRTOS no ESP32. O sistema demonstra conceitos fundamentais de controle digital aplicados ao controle de um motor de passo baseado em entrada de sensor capacitivo.

## Arquitetura do Sistema

### Diagrama de Blocos

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│   SENSOR    │─────>│ CONTROLADOR  │─────>│  ATUADOR    │
│  (Touch)    │      │  (Control)   │      │  (Stepper)  │
└─────────────┘      └──────────────┘      └─────────────┘
      │                     │                      │
      v                     v                      v
 touch_task            control_task          stepper_task
```

### Fluxo de Dados

1. **Sensor (touch_task)**: Lê o sensor capacitivo e classifica o toque em zonas
2. **Controlador (control_task)**: Processa a entrada e calcula a resposta usando função de transferência
3. **Atuador (stepper_task)**: Executa os comandos movendo o motor de passo

### Comunicação entre Tasks

As tasks se comunicam através de **filas (queues) do FreeRTOS**, garantindo:
- **Desacoplamento**: Tasks independentes e modulares
- **Sincronização**: Comunicação segura entre tasks
- **Buffering**: Capacidade de processar múltiplas mensagens

```
TouchInputMessage: touch_task → control_task
StepperMessage:    control_task → stepper_task
```

## Conceitos de Controle Digital

### 1. Período de Amostragem (Ts)

O sistema opera em **tempo discreto** com período fixo:
- **Ts = 100ms** (10 Hz)
- Define a taxa de atualização do controlador
- Fundamental para estabilidade do sistema

```cpp
constexpr TickType_t kControlPeriod = pdMS_TO_TICKS(100);
```

### 2. Função de Transferência Discreta

A relação entrada-saída é implementada através de:

#### a) Mapeamento Estático (Look-Up Table)

Conversão direta de zona de toque para número de passos:

```
Zona 0 (sem toque)   → 0 passos
Zona 1 (leve)        → 50 passos
Zona 2 (médio)       → 200 passos
Zona 3 (forte)       → 500 passos
```

Matematicamente: **y[k] = f(u[k])**

#### b) Controle PD (Proporcional-Derivativo)

Ajuste dinâmico baseado no erro e sua variação:

```
u[k] = Kp × e[k] + Kd × (e[k] - e[k-1])
```

Onde:
- **u[k]**: sinal de controle (passos do motor)
- **e[k]**: erro atual
- **e[k-1]**: erro anterior
- **Kp**: ganho proporcional (8.0)
- **Kd**: ganho derivativo (2.0)

### 3. Equação de Diferenças

A implementação discreta da função de transferência:

```
y[k] = y_base[k] + Kp × e[k] + Kd × (e[k] - e[k-1])
```

Esta é a versão digital (amostrada) da equação diferencial contínua.

### 4. Quantização (ADC Conceitual)

O sensor converte sinal contínuo em níveis discretos usando **thresholds**:

```cpp
if (valor > 50)      → Zona 0
else if (valor > 30) → Zona 1
else if (valor > 15) → Zona 2
else                 → Zona 3
```

### 5. Saturação e Zona Morta

Limitações físicas do sistema:

- **Zona Morta**: comandos < 10 passos são ignorados (atrito estático)
- **Saturação**: comandos > 1000 passos são limitados (segurança)

```cpp
if (passos < kMinSteps)      → passos = 0
else if (passos > kMaxSteps) → passos = kMaxSteps
```

### 6. Memória de Estados

O controlador mantém histórico para implementar z⁻¹ (atraso):

```cpp
struct ControlState {
  int32_t lastError;     // e[k-1]
  uint8_t lastZone;      // zona[k-1]
};
```

## Implementação das Tasks

### Touch Task (Sensor)

**Arquivo**: `src/tasks/touch_task.cpp`

**Função**: Ler e classificar entrada do sensor

**Processo**:
1. Lê valor bruto do sensor capacitivo
2. Classifica em zona (0-3) usando thresholds
3. Aplica debounce temporal
4. Envia mensagem ao controlador

**Período**: 100ms

### Control Task (Controlador)

**Arquivo**: `src/tasks/control_task.cpp`

**Função**: Implementar função de transferência digital

**Processo**:
1. Recebe mensagem do sensor
2. Calcula erro: e[k] = zona[k] - zona[k-1]
3. Calcula derivada: de[k] = e[k] - e[k-1]
4. Aplica lei de controle PD
5. Combina com mapeamento base
6. Aplica saturação e zona morta
7. Envia comando ao atuador
8. Atualiza estados

**Período**: 100ms (sincronizado)

### Stepper Task (Atuador)

**Arquivo**: `src/tasks/stepper_task.cpp`

**Função**: Executar comandos de movimento

**Processo**:
1. Recebe comando do controlador
2. Configura direção e velocidade
3. Gera pulsos com timing preciso
4. Mantém motor habilitado

**Prioridade**: Alta (timing crítico)

## Parâmetros de Ajuste

### Ganhos do Controlador

```cpp
constexpr float kProportionalGain = 8.0f;  // Kp - resposta ao erro
constexpr float kDerivativeGain = 2.0f;    // Kd - amortecimento
```

**Ajuste**:
- Aumentar Kp → resposta mais rápida (pode causar overshoot)
- Aumentar Kd → mais estabilidade (reduz oscilações)

### Thresholds do Sensor

```cpp
constexpr long kNoTouchThreshold = 50;
constexpr long kLightTouchThreshold = 30;
constexpr long kMediumTouchThreshold = 15;
```

**Ajuste**: Experimente valores observando o valor bruto do sensor.

### Mapeamento de Passos

```cpp
constexpr uint32_t kZoneToStepsMap[] = {0, 50, 200, 500};
```

**Ajuste**: Modifique para alterar a sensibilidade de cada zona.

## Análise do Sistema

### Resposta em Frequência

Com Ts = 100ms:
- Frequência de amostragem: fs = 10 Hz
- Frequência de Nyquist: fN = 5 Hz
- Dinâmicas do sistema devem estar abaixo de 5 Hz

### Estabilidade

Critérios:
- Polos dentro do círculo unitário no plano z
- Ganhos Kp e Kd adequados para evitar oscilação
- Período de amostragem suficientemente pequeno

### Performance

Métricas:
- **Tempo de resposta**: ~100-200ms (1-2 períodos de amostragem)
- **Overshoot**: Minimizado pelo componente derivativo
- **Erro de regime**: Pode existir devido à ausência de componente integral

## Expansões Futuras

### Controle PID Completo

Adicionar componente integral:

```cpp
u[k] = Kp×e[k] + Ki×∑e[k] + Kd×(e[k]-e[k-1])
```

### Anti-Windup

Prevenir saturação do integrador:

```cpp
if (saturado && mesmo_sinal(erro, integral)) {
  integral[k] = integral[k-1];  // não acumula
}
```

### Filtro Passa-Baixas

Reduzir ruído na medição:

```cpp
y_filtrado[k] = α×y[k] + (1-α)×y_filtrado[k-1]
```

### Estimador de Estados (Kalman)

Para sensores com ruído significativo.

### Controle Adaptativo

Ajuste automático de ganhos baseado em performance.

## Referências

- Franklin, Powell & Workman - "Digital Control of Dynamic Systems"
- Ogata - "Discrete-Time Control Systems"
- Åström & Wittenmark - "Computer-Controlled Systems"

## Glossário

- **z**: Operador de avanço na transformada Z
- **z⁻¹**: Operador de atraso (um período de amostragem)
- **e[k]**: Erro no instante k
- **u[k]**: Sinal de controle no instante k
- **y[k]**: Saída do sistema no instante k
- **Ts**: Período de amostragem
- **Kp**: Ganho proporcional
- **Kd**: Ganho derivativo
- **Ki**: Ganho integral

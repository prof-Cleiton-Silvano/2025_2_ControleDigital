# Control Task - Sistema de Controle Digital

## Resumo das Implementações

Este documento descreve as mudanças implementadas para criar um sistema de controle digital educacional baseado em FreeRTOS.

## Arquivos Criados

### 1. `include/tasks/control_task.h`
Header da nova task de controle digital.

**Principais componentes:**
- `TouchInputMessage`: estrutura de mensagem do sensor para o controlador
- `sendTouchInputMessage()`: função para enviar mensagens ao controlador
- `startControlTask()`: inicializa a task de controle

### 2. `src/tasks/control_task.cpp`
Implementação completa do controlador digital.

**Características:**
- **Comentários extensivos** explicando cada etapa do processo de controle
- **Função de transferência discreta** implementada com controle PD
- **Mapeamento explícito** de zona de toque → número de passos
- **Equação de diferenças** claramente documentada
- **Estados de memória** para implementar z⁻¹

**Seções documentadas:**
1. Configuração do sistema de controle
2. Parâmetros da função de transferência
3. Mapeamento entrada → saída (LUT)
4. Variáveis de estado do controlador
5. Função de processamento (10 etapas detalhadas)
6. Task principal do controlador

### 3. `docs/control_system.md`
Documentação completa do sistema de controle.

**Conteúdo:**
- Diagrama de blocos do sistema
- Fluxo de dados entre tasks
- Conceitos de controle digital explicados
- Análise de estabilidade e performance
- Parâmetros de ajuste
- Expansões futuras sugeridas

## Arquivos Refatorados

### 1. `src/tasks/touch_task.cpp`
Refatoração completa para trabalhar com mensagens e thresholds.

**Mudanças principais:**
- Implementação de **classificação por zonas** (0-3) usando thresholds
- Função `classifyTouchZone()` para quantização do sinal
- Envio de mensagens para o controlador (não mais direto para o motor)
- **Comentários didáticos** em cada etapa do processamento
- Debounce temporal para evitar ruído

**Thresholds configuráveis:**
```cpp
kNoTouchThreshold = 50      // Zona 0: sem toque
kLightTouchThreshold = 30   // Zona 1: toque leve
kMediumTouchThreshold = 15  // Zona 2: toque médio
                            // Zona 3: toque forte
```

### 2. `include/tasks/touch_task.h`
Atualizado com comentários explicativos sobre o papel do sensor no sistema.

### 3. `src/main.cpp`
Adicionada a nova control_task com prioridade intermediária.

**Estrutura de prioridades:**
```
Prioridade 1: Blink, Display, Touch (sensor)
Prioridade 2: Control (processamento)
Prioridade 3: Stepper (atuador)
```

## Conceitos de Controle Digital Implementados

### 1. Sistema em Tempo Discreto
- Período de amostragem Ts = 100ms
- Execução periódica sincronizada
- Operação no domínio z (transformada Z)

### 2. Função de Transferência
Implementação de **G(z)** que relaciona entrada → saída:

**Componente Estático (LUT):**
```
y_base[k] = f(zona[k])
```

**Componente Dinâmico (PD):**
```
u[k] = Kp × e[k] + Kd × (e[k] - e[k-1])
```

**Saída Total:**
```
y[k] = y_base[k] + u[k]
```

### 3. Equação de Diferenças
Versão discreta da equação diferencial, implementada em código C++.

### 4. Memória de Estados
Armazenamento de valores anteriores (e[k-1], zona[k-1]) para implementar atrasos (z⁻¹).

### 5. Saturação e Zona Morta
Limitações físicas do sistema modeladas no controlador.

### 6. Quantização Multi-nível
Conversão de sinal analógico (sensor) em níveis discretos (zonas).

## Fluxo de Processamento

```
┌─────────────┐
│ touch_task  │
│  (Sensor)   │
└──────┬──────┘
       │ TouchInputMessage
       │ {touchValue, touchZone, timestamp}
       v
┌─────────────┐
│control_task │
│(Controlador)│
└──────┬──────┘
       │ StepperMessage
       │ {steps, intervalUs, direction}
       v
┌─────────────┐
│stepper_task │
│  (Atuador)  │
└─────────────┘
```

## Código Didático

### Exemplo: Mapeamento Explícito (control_task.cpp, linha ~80)

```cpp
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

constexpr uint32_t kZoneToStepsMap[] = {
    0,      // Zona 0: sem toque → sem movimento
    50,     // Zona 1: toque leve → 50 passos
    200,    // Zona 2: toque médio → 200 passos
    500     // Zona 3: toque forte → 500 passos
};
```

### Exemplo: Lei de Controle PD (control_task.cpp, linha ~195)

```cpp
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
```

## Ajustes Recomendados

### 1. Ganhos do Controlador
Experimente diferentes valores em `control_task.cpp`:
```cpp
constexpr float kProportionalGain = 8.0f;  // Teste: 5.0 - 15.0
constexpr float kDerivativeGain = 2.0f;    // Teste: 0.5 - 5.0
```

### 2. Thresholds do Sensor
Ajuste em `touch_task.cpp` observando valores brutos:
```cpp
constexpr long kNoTouchThreshold = 50;     // Ajuste conforme sensor
constexpr long kLightTouchThreshold = 30;
constexpr long kMediumTouchThreshold = 15;
```

### 3. Mapeamento de Passos
Modifique a sensibilidade em `control_task.cpp`:
```cpp
constexpr uint32_t kZoneToStepsMap[] = {0, 50, 200, 500};
```

## Uso Educacional

O código foi projetado para ser **didático e explicativo**:

1. **Comentários em blocos** explicam o "porquê" de cada seção
2. **Comentários inline** explicam o "como" de cada operação
3. **Nomenclatura descritiva** usando notação de controle (e[k], u[k], y[k])
4. **Separação clara** entre componentes do sistema
5. **Documentação externa** complementar em `docs/control_system.md`

## Compilação e Upload

```bash
# Compilar o projeto
pio run

# Compilar e fazer upload
pio run --target upload

# Monitorar serial
pio device monitor
```

## Visualização do Sistema

Durante a execução:
- **Coluna 3 do LCD**: Mostra zona de toque (0-3)
- **Coluna 5 do LCD**: Mostra direção do motor (R/L)
- **Motor**: Executa movimento proporcional ao toque

## Próximos Passos

1. Adicionar componente integral (PID completo)
2. Implementar anti-windup
3. Criar filtro passa-baixas para o sensor
4. Adicionar logging de estados via serial
5. Implementar interface de ajuste de ganhos em tempo real

## Referências

- `docs/control_system.md`: Documentação completa do sistema
- `docs/architecture.md`: Arquitetura geral do projeto
- Franklin, Powell & Workman - "Digital Control of Dynamic Systems"

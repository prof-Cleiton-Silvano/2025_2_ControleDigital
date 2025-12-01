# Sistema de Controle de Motor de Passo - ESP32

Sistema didático de controle de motor de passo usando ESP32 Dev Kit V1, baseado em FreeRTOS.

## 📋 Características

- **Código Didático**: Comentários extensivos em português explicando cada função
- **Modular**: Código organizado em arquivos pequenos (<100 linhas)
- **Configuração Centralizada**: Todos os pinos e constantes em um único arquivo
- **FreeRTOS**: Sistema multitarefa com prioridades definidas

## 🔌 Hardware Necessário

- ESP32 Dev Kit V1
- Driver de Motor TB6600
- Motor de Passo NEMA 17 ou similar
- 2x Sensores de Fim de Curso (opcional)
- Fonte de alimentação adequada para o motor

## 📍 Conexões dos Pinos

### Motor de Passo (TB6600)
- **GPIO 16** → PUL- (Pulso de Step)
- **GPIO 17** → DIR- (Direção)
- **GPIO 18** → ENA- (Enable)

### Sensores de Fim de Curso
- **GPIO 26** → Fim de Curso 1 (conectar entre pino e 3.3V)
- **GPIO 27** → Fim de Curso 2 (conectar entre pino e 3.3V)

### LED Built-in
- **GPIO 2** → LED integrado (diagnóstico)

### Botões de Usuário (reservados para uso futuro)
- **GPIO 32** → Botão 1 (conectar entre pino e GND)
- **GPIO 33** → Botão 2 (conectar entre pino e GND)
- **GPIO 25** → Botão 3 (conectar entre pino e GND)

## 📁 Estrutura do Projeto

```
├── include/
│   ├── config.h                 # Configurações centralizadas (pinos, constantes)
│   └── tasks/
│       ├── blink_task.h         # Task de diagnóstico do LED
│       └── stepper_task.h       # Task de controle do motor
├── src/
│   ├── main.cpp                 # Programa principal
│   ├── config.cpp               # Implementação das funções de hardware
│   └── tasks/
│       ├── blink_task.cpp       # Implementação da task do LED
│       └── stepper_task.cpp     # Implementação da task do motor
└── platformio.ini               # Configuração do PlatformIO
```

## 🎯 Funcionalidades

### 1. Blink Task (Diagnóstico)
- **Prioridade**: Baixa (1)
- **Função**: Pisca o LED built-in a cada 1 segundo
- **Propósito**: Indicador visual de que o sistema está operacional

### 2. Stepper Task (Controle do Motor)
- **Prioridade**: Alta (3)
- **Função**: Controla o motor de passo com movimento dummy
- **Características**:
  - Move 400 passos para frente
  - Aguarda 2 segundos
  - Move 400 passos para trás
  - Aguarda 2 segundos
  - Repete indefinidamente
  - Monitora sensores de fim de curso continuamente
  - Para imediatamente se detectar fim de curso

## ⚙️ Configurações Padrão

As configurações podem ser alteradas no arquivo `include/config.h`:

```cpp
// Movimento do motor
STEPPER_DUMMY_STEPS = 400        // Passos por movimento
STEPPER_SPEED = 500.0            // Passos/segundo
STEPPER_ACCEL = 300.0            // Aceleração (passos/s²)
STEPPER_MOVE_DELAY_MS = 2000     // Delay entre movimentos

// Temporização
BLINK_INTERVAL_MS = 1000         // Intervalo do LED
```

## 🚀 Como Usar

### 1. Compilar o Projeto
```bash
pio run
```

### 2. Upload para o ESP32
```bash
pio run --target upload
```

### 3. Monitorar Serial
```bash
pio device monitor
```

## 🔧 Modificar o Movimento Dummy

Para alterar o comportamento do motor, edite a função `stepperTask()` em `src/tasks/stepper_task.cpp`:

```cpp
// Movimento atual (dummy)
executeMove(STEPPER_DUMMY_STEPS);   // 400 passos para frente
vTaskDelay(pdMS_TO_TICKS(2000));    // Aguarda 2s
executeMove(-STEPPER_DUMMY_STEPS);  // 400 passos para trás

// Exemplo: movimento customizado
executeMove(800);                    // 800 passos para frente
vTaskDelay(pdMS_TO_TICKS(1000));    // Aguarda 1s
executeMove(-400);                   // 400 passos para trás
```

## 📚 Bibliotecas Utilizadas

- **ESP-FlexyStepper**: Controle avançado de motor de passo
- **FreeRTOS**: Sistema operacional de tempo real (nativo do ESP32)

## 🛡️ Segurança

O sistema inclui:
- Monitoramento contínuo de fim de curso
- Parada de emergência automática
- Motor desabilitado por padrão ao iniciar

## 📖 Aprendizado

Este projeto é ideal para aprender:
- Programação em C++ para ESP32
- FreeRTOS e multitarefa
- Controle de motores de passo
- Organização de código modular
- Boas práticas de documentação

## 🔍 Debug

Mensagens no Serial Monitor:
```
=== Sistema de Controle de Motor de Passo ===
Inicializando hardware...
Criando tasks FreeRTOS...
Sistema iniciado com sucesso!
Tasks ativas:
  - LED Blink (prioridade 1)
  - Stepper Motor (prioridade 3)
```

## 📝 Notas

- O LED piscando indica sistema operacional
- Se o LED parar, o ESP32 travou
- Os sensores de fim de curso param o motor instantaneamente
- O movimento dummy é apenas para demonstração

## 🤝 Contribuindo

Para adicionar novas funcionalidades:
1. Mantenha arquivos com menos de 100 linhas
2. Documente todas as funções
3. Use constantes do `config.h`
4. Teste antes de commitar

## 📄 Licença

Projeto educacional - Use livremente para aprendizado.

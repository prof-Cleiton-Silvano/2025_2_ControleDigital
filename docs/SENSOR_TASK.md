# Sensor Task - EE-SH3 Photomicrosensor

## 📝 Descrição

Task simples para monitorar o sensor óptico transmissivo **EE-SH3 Photomicrosensor**.

## 🔌 Conexão do Hardware

### Sensor EE-SH3 Photomicrosensor
- **Tipo**: Sensor óptico transmissivo (feixe de luz infravermelho)
- **Pino GPIO**: 34 (entrada analógica/digital)
- **Conexão**:
  - VCC → 3.3V ou 5V (verificar datasheet)
  - GND → GND
  - OUT → GPIO 34

### Como Funciona
- **Desbloqueado** (HIGH): Feixe de luz livre, nada bloqueando
- **Bloqueado** (LOW): Objeto interrompeu o feixe de luz

## 📊 Funcionalidades

### 1. Leitura Contínua
- Verifica estado do sensor a cada 100ms
- Detecta mudanças de estado (transições)

### 2. Detecção de Eventos
- Detecta quando o feixe é **bloqueado**
- Detecta quando o feixe é **desbloqueado**
- Envia mensagens no Serial Monitor

### 3. Contador de Bloqueios
- Conta quantas vezes o sensor foi bloqueado
- Útil para contagem de objetos, RPM, etc.

### 4. Interface Pública
```cpp
// Obtém estado atual do sensor
bool getSensorState();          // true = desbloqueado, false = bloqueado

// Obtém contagem de bloqueios
uint32_t getSensorBlockCount(); // Número total de bloqueios
```

## 🖥️ Saída no Serial Monitor

```
Sensor Task: Iniciada
Estado inicial: Desbloqueado
Sensor bloqueado! Contagem: 1
Sensor desbloqueado!
Sensor bloqueado! Contagem: 2
Sensor desbloqueado!
```

## ⚙️ Configurações

No arquivo `include/config.h`:

```cpp
// Pino do sensor
constexpr uint8_t PIN_PHOTO_SENSOR = 34;

// Intervalo de leitura (em milissegundos)
constexpr uint16_t SENSOR_READ_INTERVAL_MS = 100;

// Prioridade da task
constexpr uint8_t PRIORITY_SENSOR_TASK = 2;
```

## 🎯 Aplicações Práticas

### 1. Contador de Objetos
```cpp
// Na sua aplicação
uint32_t objetos = getSensorBlockCount();
Serial.printf("Total de objetos: %lu\n", objetos);
```

### 2. Medidor de RPM
- Colocar marcação no eixo rotativo
- Cada bloqueio = uma volta
- Calcular RPM com base no tempo

### 3. Detector de Posição
- Verificar se objeto está presente
- Sincronizar com motor de passo

### 4. Sistema de Segurança
- Detectar intrusão em área protegida
- Parar motor se sensor for bloqueado

## 🔧 Exemplo de Uso no Código

### Ler Estado Atual
```cpp
#include "tasks/sensor_task.h"

void minhaFuncao() {
  if (getSensorState()) {
    Serial.println("Caminho livre");
  } else {
    Serial.println("Objeto detectado");
  }
}
```

### Obter Contagem
```cpp
void exibirContagem() {
  uint32_t total = getSensorBlockCount();
  Serial.printf("Objetos contados: %lu\n", total);
}
```

## 📈 Expansões Futuras

### Ideias para Melhorar a Task:

1. **Debouncing**: Adicionar filtro para evitar leituras falsas
2. **Temporizador**: Medir tempo entre bloqueios (velocidade)
3. **Fila de Eventos**: Enviar eventos para outras tasks
4. **Callback**: Executar função quando sensor bloquear
5. **Histórico**: Registrar últimos N bloqueios com timestamp

### Exemplo de Debouncing
```cpp
// Adicionar variável de estado
static uint32_t lastDebounceTime = 0;
const uint32_t debounceDelay = 50; // 50ms

// No loop da task
if ((millis() - lastDebounceTime) > debounceDelay) {
  if (currentState != lastState) {
    lastDebounceTime = millis();
    // Processar mudança de estado
  }
}
```

## 🛠️ Troubleshooting

### Sensor não responde
- Verificar conexões (VCC, GND, OUT)
- Verificar alimentação do sensor
- Testar com multímetro no pino OUT

### Leituras instáveis
- Adicionar resistor pull-up/pull-down
- Implementar debouncing
- Verificar interferência eletromagnética

### Contagem errada
- Ajustar intervalo de leitura
- Implementar debouncing
- Verificar velocidade do objeto

## 📊 Uso de Recursos

- **RAM**: ~100 bytes (variáveis globais)
- **Stack**: 2KB (task)
- **CPU**: Mínimo (delay de 100ms entre leituras)
- **Prioridade**: 2 (média)

## 🎓 Aprendizado

Esta task demonstra:
- Leitura de sensor digital
- Detecção de eventos (edge detection)
- Contador de eventos
- Debug via Serial
- Organização modular
- Interface pública (getters)

## 📄 Arquivos Relacionados

- `include/config.h` - Definição do pino e constantes
- `include/tasks/sensor_task.h` - Interface pública
- `src/tasks/sensor_task.cpp` - Implementação (70 linhas)
- `src/config.cpp` - Função `readPhotoSensor()`

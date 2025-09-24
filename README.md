# 2025_2_ControleDigital

Projeto base para o desenvolvimento de firmware no ESP32 usando PlatformIO com framework Arduino e FreeRTOS.

## Pré-requisitos
- [PlatformIO](https://platformio.org/install) instalado na linha de comando ou no VS Code.
- Placa baseada em ESP32 compatível com o perfil `esp32dev`.

## Como compilar e gravar
1. Compile o projeto:
   ```bash
   pio run
   ```
2. Compile e grave na placa conectada:
   ```bash
   pio run --target upload
   ```
3. Abra o monitor serial (opcional):
   ```bash
   pio device monitor
   ```

## Estrutura de diretórios
- `platformio.ini`: configuração principal do ambiente PlatformIO.
- `src/`: código-fonte da aplicação.
  - `hal/`: abstrações de hardware compartilhadas.
  - `tasks/`: implementação das tasks FreeRTOS.
  - `main.cpp`: ponto de entrada que inicializa o hardware e cria as tasks.
- `include/`: cabeçalhos públicos expostos para o projeto.
- `docs/`: documentação complementar do projeto.

## Próximos passos sugeridos
- Adicionar novas tasks em `src/tasks/` para os demais módulos de controle digital.
- Expandir a camada de hardware (`src/hal/`) para incluir sensores, atuadores e interfaces de comunicação.
- Criar testes de integração ou simulações conforme os módulos forem evoluindo.

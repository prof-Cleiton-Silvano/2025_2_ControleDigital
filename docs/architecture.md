# Arquitetura do Projeto

## Visão geral
O firmware foi estruturado para isolar camadas de hardware e organizar a lógica de controle em módulos independentes, cada um executado como uma task FreeRTOS. Essa abordagem facilita a manutenção, permite escalabilidade e reduz o acoplamento entre componentes.

## Fluxo de inicialização
1. `setup()` em `src/main.cpp` chama `hal::initBoard()` para preparar os periféricos compartilhados.
2. Em seguida, tasks individuais são criadas por meio de funções de fábrica no namespace `tasks`.
3. A função `loop()` permanece ociosa, delegando todo o trabalho ao agendador do FreeRTOS.

## Estrutura de módulos
- `hal/board.*`: define a abstração do hardware básico (LED interno e outras futuras dependências).
- `tasks/blink_task.*`: task de exemplo com prioridade baixa responsável por piscar o LED builtin.
- Novas tasks devem ser implementadas em `src/tasks/` com cabeçalho correspondente em `include/tasks/`, expondo uma função `start*Task` que receba a prioridade desejada.

## Convenções de desenvolvimento
- Centralize toda a lógica de acesso a pinos, barramentos e periféricos em `hal/` e exponha apenas as funções necessárias.
- Prefira definir constantes de hardware (pinos, temporizações padrão) nos cabeçalhos da camada `hal` para permitir reuso.
- Evite colocar lógica dentro de `loop()`; utilize tasks dedicadas para cada módulo.
- Mantenha os comentários curtos e focados no objetivo do bloco de código.

## Próximos módulos
- Task de leitura de sensores com fila de mensagens para publicar amostras.
- Task de controle que processe os dados e aplique ações no hardware.
- Task de comunicação (por exemplo, UART, Wi-Fi ou BLE) para telemetria e monitoramento remoto.

Esses módulos podem ser adicionados gradualmente, reaproveitando o padrão estabelecido pelo `blink_task` e ampliando a camada `hal` conforme necessário.

# AGENTS.md

## Projeto

Este repositório contém firmware IoT em C/C++ para dispositivos ESP32-family usando ESP-IDF e, quando aplicável, ESP-ADF.

O objetivo principal é manter firmware confiável em hardware real, diagnosticável em campo, seguro, testável e compatível com dispositivos já provisionados.

## Skills recomendadas

Use as skills locais conforme o tipo de tarefa:

- `$esp-idf` para firmware ESP-IDF geral: Wi-Fi, BLE/Blufi, MQTT/TLS, OTA, NVS, FreeRTOS, periféricos, diagnóstico, drivers e arquitetura de firmware.
- `$esp-adf` para áudio com ESP-ADF: pipelines, audio elements, codecs, I2S, streams, ringbuffers, player/recorder, board support e diagnóstico de áudio.

Quando a tarefa envolver áudio e conectividade, use `$esp-adf` para a arquitetura de áudio e aplique também as regras ESP-IDF para Wi-Fi, BLE/Blufi, MQTT, TLS, OTA, NVS, FreeRTOS, diagnóstico e contratos com app/backend.

## Relação com Spec Kit

Este projeto usa GitHub Spec Kit.

A ordem de trabalho esperada é:

1. `/speckit.constitution`
2. `/speckit.specify`
3. `/speckit.clarify`
4. `/speckit.checklist`
5. `/speckit.plan`
6. `/speckit.tasks`
7. `/speckit.implement`

Em Codex CLI com skills mode, os comandos podem aparecer no formato `$speckit-*`, por exemplo `$speckit-implement`. Use o formato disponível no ambiente atual.

Durante fases de especificação, planejamento ou geração de tarefas, não implementar código.

Durante `/speckit.implement` ou `$speckit-implement`, seguir estritamente os arquivos gerados pelo Spec Kit, especialmente:

- `.specify/memory/constitution.md`
- `spec.md`
- `plan.md`
- `tasks.md`

Quando houver múltiplas features ou múltiplos diretórios de spec, identificar a feature ativa pelo branch atual, pelo caminho indicado pelo comando Spec Kit ou pelos arquivos abertos na solicitação. Se ainda houver ambiguidade, pedir confirmação antes de planejar ou implementar.

Quando uma alteração de implementação revelar necessidade de mudança de requisito, arquitetura, pinagem, protocolo, tópico MQTT, partição, NVS schema, pipeline de áudio, codec, sample rate, I2S ou contrato com app/backend, parar e atualizar primeiro a spec, o plano ou as tarefas.

## Fontes de verdade

Prioridade das instruções:

1. Solicitação atual do usuário
2. `.specify/memory/constitution.md`
3. `AGENTS.md`
4. `spec.md`
5. `plan.md`
6. `tasks.md`
7. Código existente
8. Documentação oficial do ESP-IDF / ESP-ADF

A solicitação atual do usuário define o objetivo de trabalho, mas não autoriza implicitamente contornar restrições críticas, contratos de hardware/comunicação, segurança, compatibilidade com dispositivos provisionados ou operações destrutivas. Para qualquer exceção nesses pontos, exigir autorização explícita e registrar a justificativa; quando o Spec Kit estiver ativo, a justificativa deve aparecer nos artefatos de spec/plano/tarefas antes da implementação.

Em conflito entre implementação existente e Spec Kit, não assumir automaticamente que o código está correto. Identificar o conflito e propor correção.

## Restrições críticas

Nunca alterar sem autorização explícita e justificativa registrada. Quando houver Spec Kit ativo, essa justificativa deve constar no `spec.md`, `plan.md` ou `tasks.md` antes da implementação:

- Pinagem de GPIO
- Função elétrica dos pinos
- Tabela de partições
- Layout de NVS
- Nomes de namespaces/chaves NVS
- Tópicos MQTT
- Payloads MQTT
- QoS, retain, LWT ou keepalive
- Estratégia OTA
- Certificados, chaves, tokens ou credenciais
- Configuração de TLS
- Baud rate de interfaces externas
- Frequência de barramentos I2C/SPI/UART
- Codec de áudio
- Board config
- Pinagem I2S
- Pinos de MCLK, BCLK, LRCLK/WS, DIN, DOUT
- Pino de reset do codec
- Pino de enable/mute do amplificador
- Sample rate, bits por amostra ou canais padrão
- Pipeline de áudio e ordem dos elementos
- Contrato de comunicação com app Flutter ou backend

Nunca fazer `erase_flash`, `idf.py flash`, alteração destrutiva de NVS ou operação destrutiva em dispositivo real sem solicitação explícita.

## Stack esperada

- Linguagem principal: C/C++
- Framework base: ESP-IDF
- Áudio, quando aplicável: ESP-ADF
- RTOS: FreeRTOS
- Build: CMake via `idf.py`
- Configuração: `sdkconfig`, `sdkconfig.defaults`, `menuconfig`
- Persistência local: NVS
- Conectividade: Wi-Fi, BLE/Blufi quando aplicável, MQTT, TLS
- Logs: `ESP_LOGx`
- Erros: `esp_err_t`

## Comandos principais

Rodar a partir da raiz do projeto:

```bash
idf.py build
```

Quando relevante:

```bash
idf.py size
idf.py menuconfig
idf.py monitor
idf.py fullclean
```

Não usar `idf.py flash`, `idf.py erase_flash` ou comandos destrutivos sem autorização explícita.

## Antes de modificar código

Antes de editar, identificar:

- versão do ESP-IDF, quando disponível
- versão do ESP-ADF, quando aplicável
- alvo configurado, como `esp32`, `esp32s3`, `esp32c3`
- estrutura de componentes
- `CMakeLists.txt` raiz
- `main/CMakeLists.txt`
- `components/*/CMakeLists.txt`
- `sdkconfig.defaults`
- tabela de partições
- arquivos de configuração MQTT/Wi-Fi/BLE/OTA/NVS
- arquivos de configuração de board, codec, I2S e áudio
- módulos que já implementam reconexão, diagnóstico e logs
- módulos que já implementam state machines, filas, timers ou event groups

Não duplicar subsistemas existentes. Preferir integrar no componente correto.

# Clean Code e Clean Architecture para firmware embarcado

Aplicar Clean Code e Clean Architecture de forma pragmática para C/C++ embarcado. A arquitetura deve melhorar confiabilidade, diagnóstico e testabilidade sem criar camadas artificiais que atrapalhem desempenho, memória ou simplicidade.

## Direção de dependências

A regra principal é: lógica de aplicação e domínio não deve depender diretamente de detalhes de infraestrutura.

Camadas conceituais recomendadas:

```text
app/domain
  Regras de negócio, estados do dispositivo, comandos, eventos e diagnósticos.

protocol
  Tópicos MQTT, payloads, versionamento, serialização e desserialização.

connectivity
  Wi-Fi, BLE/Blufi, MQTT, TLS, reconexão e transporte.

storage
  NVS, migrações, persistência e leitura/gravação de configuração.

drivers
  GPIO, ADC, PWM, I2C, SPI, UART, CAN, sensores e atuadores.

audio
  ESP-ADF, pipeline, streams, codec, I2S e controle de player/recorder.

platform
  Adaptações específicas de ESP-IDF/ESP-ADF, board config e hardware.
```

Direção esperada:

- `app/domain` pode conhecer tipos de comando, eventos, estado e interfaces abstratas simples.
- `app/domain` não deve incluir headers de Wi-Fi, MQTT, NVS, I2S, codec, GPIO ou ESP-ADF, salvo quando não houver alternativa prática e a decisão estiver justificada.
- `connectivity`, `storage`, `drivers`, `audio` e `platform` implementam detalhes técnicos.
- `protocol` é dono de tópicos, payloads e versionamento, não de transporte MQTT.
- `app_controller` ou módulo equivalente orquestra os casos de uso.

## Separação de responsabilidades

Cada componente deve ter um motivo principal para mudar.

Evitar componentes que misturam:

- regra de negócio + Wi-Fi
- regra de negócio + MQTT
- regra de negócio + NVS
- regra de negócio + GPIO
- regra de negócio + I2S/codec
- parser de payload + publicação MQTT
- callback ESP-IDF + lógica pesada
- audio element callback + decisão de produto
- configuração de board + lógica de aplicação

Preferir componentes pequenos com APIs explícitas.

## Regras práticas de Clean Code

- Funções devem ter propósito único e nome claro.
- Headers públicos devem ser mínimos.
- Detalhes internos devem ficar em `.c`/`.cpp`, não em headers públicos.
- Structs privadas devem permanecer privadas quando possível.
- Usar handles opacos para componentes com ciclo de vida.
- Evitar variáveis globais mutáveis; quando inevitáveis, proteger e documentar.
- Evitar flags booleanas espalhadas; preferir state machines explícitas.
- Evitar funções gigantes de inicialização que configuram subsistemas não relacionados.
- Evitar callbacks longos; callbacks devem validar, registrar e sinalizar outra task quando necessário.
- Evitar duplicação de lógica de erro, retry, payload e parsing.
- Preferir nomes que expressem intenção, não implementação acidental.
- Evitar comentários que expliquem código confuso; melhorar o código primeiro.
- Comentários são úteis para explicar decisões de hardware, timing, protocolo, compatibilidade e trade-offs.

## Interfaces e injeção de dependência em C

Usar injeção de dependência de forma simples, por exemplo:

- structs de configuração
- callbacks explícitos
- tabelas de funções quando realmente necessário
- handles passados como argumento
- interfaces pequenas para storage, transporte, clock, diagnóstico ou driver

Não criar hierarquias complexas ou abstrações genéricas sem benefício claro.

Exemplo conceitual:

```c
typedef struct {
    esp_err_t (*publish)(void *ctx, const char *topic, const char *payload, int qos, bool retain);
    void *ctx;
} app_mqtt_port_t;
```

Use esse padrão apenas quando ele reduzir acoplamento real ou facilitar teste.

## Testabilidade

Sempre que possível, escrever lógica pura ou quase pura em módulos sem dependência direta de hardware.

Bons candidatos para testes sem hardware:

- parser de payload
- geração de payload
- state machines
- mapeamento de erro para diagnóstico
- validação de configuração
- cálculo de backoff
- migração de schema lógico
- roteamento de comandos
- regras de volume/playback quando independentes do codec

Deixar acesso a hardware em adapters/drivers.

## Anti-padrões proibidos ou suspeitos

Evitar:

- `app_main.c` com toda a aplicação.
- `manager.c` que gerencia tudo.
- `wifi_event_handler()` publicando MQTT, escrevendo NVS e alterando GPIO diretamente.
- `mqtt_event_handler()` com parsing complexo, regra de negócio e acesso a hardware.
- `audio_element` decidindo regra de produto ou acessando NVS/MQTT diretamente.
- `#include` de headers privados entre componentes sem necessidade.
- dependência circular entre componentes.
- estado global acessado por vários módulos sem owner claro.
- retries infinitos sem backoff.
- strings de tópico/payload duplicadas em vários arquivos.
- `sprintf`, cópias sem limite ou buffers sem validação.
- abstração criada apenas “por arquitetura”, sem uso real.

## Checklist de arquitetura antes de finalizar

Antes de concluir uma alteração, verificar:

- A regra de negócio ficou separada de callbacks, drivers e transporte?
- A API pública ficou pequena e estável?
- O componente tem um motivo claro para mudar?
- Há dependência circular?
- O estado do dispositivo está explícito?
- Erros são propagados e diagnosticáveis?
- Logs ajudam em campo sem vazar segredos?
- Mudanças em pinagem, NVS, MQTT, partição, codec, I2S ou pipeline foram evitadas ou justificadas?
- O código novo é testável sem hardware quando possível?
- O build foi executado ou a impossibilidade foi informada?

## Estilo de código C / ESP-IDF

Preferir APIs no estilo ESP-IDF:

- retorno `esp_err_t` para funções públicas
- validação de argumentos no início
- `ESP_RETURN_ON_ERROR`
- `ESP_RETURN_ON_FALSE`
- `ESP_GOTO_ON_ERROR` quando houver cleanup
- `static const char *TAG = "...";` por arquivo
- logs objetivos e acionáveis
- structs privadas no `.c` quando possível
- handles opacos para APIs públicas quando fizer sentido

Evitar:

- funções longas demais
- variáveis globais mutáveis sem proteção
- callbacks com lógica pesada
- `vTaskDelay` como solução genérica para sincronismo
- busy wait
- alocação dinâmica em caminhos críticos
- `malloc/free` repetitivo em loops de alta frequência
- strings MQTT montadas sem limite de tamanho
- `sprintf`; preferir `snprintf`
- logs com dados sensíveis

Toda chamada ESP-IDF relevante deve ter retorno verificado.

## FreeRTOS

Ao criar ou alterar tasks:

- justificar prioridade
- justificar tamanho de stack
- evitar lógica bloqueante em task crítica
- usar filas, event groups, semáforos ou timers quando apropriado
- proteger estado compartilhado com mutex/semaphore
- evitar deadlocks
- evitar starvation
- considerar watchdog
- não bloquear event loop do ESP-IDF
- não fazer trabalho pesado em ISR

Em ISR:

- usar apenas APIs `FromISR`
- manter execução mínima
- sinalizar task para processamento posterior

## Wi-Fi, BLE e MQTT

Toda falha de conectividade deve ser diagnosticável.

Wi-Fi deve registrar, quando disponível:

- evento recebido
- motivo de desconexão
- número de tentativas
- RSSI quando aplicável
- SSID mascarado quando necessário
- estado atual da máquina de conexão

MQTT deve registrar, quando disponível:

- evento MQTT
- erro de transporte
- erro TLS
- erro de socket
- estado de conexão
- tentativas de reconexão
- broker/porta sem expor credenciais
- keepalive configurado
- LWT quando usado

BLE/Blufi deve registrar:

- início e fim do provisionamento
- dados sensíveis sempre mascarados
- sucesso/falha de credenciais recebidas
- transição para tentativa Wi-Fi
- motivo de falha retornável para app quando existir

Toda reconexão deve usar backoff ou estratégia equivalente. Não fazer loop agressivo de reconexão.

## Diagnóstico para o app/backend

Quando o firmware precisar informar falhas ao app ou backend, retornar código estruturado, mensagem curta e dados técnicos úteis.

Exemplo conceitual:

```c
typedef enum {
    IOT_DIAG_OK = 0,
    IOT_DIAG_WIFI_AUTH_FAILED,
    IOT_DIAG_WIFI_NO_AP_FOUND,
    IOT_DIAG_WIFI_ASSOC_FAIL,
    IOT_DIAG_WIFI_HANDSHAKE_TIMEOUT,
    IOT_DIAG_MQTT_DNS_FAILED,
    IOT_DIAG_MQTT_TCP_FAILED,
    IOT_DIAG_MQTT_TLS_FAILED,
    IOT_DIAG_MQTT_AUTH_FAILED,
    IOT_DIAG_MQTT_BROKER_UNREACHABLE,
    IOT_DIAG_NVS_ERROR,
    IOT_DIAG_AUDIO_CODEC_FAILED,
    IOT_DIAG_AUDIO_I2S_FAILED,
    IOT_DIAG_AUDIO_STREAM_TIMEOUT,
    IOT_DIAG_AUDIO_UNSUPPORTED_FORMAT,
    IOT_DIAG_INTERNAL_ERROR
} iot_diag_code_t;
```

Não retornar apenas “erro desconhecido” quando houver motivo técnico disponível.

## MQTT

Ao alterar MQTT:

- preservar tópicos existentes
- preservar compatibilidade de payload
- documentar nova versão de payload, se criada
- evitar payload ambíguo
- validar tamanho antes de publicar
- tratar publicação quando offline
- decidir explicitamente entre QoS 0, 1 ou 2
- decidir explicitamente uso de retained
- configurar LWT quando fizer sentido
- não publicar credenciais, tokens ou dados sensíveis

## NVS

Ao alterar NVS:

- não quebrar dispositivos já provisionados
- manter compatibilidade de chaves antigas quando possível
- criar migração explícita quando necessário
- tratar erro de namespace/chave ausente
- tratar corrupção ou ausência de dados
- não armazenar segredo em texto claro quando houver alternativa segura no projeto

## OTA

Ao alterar OTA:

- preservar segurança do processo
- validar partições
- validar rollback quando configurado
- registrar motivo de falha
- não remover verificação de imagem
- não enfraquecer TLS
- não ignorar erro de escrita, validação ou boot

## Hardware e periféricos

Antes de alterar driver ou periférico:

- conferir pinagem
- conferir nível lógico
- conferir pull-up/pull-down
- conferir frequência de barramento
- conferir modo SPI/I2C/UART
- conferir conflito de GPIO com bootstrapping
- conferir uso por outro componente

Drivers devem isolar detalhes de hardware atrás de API clara.

## Logs

Logs devem ajudar diagnóstico real em campo.

Usar níveis adequados:

- `ESP_LOGE` para falhas
- `ESP_LOGW` para condição anormal recuperável
- `ESP_LOGI` para eventos importantes de ciclo de vida
- `ESP_LOGD` para diagnóstico detalhado
- `ESP_LOGV` apenas quando realmente útil

Não logar:

- senha Wi-Fi
- token
- certificado privado
- chave
- payload sensível
- dados pessoais desnecessários

# ESP-ADF / Áudio

Este projeto também pode usar ESP-ADF, além de ESP-IDF.

ESP-ADF deve ser tratado como camada de áudio sobre ESP-IDF, não como substituto do ESP-IDF.

## Clean Architecture para áudio

Separar claramente:

```text
audio_app
  Comandos de alto nível: play, pause, resume, stop, set_volume, select_source.

audio_domain
  Estado do player/recorder, regras de transição, diagnósticos e políticas de erro.

audio_pipeline
  Criação, ligação, execução, parada e destruição de pipelines ESP-ADF.

audio_board
  Codec, I2S, amplificador, mute, reset, pinos e detalhes do hardware.

audio_streams
  HTTP, SD card, Bluetooth, microfone, tone generator ou streams customizados.
```

Regras:

- `audio_domain` não deve conhecer detalhes de I2S, codec, board config ou ESP-ADF internamente.
- `audio_pipeline` não deve decidir regra de produto; deve executar comandos claros.
- `audio_board` não deve conhecer MQTT, Wi-Fi, app Flutter ou regra de negócio.
- callbacks de audio elements não devem acessar NVS/MQTT/GPIO de aplicação diretamente.
- eventos do pipeline devem ser convertidos em eventos de aplicação/diagnóstico.
- restart de áudio deve ser uma transição explícita de estado, não uma sequência espalhada de chamadas.

## Regras específicas para ESP-ADF

Antes de alterar áudio, identificar:

- versão do ESP-ADF
- versão do ESP-IDF suportada pelo ESP-ADF em uso
- valor esperado de `ADF_PATH`
- placa de áudio ou hardware customizado
- codec de áudio usado
- interface de áudio: I2S, ADC, DAC, PDM ou outra
- pinos de MCLK, BCLK, LRCLK/WS, DOUT, DIN, PA enable, codec reset e controle I2C/SPI
- sample rate
- bits por amostra
- canais: mono/stereo
- ganho/volume/mute
- fonte de áudio: flash, SD card, HTTP, Bluetooth, microfone, tone generator, stream customizado
- saída de áudio: codec, I2S, DAC, arquivo, rede ou stream customizado

Nunca alterar sem decisão explícita no Spec Kit:

- codec de áudio
- driver de codec
- pinagem I2S
- pinos de controle do amplificador
- pino de mute
- pino de enable do PA
- clock de áudio
- sample rate padrão
- bits por amostra
- número de canais
- board config
- pipeline de áudio
- ordem dos elementos no pipeline
- tamanho de ringbuffer
- stack size ou prioridade de tasks de áudio
- fonte ou destino do stream de áudio
- formato de áudio suportado
- comportamento de start/stop/pause/resume
- estratégia de recuperação após underrun, overrun ou falha de stream

## Build ESP-ADF

Quando o projeto usar ESP-ADF, confirmar que o ambiente foi carregado corretamente.

Em Linux/macOS, normalmente:

```bash
. $HOME/esp/esp-adf/export.sh
```

Verificar:

```bash
echo $ADF_PATH
echo $IDF_PATH
idf.py --version
```

O projeto ESP-ADF deve incluir o ADF no build quando aplicável:

```cmake
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
```

Não remover `include($ENV{ADF_PATH}/CMakeLists.txt)` em projetos que dependem de componentes ESP-ADF.

## Audio pipeline

Ao criar ou alterar um `audio_pipeline`:

- registrar elementos em ordem lógica: entrada, processamento, saída
- linkar elementos na mesma ordem do fluxo de áudio
- tratar eventos do pipeline
- tratar eventos dos elementos
- tratar fim de stream
- tratar erro de decoder
- tratar erro de I2S/codec
- tratar pause/resume/stop de forma explícita
- liberar recursos em todos os caminhos de erro
- destruir pipeline, elementos, event interfaces e ringbuffers corretamente
- evitar recriar pipeline repetidamente sem necessidade
- evitar vazamento de memória em restart de áudio

A ordem dos elementos deve ser documentada no código quando não for óbvia.

Exemplo conceitual de pipeline:

```text
input_stream -> decoder -> filter/resampler -> i2s_stream -> codec
```

Não misturar lógica de negócio diretamente dentro de callback pesado de áudio.

## Audio elements

Ao implementar ou alterar `audio_element`:

- manter callbacks curtos e previsíveis
- evitar alocação dinâmica dentro do loop de processamento
- checar todos os retornos
- respeitar tamanho de buffers
- tratar fim de stream
- tratar timeout
- tratar underrun/overrun
- propagar erro para o pipeline
- logar estado de forma útil
- não bloquear indefinidamente

## I2S e codec

Antes de alterar I2S ou codec:

- confirmar modo: master/slave
- confirmar sample rate
- confirmar bits por amostra
- confirmar formato: Philips, MSB, PCM, PDM quando aplicável
- confirmar canais
- confirmar clock source
- confirmar MCLK quando necessário
- confirmar sequência de inicialização do codec
- confirmar controle de volume/mute
- confirmar enable do amplificador
- confirmar pull-ups de I2C do codec
- confirmar conflito de GPIO com bootstrapping

Nunca alterar configuração de I2S apenas para “fazer compilar”.

## Board customizada

Para hardware customizado:

- preferir board abstraction própria no projeto
- não modificar árvore do ESP-ADF diretamente
- documentar codec, pinos, clocks e sequência de init
- manter configuração de placa versionada no repositório
- não depender de alterações locais dentro da pasta clonada do ESP-ADF

## Diagnóstico de áudio

Falhas de áudio devem gerar logs úteis.

Registrar, quando disponível:

- estado do pipeline
- elemento que falhou
- evento recebido
- sample rate
- bits
- canais
- tamanho de ringbuffer
- free heap
- stack high water mark das tasks relevantes
- motivo de erro do codec/I2S/stream
- URL ou origem de stream sem credenciais
- tempo desde início do playback/recording

Diferenciar:

- falha de arquivo
- falha de rede
- falha de decoder
- falha de codec
- falha de I2S
- underrun
- overrun
- timeout
- falta de memória
- formato incompatível
- fim normal de stream

## Integração com conectividade

Quando áudio depender de rede:

- não travar pipeline indefinidamente por Wi-Fi/MQTT offline
- tratar reconexão separadamente do pipeline de áudio
- pausar, parar ou recriar pipeline de forma explícita
- diferenciar erro de rede de erro de decoder
- evitar loop agressivo de restart de stream
- aplicar backoff em stream HTTP/WebSocket/MQTT quando aplicável

## Integração com Spec Kit

Durante `/speckit.specify`, requisitos de áudio devem declarar:

- fonte de áudio
- destino de áudio
- codecs suportados
- sample rates suportados
- comportamento esperado de play/pause/stop/resume
- comportamento offline
- comportamento em falha de decoder
- comportamento em falha de I2S/codec
- comportamento em perda de Wi-Fi
- latência aceitável
- consumo de memória aceitável
- critérios de validação em hardware real

Durante `/speckit.plan`, o plano deve declarar:

- componentes ESP-ADF usados
- pipeline proposto
- ordem dos elementos
- eventos tratados
- estratégia de cleanup
- estratégia de restart
- impacto em heap/stack
- impacto em CPU
- impacto em partições, se houver arquivos de áudio embarcados
- plano de teste com áudio real

Durante `/speckit.tasks`, criar tarefas separadas para:

- board config
- codec/I2S
- pipeline
- streams
- decoder/encoder
- eventos
- diagnóstico/logs
- cleanup
- validação de build
- validação em hardware

Durante `/speckit.implement`, não alterar arquitetura de áudio fora das tarefas aprovadas.

## Testes e validação

Sempre que possível, adicionar ou atualizar testes.

Antes de finalizar alteração de código, rodar:

```bash
idf.py build
```

Quando a alteração envolver tamanho de firmware, memória, partição, OTA ou áudio relevante, rodar também:

```bash
idf.py size
```

Quando não for possível rodar build/teste, informar claramente.

## Critérios de pronto

Uma tarefa só está pronta quando:

- compila sem erro
- não introduz warnings relevantes
- mantém compatibilidade com hardware existente
- não quebra tópicos MQTT existentes
- não quebra schema NVS existente
- não altera codec/I2S/pinagem/board config sem justificativa
- pipeline de áudio cria, inicia, para e libera recursos corretamente quando aplicável
- falhas relevantes possuem logs claros
- erros são propagados por `esp_err_t` ou mecanismo equivalente
- callbacks e ISRs continuam leves
- tasks têm prioridade e stack coerentes
- documentação/spec/tarefas foram atualizadas quando necessário

## Resposta final esperada do agente

Ao terminar uma alteração, responder com:

- resumo do que mudou
- arquivos alterados
- comandos executados
- resultado do build/testes
- riscos restantes
- pontos que exigem teste em hardware real

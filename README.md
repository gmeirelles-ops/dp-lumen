# dp-lumen

Firmware para a luminária de emergência **dp-lumen** (Diponto): bateria UP645 6V,
carga LED1/LED2 @ 6V, MCU **RA-08H** (ASR6601).

Repositório criado a partir do [esp32-template](https://github.com/diponto/esp32-template.git), com GitHub Spec Kit e skills locais para Codex e Cursor.

Hardware de referência: `hw-dp-lumen/04. Projeto` (KiCad rev v0.0.1).

## Estado do projeto

| Item | Valor |
|---|---|
| Target | **RA-08H** (ASR6601) |
| SDK | ASR6601 / Ai-Thinker RA-08H (migração pendente) |
| Hardware | `hw-dp-lumen/04. Projeto` v0.0.1 |
| Constituição | v2.0.0 (2026-06-24) |
| Remoto | `git@github.com:gmeirelles-ops/dp-lumen.git` |
| Firmware | P1+P2 implementados (stub RA-08H); testes de domínio no host |

> **Nota:** o build oficial usa **CMake + RA-08H SDK** (`RA08H_SDK_PATH`). Sem o SDK,
> o projeto compila em modo **stub** com `gcc` para validação de lógica e testes unitários.

## Build (RA-08H / stub)

```bash
# Stub (host): lógica + testes — não gera binário para flash no RA-08H
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure

# Flash dp-lumen no RA-08H (placa em modo BOOT — ver abaixo)
./scripts/flash-dp-lumen.sh
# ou: SERIAL_PORT=/dev/ttyACM0 ./scripts/flash-dp-lumen.sh
```

**Modo download (obrigatório para flash):** segurar **BOOT** (GPIO2 / `LORA.BOOT`), pulsar **RST**, soltar RST, depois rodar o flash. O adaptador USB‑serial precisa ligar **DTR→BOOT** e **RTS→RST** (como no J2), ou fazer isso manualmente.

Se `Connect failed: Read response header timeout`: porta errada, placa não em BOOT, ou cabo só com TX/RX.

```bash
# Binário gerado:
# sdk/ra08h-dp-lumen/out/dp-lumen.bin
```

### Botões na barra de status (Build / Flash / Monitor)

Igual ao fluxo do `ra08h-dp-comm`: com o devcontainer (`.devcontainer`, imagem
`ra08h-env`, SDK em `/sdk`) e a extensão `julynx.project-actions`, a barra inferior
mostra **🔨 Build**, **🧹 Clean**, **🚀 Flash** e **Monitor** (config em
`.vscode/.project-actions.json`, espelhados em `.vscode/tasks.json`). Build/Clean/Flash
rodam `make` em `sdk/ra08h-dp-lumen`; o Monitor abre `miniterm` em **115200** (UART de
diagnóstico J2, diferente dos 9600 do `ra08h-dp-comm`). O flash continua exigindo a
placa em modo BOOT/download (ver acima) e autorização explícita.

### Telemetria LoRa P2P (P3)

Uplink-only, compatível com o gateway `ra08h-dp-comm` (915 MHz, BW500, SF7, CR4/5,
22 dBm). Payload de 3 bytes `[product_id=4][flags AC/carga][bateria %]`, com checksum
+ `crypt` idênticos ao gateway. Envio por mudança (modo/faixa/carga/rede) e heartbeat
de 15 min; sem transmissão em hibernação. Contrato:
`specs/001-emergency-luminaire-firmware/contracts/telemetry-lora.md`.

**Problemas comuns**

| Erro | Causa | Correção |
|---|---|---|
| `stdint-gcc.h: No such file` | `gcc-arm-none-eabi` 14+ no PATH | `./scripts/patch-ai-thinker-sdk.sh` |
| `common.mk: Arquivo inexistente` | `source build/envsetup.sh` fora da raiz do SDK | `cd Ai-Thinker-LoRaWAN-Ra-08` antes do `source` |
| `ninja: build.ninja not found` | Extensão ESP-IDF no VS Code | Use task **dp-lumen: cmake build (stub)** ou `cmake --build build` |
| `RA08H_SDK_PATH=/caminho/para/...` | placeholder literal | exporte o path real do clone |


Não executar flash/erase sem autorização explícita (AGENTS.md).

A ideia deste template é simples: você conduz o projeto pelos comandos do Spec Kit; o agente no Codex ou no Cursor executa inspeção, Git, geração de arquivos, build e validações. Você não precisa operar manualmente os scripts `.specify/`, comandos Git ou `idf.py`, salvo quando quiser conferir algo por conta própria.

## O Que Vem Pronto

- `AGENTS.md`: regras de segurança, arquitetura, diagnóstico e validação para firmware ESP32.
- `$esp-idf` e `/esp-idf`: skill local para ESP-IDF.
- `$esp-adf` e `/esp-adf`: skill local para ESP-ADF/audio.
- `$speckit-*` no Codex e `/speckit-*` no Cursor: skills locais do GitHub Spec Kit.
- `.specify/`: infraestrutura Spec Kit, constituição, templates e scripts.
- `.agents/skills/`: skills para Codex.
- `.cursor/skills/` e `.cursor/rules/`: skills e regras para Cursor.
- Scripts Spec Kit para Linux/macOS/WSL/Git Bash e Windows PowerShell.
- `.gitignore` e `.gitattributes` para um template portável entre Windows, Linux e macOS.

## Responsabilidades

Você faz:

- descreve o objetivo;
- responde perguntas de esclarecimento;
- aprova decisões de requisito, hardware, protocolo e arquitetura;
- pede commit/push quando quiser publicar.

O agente no Codex ou Cursor faz:

- lê `AGENTS.md`, constituição, spec, plano, tarefas e código;
- executa comandos Git necessários;
- cria ou altera arquivos;
- roda scripts Spec Kit auxiliares;
- roda `idf.py build` e `idf.py size` quando aplicável;
- informa claramente quando algo não puder ser validado;
- nunca roda `idf.py flash`, `idf.py erase_flash` ou operação destrutiva em hardware sem pedido explícito.

## Criar Um Projeto A Partir Do Template

Abra uma conversa no Codex ou no Cursor no diretório onde você quer trabalhar e diga:

```text
Crie um novo projeto a partir do template https://github.com/diponto/esp32-template.git.
Nome do projeto: meu-firmware-esp32.
Novo remoto: git@github.com:diponto/meu-firmware-esp32.git.
Prepare para ESP-IDF, target esp32s3.
Não implemente firmware ainda.
```

O agente deve:

- clonar ou copiar o template;
- configurar o remoto do novo projeto;
- criar branch inicial;
- verificar Spec Kit;
- verificar ESP-IDF;
- preparar ou orientar a estrutura mínima do projeto;
- atualizar a constituição quando você aprovar;
- commitar/pushar se você pedir.

No Cursor, abra o repositório na IDE e use os mesmos pedidos em chat, trocando comandos `$speckit-*` por `/speckit-*`. O arquivo `.cursor/rules/specify-rules.mdc` aponta para `AGENTS.md`, então as mesmas regras de ESP32, segurança, arquitetura e validação continuam valendo.

Se o projeto for de áudio, diga também:

```text
Este projeto usa ESP-ADF. Preparar o fluxo considerando codec, I2S, board config, pipeline de áudio e validação em hardware real.
```

## Preparar A Constituição Do Projeto

Primeiro comando Spec Kit recomendado:

```text
$speckit-constitution
```

Informe as decisões iniciais. Exemplo:

```text
Este projeto usa ESP-IDF com target esp32s3, hardware customizado, Wi-Fi, MQTT/TLS, OTA e NVS. Deve manter compatibilidade com dispositivos provisionados. Pinagem, partição, NVS, tópicos MQTT, TLS e OTA só podem mudar com spec, plano e tarefas explícitos. Build obrigatório com idf.py build. Não fazer flash sem autorização.
```

Para áudio:

```text
Também usa ESP-ADF. Codec, I2S, sample rate, board config, pipeline e ordem dos elementos são contratos de hardware e não podem mudar sem decisão explícita.
```

O agente deve atualizar `.specify/memory/constitution.md` e, se necessário, ajustar `AGENTS.md`.

## Modos De Trabalho Com Spec Kit

Quando o ambiente oferecer seleção de modo pela UI, prefira selecionar o modo antes de enviar o comando Spec Kit. A seleção pela UI é mais forte que uma instrução em texto porque pode persistir objetivo, planejamento ou regras extras entre turnos.

Use esta regra prática:

| Comando | Modo recomendado |
|---|---|
| `$speckit-constitution` | Planejamento |
| `$speckit-specify` | Planejamento |
| `$speckit-clarify` | Planejamento leve quando houver ambiguidade relevante |
| `$speckit-checklist` | Normal |
| `$speckit-plan` | Planejamento |
| `$speckit-tasks` | Planejamento |
| `$speckit-analyze` | Normal |
| `$speckit-implement` | Meta |

No Cursor, use os mesmos critérios trocando `$speckit-*` por `/speckit-*`.

Use **modo planejamento** para fases que tomam decisões ou produzem artefatos de projeto. Durante planejamento, o agente deve entender contexto, riscos, contratos sensíveis e decisões pendentes antes de escrever spec, plano ou tarefas. Não deve implementar código.

Use **modo meta** principalmente em `$speckit-implement`, quando a implementação for longa ou sensível. O objetivo é evitar encerramento cedo demais: o agente deve manter a implementação ativa até concluir as tarefas, revisar o diff, validar o que for possível e reportar pendências.

Se você esquecer de selecionar o modo pela UI, o fallback automático fica em `AGENTS.md`: durante `$speckit-implement`, o agente deve agir em modo meta; durante spec, plano e tasks, deve agir em modo planejamento.

## Fluxo Para Adicionar Um Recurso Ao Firmware

Use este fluxo para novos recursos: Wi-Fi, BLE/Blufi, MQTT, OTA, NVS, sensores, drivers, comandos remotos, diagnóstico, áudio, etc.

### 1. Especificar

```text
$speckit-specify
```

Descreva o recurso pelo comportamento esperado, não pela implementação. Exemplo:

```text
Adicionar leitura de sensor I2C de temperatura. O firmware deve publicar a temperatura no MQTT a cada 60 segundos, preservar tópicos existentes, validar falhas de I2C, gerar diagnóstico estruturado e não alterar pinagem sem decisão explícita.
```

### 2. Esclarecer

```text
$speckit-clarify
```

Responda às perguntas do agente. Para firmware, esclareça principalmente:

- pinagem;
- frequência I2C/SPI/UART;
- NVS namespace/chaves;
- tópicos e payloads MQTT;
- QoS, retain, LWT e keepalive;
- comportamento offline;
- diagnóstico esperado;
- validação em hardware real.

### 3. Checklist

```text
$speckit-checklist
```

Peça uma checklist adequada ao risco. Exemplo:

```text
Gerar checklist para requisitos de firmware ESP-IDF, compatibilidade NVS/MQTT, diagnóstico de campo e validação em hardware real.
```

### 4. Planejar

```text
$speckit-plan
```

O plano deve declarar:

- componentes ESP-IDF/ESP-ADF afetados;
- fronteiras de arquitetura;
- tasks FreeRTOS, filas, timers ou event groups;
- impactos em NVS, MQTT, OTA, TLS, pinagem ou partições;
- logs e diagnósticos;
- estratégia de teste;
- validação em hardware.

### 5. Gerar Tarefas

```text
$speckit-tasks
```

As tarefas devem ser pequenas, ordenadas e específicas por arquivo. Se aparecer tarefa vaga, peça ao agente para dividir.

### 6. Implementar

```text
$speckit-implement
```

O agente deve implementar somente o que está em `tasks.md`, rodar validações possíveis e reportar:

- arquivos alterados;
- comandos executados;
- resultado do build/testes;
- riscos restantes;
- validação em hardware pendente.

## Fluxo Para Editar Um Recurso Existente

Use para alterar comportamento já existente: reconexão Wi-Fi, retry MQTT, payload, regra de domínio, diagnóstico, storage, OTA, driver, etc.

Comando inicial:

```text
$speckit-specify
```

Descreva três coisas:

- comportamento atual;
- comportamento desejado;
- contratos que não podem mudar.

Exemplo:

```text
Editar o recurso MQTT existente para publicar diagnóstico de falha de Wi-Fi. Preservar tópicos existentes, payloads atuais, QoS, retain, LWT, keepalive e compatibilidade com dispositivos já provisionados. Se for necessário novo payload, documentar versionamento e migração antes de implementar.
```

Depois siga:

```text
$speckit-clarify
$speckit-plan
$speckit-tasks
$speckit-implement
```

O agente deve mapear o código existente, encontrar contratos sensíveis, implementar a menor alteração correta e rodar `idf.py build`.

## Fluxo Para Refatorar Um Projeto Existente

Use para melhorar arquitetura sem mudar comportamento externo.

Comando inicial:

```text
$speckit-specify
```

Descreva a refatoração como preservação de comportamento:

```text
Refatorar o firmware existente sem alterar comportamento externo. Preservar pinagem, partições, NVS, tópicos MQTT, payloads, QoS, TLS, OTA e provisionamento. Separar callbacks de Wi-Fi/MQTT/NVS/GPIO em componentes menores, mantendo build verde e logs equivalentes ou melhores.
```

Depois siga:

```text
$speckit-clarify
$speckit-plan
$speckit-tasks
$speckit-implement
```

Boas refatorações para pedir:

- extrair parsing para `protocol`;
- isolar NVS em `storage`;
- isolar GPIO/sensores em `drivers`;
- mover lógica pesada de callbacks para tasks;
- tornar state machines explícitas;
- substituir `sprintf` por `snprintf`;
- melhorar logs e diagnósticos;
- reduzir acoplamento sem criar abstrações vazias.

Não misture refatoração com mudança funcional sem explicitar isso na spec.

## Fluxo Para Áudio ESP-ADF

Quando envolver áudio, comece declarando:

```text
$esp-adf
```

Depois use o fluxo Spec Kit:

```text
$speckit-specify
$speckit-clarify
$speckit-plan
$speckit-tasks
$speckit-implement
```

Na especificação de áudio, declare:

- fonte de áudio;
- destino de áudio;
- codec;
- board;
- MCLK, BCLK, LRCLK/WS, DIN e DOUT;
- PA enable, mute e reset;
- sample rate, bits e canais;
- comportamento play/pause/resume/stop;
- comportamento offline;
- falhas de rede, decoder, I2S e codec;
- estratégia de cleanup/restart;
- validação em hardware real.

Não altere codec, I2S, board config, pipeline ou ordem dos elementos apenas para compilar.

## Pedir Commit E Push

Quando quiser publicar:

```text
Faça commit e push das alterações.
```

Ou seja mais específico:

```text
Faça commit e push apenas das alterações do README.
Mensagem: docs: simplify template workflow
```

O agente deve revisar `git status`, evitar incluir arquivos fora do escopo, commitar, pushar e informar o hash.

## Validação Esperada

O agente deve rodar:

- `git diff --check` para alterações de texto/código;
- `idf.py build` para alterações de firmware;
- `idf.py size` quando tamanho, memória, partição, OTA ou áudio forem afetados;
- validações específicas de scripts ou skills quando o template for alterado.

Se alguma validação não puder ser executada, o agente deve dizer o motivo.

## Operações Que Exigem Pedido Explícito

O agente não deve executar sem autorização clara:

```text
idf.py flash
idf.py erase_flash
operações destrutivas de NVS
alterações destrutivas em dispositivo real
```

Também não deve alterar sem spec/plano/tarefas:

- pinagem GPIO;
- função elétrica dos pinos;
- tabela de partições;
- layout e chaves NVS;
- tópicos/payloads MQTT;
- QoS, retain, LWT ou keepalive;
- TLS, certificados, chaves ou credenciais;
- estratégia OTA;
- codec, I2S, board config ou pipeline de áudio.

## Atualizar O Spec Kit Do Template

Peça ao agente:

```text
Atualize o GitHub Spec Kit deste template, preserve suporte Windows/Linux/macOS e valide as skills.
```

O agente deve usar o inicializador oficial do Spec Kit, revisar o diff, preservar scripts Bash e PowerShell, ajustar skills se necessário, validar e só então commitar/pushar quando solicitado.

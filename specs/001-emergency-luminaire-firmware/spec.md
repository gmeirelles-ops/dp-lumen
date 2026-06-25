# Feature Specification: Emergency Luminaire Firmware

**Feature Branch**: `001-emergency-luminaire-firmware`

**Created**: 2026-06-24

**Status**: Draft

**Input**: Firmware para controle e monitoramento de luminária de emergência com
bateria UP645 SEG (chumbo-ácido 6V 4.5Ah), lâmpadas 6V, indicação visual,
detecção de rede, proteção de bateria e telemetria LoRa (sem comandos remotos).

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Operação de emergência na bateria (Priority: P1)

Quando a rede elétrica falha, a luminária deve iluminar automaticamente o ambiente
usando a bateria de backup, permitir que o usuário desligue as lâmpadas para
preservar energia e alertar quando a bateria estiver fraca ou em risco crítico.

**Why this priority**: Garante a função essencial de segurança da luminária de
emergência — iluminação automática em queda de energia e proteção da bateria.

**Independent Test**: Simular ausência de rede com bateria carregada; verificar
acendimento automático das lâmpadas, indicadores corretos e resposta ao botão
de power sem rede presente.

**Acceptance Scenarios**:

1. **Given** rede presente e lâmpadas desligadas, **When** a rede é removida,
   **Then** as lâmpadas principais acendem automaticamente em até 1 segundo e
   o indicador de rede apaga.
2. **Given** modo bateria com lâmpadas ligadas, **When** o usuário pressiona
   `btn.power`, **Then** as lâmpadas alternam entre ligado e desligado.
3. **Given** modo bateria, **When** o usuário pressiona `btn.test`, **Then**
   nada acontece (botão ignorado).
4. **Given** modo bateria, **When** a tensão da bateria cai para 5.8V ou menos,
   **Then** o indicador de bateria fraca acende.
5. **Given** modo bateria com lâmpadas ligadas, **When** a tensão da bateria
   cai para 5.4V ou menos, **Then** as lâmpadas desligam e o dispositivo entra
   em modo de proteção por subtensão (hibernação profunda).
6. **Given** modo proteção por subtensão, **When** a rede retorna ou a tensão
   da bateria sobe acima de 5.6V, **Then** o dispositivo retoma operação normal.

---

### User Story 2 - Operação com rede presente (Priority: P2)

Com energia da rede disponível, a luminária recarrega a bateria (via hardware),
mantém as lâmpadas desligadas por padrão, indica status de rede e carga, e
permite teste de manutenção das lâmpadas.

**Why this priority**: Operação diária segura — recarga passiva, indicação clara
para o instalador e teste de lâmpadas sem depender da bateria.

**Independent Test**: Aplicar rede elétrica com bateria em diferentes níveis de
carga; verificar indicadores, botão de teste e que lâmpadas permanecem off até
teste manual.

**Acceptance Scenarios**:

1. **Given** rede presente, **When** o dispositivo está em operação normal,
   **Then** o indicador de rede permanece ligado e as lâmpadas permanecem
   desligadas por padrão.
2. **Given** rede presente e bateria com tensão ≥ 6.7V, **When** o sistema
   avalia o estado da bateria, **Then** o indicador de bateria carregada acende.
3. **Given** rede presente e bateria com tensão ≤ 5.8V, **When** o sistema
   avalia o estado da bateria, **Then** o indicador de bateria fraca acende.
4. **Given** rede presente, **When** o técnico pressiona `btn.test` repetidamente,
   **Then** as lâmpadas alternam ligado/desligado a cada pressão válida.
5. **Given** rede presente, **When** o usuário pressiona `btn.power`, **Then**
   nada acontece (botão ignorado).
6. **Given** rede presente, **When** o dispositivo está em modo bateria com
   lâmpadas ligadas e a rede retorna, **Then** as lâmpadas desligam (retorno ao
   padrão de modo rede) salvo se um ciclo de teste estiver ativo via `btn.test`.

---

### User Story 3 - Telemetria LoRa para gestão de frota (Priority: P3)

Operadores de frota precisam receber remotamente o status da rede elétrica e da
bateria de cada luminária, sem enviar comandos de controle nesta versão.

**Why this priority**: Permite monitoramento centralizado sem alterar a lógica
local crítica de emergência; comandos remotos ficam para versão futura.

**Independent Test**: Com gateway LoRa disponível, provocar mudanças de rede e
nível de bateria; confirmar que telemetria reflete o estado em tempo aceitável.

**Acceptance Scenarios**:

1. **Given** dispositivo operando normalmente, **When** o estado de rede ou faixa
   de bateria muda, **Then** uma atualização de telemetria é enviada em até 60
   segundos.
2. **Given** dispositivo em modo rede com bateria carregada, **When** telemetria
   é transmitida, **Then** o pacote indica rede presente e bateria na faixa
   carregada.
3. **Given** dispositivo em modo bateria com bateria fraca, **When** telemetria
   é transmitida, **Then** o pacote indica rede ausente e bateria na faixa fraca.
4. **Given** qualquer mensagem recebida via rádio, **When** contém comando de
   controle remoto, **Then** o dispositivo ignora o comando e mantém comportamento
   local inalterado.

---

### Edge Cases

- Flapping de detecção de rede na fronteira do limiar: o sistema deve estabilizar
  a transição de modo sem oscilar lâmpadas ou indicadores mais de uma vez por
  evento real de queda/retorno.
- `btn.test` pressionado no instante exato da queda de rede: apenas `btn.power`
  passa a ter efeito após confirmação do modo bateria.
- Entrada em proteção por subtensão com lâmpadas ligadas: lâmpadas desligam antes
  ou junto com a hibernação.
- Retomada por tensão de bateria > 5.6V sem rede: dispositivo acorda e retoma modo
  bateria (lâmpadas ON por padrão).
- Falha ou indisponibilidade do rádio LoRa: operação local de emergência continua
  sem degradação.
- Repique mecânico de botão dentro de 50 ms: conta como um único pressionamento.

## Requirements *(mandatory)*

### Functional Requirements

**Detecção e modos**

- **FR-001**: O sistema MUST detectar presença de rede elétrica via sinal lógico
  `AC_DETECT` (1 = rede presente, 0 = rede ausente).
- **FR-002**: O sistema MUST aplicar histerese na detecção de rede para evitar
  oscilação rápida entre modos (detalhes de limiar na fase de planejamento).
- **FR-003**: Em modo rede (`AC_DETECT` = 1), as lâmpadas principais MUST
  permanecer desligadas por padrão.
- **FR-004**: Em modo bateria (`AC_DETECT` = 0), ao entrar no modo, as lâmpadas
  principais MUST acender automaticamente.

**Indicadores visuais**

- **FR-005**: `led.status` MUST estar ligado somente quando `AC_DETECT` = 1.
- **FR-006**: `led.battery` MUST estar ligado quando a tensão da bateria for
  ≤ 5.8V (aproximadamente 10% de carga), em qualquer modo exceto proteção por
  subtensão onde indicadores podem ser desligados para economia.
- **FR-007**: `led.load` MUST estar ligado quando a tensão da bateria for ≥ 6.7V
  **e** `AC_DETECT` = 1; MUST estar desligado em modo bateria (economia de energia).

**Botões**

- **FR-008**: `btn.test` MUST alternar lâmpadas ligado/desligado somente quando
  `AC_DETECT` = 1.
- **FR-009**: `btn.power` MUST alternar lâmpadas ligado/desligado somente quando
  `AC_DETECT` = 0 e o dispositivo não está em proteção por subtensão.
- **FR-010**: `btn.test` MUST ser ignorado quando `AC_DETECT` = 0.
- **FR-011**: `btn.power` MUST ser ignorado quando `AC_DETECT` = 1.
- **FR-012**: Ambos os botões MUST usar debounce de 50 milissegundos.

**Bateria e proteção**

- **FR-013**: O sistema MUST monitorar continuamente a tensão da bateria via
  `V_BATT_ADC`.
- **FR-014**: Quando tensão ≤ 5.4V, o sistema MUST desligar lâmpadas e entrar
  em hibernação profunda (proteção por subtensão).
- **FR-015**: O sistema MUST retomar da hibernação profunda quando `AC_DETECT` = 1
  **ou** tensão da bateria > 5.6V.
- **FR-016**: O carregamento da bateria com rede presente é realizado pelo circuito
  analógico; o firmware MUST NOT controlar o carregador.

**Telemetria LoRa (uplink apenas — LoRa P2P)**

A telemetria usa **LoRa P2P cru** (sem LoRaWAN/join), wire-compatível com o gateway
`ra08h-dp-comm`. Detalhes de rádio, framing e payload em
[`contracts/telemetry-lora.md`](./contracts/telemetry-lora.md).

- **FR-017**: O sistema MUST transmitir telemetria contendo `product_id`
  (provisório `4`), presença de rede (AC presente/ausente), porcentagem da bateria
  (0–100 %) e estado da carga das lâmpadas (ligado/desligado).
- **FR-018**: O sistema MUST retransmitir telemetria quando o modo operacional,
  a faixa de bateria, o estado da carga ou a presença de rede mudar.
- **FR-019**: O sistema MUST transmitir telemetria periódica em intervalo máximo
  de 15 minutos durante operação normal (heartbeat).
- **FR-020**: Comandos remotos via rádio MUST NOT alterar o comportamento do
  dispositivo nesta versão (rádio configurado apenas para transmissão).

**Diagnóstico**

- **FR-021**: O sistema MUST registrar estados operacionais (modo, faixa de bateria,
  estado das lâmpadas, botões ignorados) para suporte em campo via interface de
  diagnóstico local.

### Key Entities

- **OperatingMode**: modo atual — `MainsPresent`, `BatteryEmergency`,
  `DeepSleepProtection`.
- **BatteryBand**: faixa derivada de `V_BATT_ADC` — `Charged` (≥ 6.7V), `Normal`
  (> 5.8V e < 6.7V), `Low` (≤ 5.8V e > 5.4V), `Critical` (≤ 5.4V).
- **LoadState**: estado das lâmpadas principais — `On` ou `Off`.
- **TelemetrySnapshot**: instantâneo enviado por uplink — `product_id`, AC presente,
  porcentagem da bateria (0–100 %), estado da carga. Codificado em 3 bytes de payload
  conforme `contracts/telemetry-lora.md`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Com rede presente, o indicador de rede está visível em 100% dos
  ciclos de teste em bancada (mínimo 20 ciclos).
- **SC-002**: Na queda de rede, as lâmpadas acendem sem intervenção do usuário em
  até 1 segundo em 100% dos testes (mínimo 10 quedas simuladas).
- **SC-003**: Em 100 pressionamentos debounced por botão, `btn.test` só altera
  lâmpadas com rede presente e `btn.power` só altera lâmpadas em modo bateria —
  zero falsos positivos.
- **SC-004**: Com tensão de bateria ≤ 5.4V, lâmpadas desligam e o consumo cai
  para nível de hibernação mensurável em bancada.
- **SC-005**: Após mudança de modo ou faixa de bateria, telemetria reflete o novo
  estado em até 60 segundos em 95% dos eventos de teste.
- **SC-006**: Nenhum comando remoto recebido altera o estado das lâmpadas ou
  indicadores durante testes de penetração de downlink.

## Assumptions

- Hardware de referência: luminária dp-lumen KiCad rev v0.0.1 com bateria UP645
  SEG (chumbo-ácido 6V 4.5Ah) e duas lâmpadas principais de 6V.
- Mapeamento lógico para hardware documentado na constituição v2.0.0: `AC_DETECT`
  derivado de leitura analógica do rail de 12V; `V_BATT_ADC` do divisor de
  bateria; `LAMP_CTRL` no enable de potência das lâmpadas; rádio LoRa integrado
  ao módulo de controle (sem módulo SPI externo).
- Conversão de tensões reais (5.4V–6.7V) para valores de leitura analógica será
  calibrada na fase de planejamento conforme divisor do esquemático.
- Modo bateria: acionamento automático das lâmpadas ao perder rede (atualização
  em relação à constituição v2.0.0 que previa apenas toggle manual).
- `led.load` desligado em modo bateria para economia de energia.
- Telemetria LoRa incluída nesta feature; comandos remotos explicitamente excluídos.
- Telemetria usa **LoRa P2P cru** (não LoRaWAN), compatível com o gateway
  `ra08h-dp-comm` (mesmos parâmetros de rádio e framing checksum + `crypt`).
- `product_id = 4` é **provisório** (ainda não definido); não há identificador único
  por dispositivo nesta versão.
- Porcentagem da bateria derivada de VBAT por mapeamento linear (5.4 V = 0 %,
  6.7 V = 100 %), aproximação aceita para monitoramento de frota.
- Wake de hibernação profunda: retorno de rede **ou** tensão de bateria > 5.6V.
- Intervalo de heartbeat de telemetria: 15 minutos quando não há mudança de estado.
- Instaladores e técnicos de manutenção são os usuários primários dos botões de
  teste e power; operadores de frota são consumidores da telemetria.

## Out of Scope

- Comandos remotos LoRa (downlink de controle de lâmpadas ou configuração).
- LoRaWAN (join, chaves, network server) — esta versão usa LoRa P2P cru.
- Identificador único por dispositivo na telemetria (apenas `product_id` nesta versão).
- Controle do circuito carregador analógico (LM317 / VIPer22).

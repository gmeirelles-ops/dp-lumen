# ESP32 Firmware Template

Template para projetos ESP32-family com ESP-IDF, opção de ESP-ADF, GitHub Spec Kit e skills locais para Codex.

## O Que Vem Pronto

- `AGENTS.md` com regras de arquitetura, segurança, diagnóstico e validação para firmware ESP32.
- Skills locais em `.agents/skills/`:
  - `$esp-idf` para firmware ESP-IDF geral.
  - `$esp-adf` para áudio com ESP-ADF.
  - `$speckit-*` para o fluxo GitHub Spec Kit.
- Infraestrutura Spec Kit em `.specify/`.
- Scripts Spec Kit para macOS/Linux/WSL/Git Bash e para Windows PowerShell.

## Pré-Requisitos

- Git.
- Python 3.11 ou superior para o Specify CLI.
- `uv`/`uvx` para rodar o Spec Kit sem instalação global.
- ESP-IDF instalado e exportado conforme o sistema operacional.
- ESP-ADF apenas quando o projeto usar áudio.

## Spec Kit

Este template já foi inicializado com:

```bash
uvx --from git+https://github.com/github/spec-kit.git specify init --here --force --integration codex --integration-options="--skills" --script sh --ignore-agent-tools
```

Os scripts de automação foram mantidos nos dois formatos:

```text
.specify/scripts/bash/        # Linux, macOS, WSL ou Git Bash
.specify/scripts/powershell/  # Windows PowerShell
```

Exemplos de verificação:

```bash
.specify/scripts/bash/check-prerequisites.sh --help
```

```powershell
.\.specify\scripts\powershell\check-prerequisites.ps1 -Help
```

Fluxo recomendado no Codex:

```text
$speckit-constitution
$speckit-specify
$speckit-clarify
$speckit-checklist
$speckit-plan
$speckit-tasks
$speckit-implement
```

Durante fases de especificação, planejamento e geração de tarefas, não implemente código. Durante implementação, siga `spec.md`, `plan.md` e `tasks.md`.

## ESP-IDF

Depois de criar um projeto derivado deste template, defina explicitamente:

- target ESP32 (`esp32`, `esp32s3`, `esp32c3`, etc.)
- versão ESP-IDF
- pinagem e board revision
- partição/OTA
- NVS schema
- Wi-Fi/BLE/Blufi/MQTT/TLS
- diagnósticos esperados pelo app/backend
- configurações ESP-ADF quando áudio for usado

Build padrão:

```bash
idf.py build
```

Quando tamanho de firmware, memória, partição, OTA ou áudio forem afetados:

```bash
idf.py size
```

Não rode `idf.py flash`, `idf.py erase_flash` ou operações destrutivas de NVS em hardware real sem solicitação explícita.

## Atualizando O Spec Kit

Para consultar a versão disponível:

```bash
uvx --from git+https://github.com/github/spec-kit.git specify version
```

Para atualizar a infraestrutura do template, faça em uma branch separada e revise o diff. O template mantém ajustes cross-platform nas skills do Spec Kit, então preserve as referências a Bash e PowerShell antes de publicar.

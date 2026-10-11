# Scarlett 2i2 Control (Linux)

Controle em C rápido, leve e sem dependências pesadas de desenvolvimento para a **Focusrite Scarlett 2i2** no Linux, baseado no funcionamento do [`alsa-scarlett-gui`](https://github.com/geoffreybennett/alsa-scarlett-gui) e no driver do kernel Linux (`mixer_scarlett2`).

Controla exatamente o que você precisa:
* **Entradas (Input 1 e Input 2):**
  * Nível de Impedância: **Line** (Linha) ou **Inst** (Instrumento)
  * Modo **AIR** (Boost de presença Focusrite)
* **Alimentação Fantasma:**
  * **+48V Phantom Power** para microfones condensadores
* **Saída / Monitoramento (Direct Monitor):**
  * **Off** (Desligado / áudio apenas do PC)
  * **Mono** (Soma as entradas para ambos os canais de fone/monitor)
  * **Stereo** (Entrada 1 no canal esquerdo, Entrada 2 no canal direito)
* **Bandeja do Sistema (System Tray):**
  * Ícone na system tray (KDE Plasma, GNOME via AppIndicator, XFCE, etc.)
  * Menus com checkboxes e radio buttons intuitivos
  * **Sincronização em tempo real com o hardware físico:** se você apertar o botão físico na Scarlett 2i2, a bandeja atualiza o estado imediatamente via eventos ALSA do kernel (`SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS`).

---

## 🚀 Compilação

O projeto foi projetado para compilar em **qualquer ambiente Linux** sem exigir pacotes `-devel` pesados (como `libasound-dev`, `libgtk-3-dev` ou `libappindicator-dev`). Ele utiliza os headers padrão do kernel (`<sound/asound.h>`) e carrega as bibliotecas da interface gráfica dinamicamente em tempo de execução via `dlopen`.

Para compilar, basta rodar:

```bash
cd scarlett-control
make
```

Isso gera o binário executável `scarlett-control` em segundos.

---

## 🖥️ Uso na System Tray (Bandeja do Sistema)

Para iniciar o controle na bandeja do sistema:

```bash
./scarlett-control tray
```

*(Se executado sem argumentos em um ambiente gráfico X11 ou Wayland, o modo `tray` é iniciado automaticamente).*

### Menu da Bandeja:
* **[ Input 1 ]**
  * ☑️ Inst (Instrument)
  * ☑️ Air Mode
* **[ Input 2 ]**
  * ☑️ Inst (Instrument)
  * ☑️ Air Mode
* **[ Power ]**
  * ☑️ +48V Phantom Power
* **[ Output (Direct Monitor) ]**
  * 🔘 Direct Monitor: Off
  * 🔘 Direct Monitor: Mono
  * 🔘 Direct Monitor: Stereo
* ❌ Sair (Quit)

---

## ⌨️ Uso via Linha de Comando (CLI & Scripts)

Você também pode consultar e alterar qualquer parâmetro diretamente pelo terminal, ideal para atalhos de teclado (ex: no KDE Shortcuts ou Sway/Hyprland) ou automações:

### 1. Ver status atual:
```bash
./scarlett-control status
```
Ou com saída JSON:
```bash
./scarlett-control status --json
```

### 2. Alterar configurações (`set`):
```bash
# Ativar Instrumento e Air na entrada 1, 48V ligado e Direct Monitor em Stereo:
./scarlett-control set --inst1 on --air1 on --48v on --output stereo

# Mudar monitoramento para mono:
./scarlett-control set --output mono

# Desligar 48V:
./scarlett-control set --48v off
```

### 3. Alternar estados (`toggle`):
Excelente para vincular a teclas de atalho:
```bash
./scarlett-control toggle air1      # Alterna Air na entrada 1 (on/off)
./scarlett-control toggle inst1     # Alterna Inst na entrada 1
./scarlett-control toggle air2      # Alterna Air na entrada 2
./scarlett-control toggle inst2     # Alterna Inst na entrada 2
./scarlett-control toggle 48v       # Alterna 48V Phantom Power
./scarlett-control toggle monitor   # Cicla entre Off -> Mono -> Stereo -> Off
```

### 4. Monitorar eventos do hardware em tempo real (`watch`):
Exibe o estado e atualiza instantaneamente quando você aperta qualquer botão físico na placa de áudio:
```bash
./scarlett-control watch
```

---

## 🌐 Internacionalização (i18n Multi-Idiomas)

O `scarlett-control` detecta automaticamente o idioma da sua sessão Linux através das variáveis de ambiente (`LANG`, `LC_MESSAGES`, `LC_ALL`) ou permite selecionar manualmente qualquer idioma suportado:

| Código | Idioma | Arquivo de Tradução |
|:---:|:---|:---|
| `en` | Inglês (*English*) | `locales/en.json` |
| `pt` | Português do Brasil | `locales/pt_BR.json` |
| `es` | Espanhol (*Español*) | `locales/es.json` |
| `fr` | Francês (*Français*) | `locales/fr.json` |
| `de` | Alemão (*Deutsch*) | `locales/de.json` |
| `it` | Italiano (*Italiano*) | `locales/it.json` |
| `ja` | Japonês (*日本語*) | `locales/ja.json` |
| `zh` | Chinês Simplificado (*简体中文*) | `locales/zh_CN.json` |

Você pode definir o idioma manualmente tanto na CLI quanto na System Tray:

* **Via parâmetro de linha de comando (`-l` / `--lang`):**
  ```bash
  ./scarlett-control --lang es status    # Força espanhol
  ./scarlett-control --lang fr status    # Força francês
  ./scarlett-control --lang de status    # Força alemão
  ./scarlett-control --lang ja status    # Força japonês
  ./scarlett-control --lang zh status    # Força chinês
  ./scarlett-control -l pt tray          # Inicia bandeja em português
  ```

* **Via variável de ambiente (`SCARLETT_LANG`):**
  ```bash
  export SCARLETT_LANG=es
  ./scarlett-control status
  ```

---

## 📦 Instalação no Sistema

### Instalação para o usuário atual (recomendado):
```bash
make install-user
```
Isso instala:
* O executável em `~/.local/bin/scarlett-control`
* O atalho no lançador de aplicativos (`~/.local/share/applications/scarlett-control.desktop`)
* O autostart para iniciar automaticamente ao fazer login (`~/.config/autostart/scarlett-control.desktop`)

### Instalação global:
```bash
sudo make install
```
Instala em `/usr/local/bin` e `/usr/local/share/applications`.

---

## 🔍 Como funciona a comunicação ALSA

Diferente de interfaces genéricas, a linha Focusrite Scarlett (Gen 2, 3 e 4) expõe controles de mixer ALSA especializados criados pelo módulo `sound/usb/mixer_scarlett2.c`. O `scarlett-control`:
1. Faz a varredura automática dos dispositivos de controle `/dev/snd/controlC*`.
2. Identifica o hardware correspondente à Scarlett 2i2.
3. Lê e altera os numids de controle (`Line In Level`, `Air Capture`, `Phantom Power`, `Direct Monitor`).
4. Subscreve à fila de eventos ALSA do kernel para refletir mudanças do hardware na interface imediatamente.

---

## 🤖 Esteira de Integração Contínua (GitHub Actions)

O projeto inclui uma pipeline automatizada no GitHub Actions (`.github/workflows/build-and-release.yml`) que:

1. **Testa a compilação em múltiplas distros:**
   - Ubuntu Latest
   - Debian Stable
   - Fedora Latest
   - Arch Linux
   - Alpine Linux (musl)

2. **Gera pacotes e binários portáveis para todas as distribuições Linux:**
   - **`.AppImage` (Universal):** Executável autossuficiente compatível com qualquer distribuição Linux sem necessidade de instalação.
   - **`.deb`:** Pacote Debian/Ubuntu/Mint/Pop!_OS (`amd64` e `arm64`).
   - **`.rpm`:** Pacote Fedora/RHEL/CentOS/openSUSE/Bazzite (`x86_64` e `aarch64`).
   - **`.tar.gz`:** Arquivo portável com o binário compilado, ícone, arquivo `.desktop` e script `install.sh`.
   - **`PKGBUILD`:** Receita oficial para Arch Linux / Manjaro / SteamOS localizada em `packaging/arch/PKGBUILD`.

3. **Publicação Automática de Releases:**
   - Sempre que uma tag `v*` é criada (ex: `git tag v1.0.0 && git push --tags`) ou uma release é publicada no GitHub, a esteira anexa automaticamente todos os pacotes e o arquivo `SHA256SUMS.txt` aos Assets da Release!


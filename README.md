# Scarlett Control

[![CI / Build & Release](https://github.com/Kelvao/Scarlett-Control/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/Kelvao/Scarlett-Control/actions/workflows/build-and-release.yml)
[![Release](https://img.shields.io/github/v/release/Kelvao/Scarlett-Control?include_prereleases&style=flat-square)](https://github.com/Kelvao/Scarlett-Control/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20(x86__64%20%7C%20aarch64)-orange?style=flat-square)](#)

A high-performance, lightweight, zero-dependency C utility and system tray indicator for **Focusrite Scarlett 2i2** audio interfaces on Linux. 

Built on direct Linux kernel ALSA ioctl interfaces (`mixer_scarlett2`), **Scarlett Control** operates without bulky development dependencies, runtime bloat, or background CPU overhead.

---

## Key Features

* **Hardware Input Control:**
  * **Impedance Level:** Toggle between **Line** and **Inst** (Instrument) per channel (Input 1 & Input 2).
  * **AIR Mode:** Engage Focusrite's proprietary analog AIR presence boost per channel.
* **Phantom Power (+48V):**
  * Dedicated toggle for condenser microphones.
* **Direct Monitoring:**
  * Select between **Off** (host playback only), **Mono** (summed to both channels), and **Stereo** (Input 1 Left, Input 2 Right).
* **System Tray & Desktop Integration:**
  * Native system tray indicator compatible with KDE Plasma, GNOME (via AppIndicator / KStatusNotifierItem), XFCE, and other standard desktop environments.
  * **Start with system** toggle directly from the tray menu (creates/removes standard XDG autostart entries).
  * **Real-time Hardware Synchronization:** Hardware button presses and knob toggles on the physical Scarlett 2i2 reflect immediately in the tray menu via kernel ALSA event subscription (`SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS`).
* **Zero Runtime Overhead & Dynamic Linkage:**
  * Written in clean, standard C with zero external runtime requirements.
  * GUI libraries (`libgtk-3` / `libayatana-appindicator3` / `libappindicator3`) are loaded dynamically via `dlopen()`. In headless or pure CLI mode, GUI libraries are never initialized.
* **First-Class AppImage & Gear Lever Support:**
  * Self-contained AppImage builds with embedded update metadata (`gh-releases-zsync`) for seamless automatic updates with [Gear Lever](https://github.com/mijorus/gearlever) and AppImagePool.
* **Full CLI Suite:**
  * Comprehensive CLI commands (`status`, `set`, `toggle`, `watch`) with JSON export capability for scripts, window managers (Hyprland, Sway, i3), and keyboard shortcut daemon integration.
* **Multi-Language Support (i18n):**
  * Auto-detects system locale across 8 languages with embedded fallbacks and runtime overrides.

---

## System Tray Overview

When launched, the indicator stays active in your system tray:

```
Focusrite Scarlett 2i2 (USB)
─────────────────────────────
[ Input 1 ]
  [✓] Inst (Instrument)
  [ ] Air Mode
─────────────────────────────
[ Input 2 ]
  [ ] Inst (Instrument)
  [✓] Air Mode
─────────────────────────────
[ Power ]
  [✓] +48V Phantom Power
─────────────────────────────
[ Output (Direct Monitor) ]
  ( ) Direct Monitor: Off
  ( ) Direct Monitor: Mono
  (•) Direct Monitor: Stereo
─────────────────────────────
  [✓] Start with system
  Quit
```

---

## Installation & Packaging

Precompiled packages are available on the [Releases](https://github.com/Kelvao/Scarlett-Control/releases) page for every release:

### 1. AppImage (Universal Linux - Recommended)
Compatible with any modern Linux distribution:
```bash
chmod +x scarlett-control-x86_64.AppImage
./scarlett-control-x86_64.AppImage
```
> **Tip:** If you use **Gear Lever**, simply open the AppImage with Gear Lever to automatically integrate desktop shortcuts, icons, and GitHub one-click updates.

### 2. Debian / Ubuntu / Linux Mint / Pop!_OS (`.deb`)
```bash
sudo dpkg -i scarlett-control_*_amd64.deb
# or for ARM64:
sudo dpkg -i scarlett-control_*_arm64.deb
```

### 3. Fedora / RHEL / openSUSE / Bazzite (`.rpm`)
```bash
sudo rpm -i scarlett-control-*.x86_64.rpm
# or for aarch64:
sudo rpm -i scarlett-control-*.aarch64.rpm
```

### 4. Arch Linux / Manjaro / SteamOS
A standalone recipe is available in `packaging/arch/PKGBUILD`:
```bash
cd packaging/arch
makepkg -si
```

---

## Building from Source

The build system requires only standard GCC/Clang, `make`, and standard C library headers. No GUI development packages (`-dev`/`-devel`) are required.

```bash
git clone https://github.com/Kelvao/Scarlett-Control.git
cd Scarlett-Control
make
```

### Installation Targets
To install for the current user (`~/.local/bin` and `~/.local/share/applications`):
```bash
make install-user
```

To install globally for all users (`/usr/local/bin`):
```bash
sudo make install
```

To remove a previously installed local version:
```bash
make uninstall-user
```

---

## Command-Line Interface (CLI)

The CLI interface allows instant status inspection and seamless hardware control without opening the tray menu.

### Query Hardware Status

* **Human-readable overview:**
  ```bash
  scarlett-control status
  ```

* **Machine-readable JSON output (ideal for status bars like Waybar, Polybar, or scripts):**
  ```bash
  scarlett-control status --json
  ```
  ```json
  {
    "card_id": "USB",
    "card_index": 2,
    "input1": { "inst": true, "air": false },
    "input2": { "inst": false, "air": true },
    "phantom_48v": true,
    "direct_monitor": "stereo"
  }
  ```

### Change Hardware Settings (`set`)

Update one or more parameters in a single invocation:
```bash
# Enable Instrument and Air on Input 1, turn on 48V, and set Monitor to Stereo:
scarlett-control set --inst1 on --air1 on --48v on --output stereo

# Switch Direct Monitoring to Mono:
scarlett-control set --output mono

# Disable Phantom Power:
scarlett-control set --48v off
```

### Toggle Controls (`toggle`)

Designed for binding to global desktop shortcuts (e.g., KDE Shortcuts, GNOME Custom Shortcuts, Hyprland, Sway, or i3):
```bash
scarlett-control toggle air1      # Toggle Air mode on Input 1
scarlett-control toggle inst1     # Toggle Instrument mode on Input 1
scarlett-control toggle air2      # Toggle Air mode on Input 2
scarlett-control toggle inst2     # Toggle Instrument mode on Input 2
scarlett-control toggle 48v       # Toggle +48V Phantom Power
scarlett-control toggle monitor   # Cycle Monitor: Off -> Mono -> Stereo -> Off
```

### Real-Time Event Monitor (`watch`)

Listen directly to ALSA kernel events and display real-time updates when hardware buttons or knobs are physically toggled:
```bash
scarlett-control watch
```

---

## Internationalization (i18n)

**Scarlett Control** automatically detects your active session language using standard environment variables (`LANG`, `LC_MESSAGES`, `LC_ALL`). If no translation files are present on disk, it falls back gracefully to embedded internal defaults.

| Code | Language | Translation File |
|:---:|:---|:---|
| `en` | English | `locales/en.json` |
| `pt` / `pt_BR` | Português (Brasil) | `locales/pt_BR.json` |
| `es` | Español | `locales/es.json` |
| `fr` | Français | `locales/fr.json` |
| `de` | Deutsch | `locales/de.json` |
| `it` | Italiano | `locales/it.json` |
| `ja` | 日本語 | `locales/ja.json` |
| `zh` / `zh_CN` | 简体中文 | `locales/zh_CN.json` |

You can force a specific language at runtime via CLI or environment variable:

```bash
# Via command-line argument:
scarlett-control --lang de status
scarlett-control -l pt tray

# Via environment variable:
export SCARLETT_LANG=es
scarlett-control status
```

---

## Technical Architecture

Traditional GUI tools often depend on heavyweight graphical stacks and runtime interpreters. **Scarlett Control** is engineered with a minimalist systems-programming approach:

1. **Direct ALSA Kernel Interface:**
   Communicates directly with the Linux kernel's `sound/usb/mixer_scarlett2.c` driver via raw `ioctl` syscalls on `/dev/snd/controlC*`. No dependence on external sound server abstractions.
2. **Zero Polling CPU Waste:**
   Utilizes ALSA kernel event notifications via `SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS`. State synchronization occurs asynchronously without waking the CPU when idle.
3. **Dynamic Library Loading (`dlopen`):**
   GTK3 and AppIndicator libraries are resolved dynamically at runtime. If running in a headless server or purely via the CLI, graphical libraries are never loaded into memory.
4. **Resilient Reconnection:**
   If the audio interface is disconnected (unplugged USB cable or audio reset), the tray daemon remains stable and automatically re-establishes connection once the hardware is reattached.

---

## Continuous Integration & Release Automation

Every commit and pull request is verified via [GitHub Actions](https://github.com/Kelvao/Scarlett-Control/actions):
* **Cross-Distribution Compilation Tests:** Ubuntu, Debian, Fedora, Arch Linux, and Alpine Linux (`musl`).
* **Multi-Architecture Packaging:** Automated generation of `x86_64` and `aarch64` binaries, AppImages, Debian packages (`.deb`), Red Hat packages (`.rpm`), and tarballs.
* **Release Delivery:** Automated signing, checksum generation (`SHA256SUMS.txt`), and asset attachment upon tag creation.

---

## License

This project is open-source software licensed under the [MIT License](LICENSE).

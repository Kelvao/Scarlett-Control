#!/bin/bash
set -euo pipefail

VERSION="${1:-1.0.0}"
ARCH="${2:-amd64}"
TARGET_DIR="dist"
PKG_NAME="scarlett-control"
PKG_DIR="${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}"

echo "==> Gerando pacote Debian (.deb) para ${ARCH}..."
rm -rf "${PKG_DIR}"
mkdir -p "${PKG_DIR}/DEBIAN"
mkdir -p "${PKG_DIR}/usr/bin"
mkdir -p "${PKG_DIR}/usr/share/applications"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/scalable/apps"
mkdir -p "${PKG_DIR}/etc/xdg/autostart"

# Copy binary
cp scarlett-control "${PKG_DIR}/usr/bin/scarlett-control"
chmod 755 "${PKG_DIR}/usr/bin/scarlett-control"

# Copy desktop and icon
cp scarlett-control.desktop "${PKG_DIR}/usr/share/applications/scarlett-control.desktop"
cp scarlett-control.desktop "${PKG_DIR}/etc/xdg/autostart/scarlett-control.desktop"
cp assets/scarlett-control.svg "${PKG_DIR}/usr/share/icons/hicolor/scalable/apps/scarlett-control.svg"

# Copy locales
mkdir -p "${PKG_DIR}/usr/share/scarlett-control/locales"
cp locales/*.json "${PKG_DIR}/usr/share/scarlett-control/locales/"

# Generate control file
cat <<EOF > "${PKG_DIR}/DEBIAN/control"
Package: ${PKG_NAME}
Version: ${VERSION}
Section: sound
Priority: optional
Architecture: ${ARCH}
Maintainer: Community <scarlett-control@localhost>
Description: Controle de entradas (Inst, Air, 48V) e saída para Focusrite Scarlett 2i2
 Utilitário em C leve com suporte a linha de comando e ícone na bandeja do sistema
 (System Tray / AppIndicator) para controle de recursos de hardware da Scarlett 2i2.
Recommends: libayatana-appindicator3-1 | libappindicator3-1, libgtk-3-0
EOF

dpkg-deb --build --root-owner-group "${PKG_DIR}" "${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"
rm -rf "${PKG_DIR}"
echo "==> Pacote criado: ${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"


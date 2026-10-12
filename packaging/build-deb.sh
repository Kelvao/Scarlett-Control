#!/bin/bash
set -euo pipefail

VERSION="${1:-1.0.0}"
ARCH="${2:-amd64}"
TARGET_DIR="dist"
PKG_NAME="scarlett-control"
PKG_DIR="${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}"

echo "==> Building Debian package (.deb) for ${ARCH}..."
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
Maintainer: Kelvao <https://github.com/Kelvao/Scarlett-Control>
Description: Hardware input (Inst, Air, 48V) and output control for Focusrite Scarlett 2i2
 High-performance, lightweight C utility with CLI and system tray
 indicator (AppIndicator / StatusNotifierItem) for Focusrite Scarlett 2i2.
Recommends: libayatana-appindicator3-1 | libappindicator3-1, libgtk-3-0
EOF

dpkg-deb --build --root-owner-group "${PKG_DIR}" "${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"
rm -rf "${PKG_DIR}"
echo "==> Package created: ${TARGET_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"

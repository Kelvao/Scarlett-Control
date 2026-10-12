#!/bin/bash
set -euo pipefail

VERSION="${1:-1.0.0}"
ARCH="${2:-x86_64}"
TARGET_DIR="dist"
APP_DIR="/tmp/ScarlettControl.AppDir"

echo "==> Building AppImage for ${ARCH}..."
rm -rf "${APP_DIR}"
mkdir -p "${APP_DIR}/usr/bin"
mkdir -p "${APP_DIR}/usr/share/applications"
mkdir -p "${APP_DIR}/usr/share/icons/hicolor/scalable/apps"

# Copy binary
cp scarlett-control "${APP_DIR}/usr/bin/scarlett-control"
chmod 755 "${APP_DIR}/usr/bin/scarlett-control"

# Copy desktop and icons
cp scarlett-control.desktop "${APP_DIR}/scarlett-control.desktop"
cp scarlett-control.desktop "${APP_DIR}/usr/share/applications/scarlett-control.desktop"
echo "X-AppImage-Version=${VERSION}" >> "${APP_DIR}/scarlett-control.desktop"
echo "X-AppImage-Version=${VERSION}" >> "${APP_DIR}/usr/share/applications/scarlett-control.desktop"

# Create AppStream metainfo for software centers & Gear Lever
mkdir -p "${APP_DIR}/usr/share/metainfo"
cat <<EOF > "${APP_DIR}/usr/share/metainfo/scarlett-control.appdata.xml"
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>scarlett-control</id>
  <name>Scarlett 2i2 Control</name>
  <summary>Hardware control and system tray for Focusrite Scarlett 2i2</summary>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT</project_license>
  <url type="homepage">https://github.com/Kelvao/Scarlett-Control</url>
  <url type="bugtracker">https://github.com/Kelvao/Scarlett-Control/issues</url>
</component>
EOF

cp assets/scarlett-control.svg "${APP_DIR}/scarlett-control.svg"
cp assets/scarlett-control.svg "${APP_DIR}/.DirIcon"
cp assets/scarlett-control.svg "${APP_DIR}/usr/share/icons/hicolor/scalable/apps/scarlett-control.svg"

# Copy locales
mkdir -p "${APP_DIR}/usr/share/scarlett-control/locales"
cp locales/*.json "${APP_DIR}/usr/share/scarlett-control/locales/"
mkdir -p "${APP_DIR}/locales"
cp locales/*.json "${APP_DIR}/locales/"

# AppRun script
cat <<'EOF' > "${APP_DIR}/AppRun"
#!/bin/sh
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}/usr/bin:${PATH}"
exec "${HERE}/usr/bin/scarlett-control" "$@"
EOF
chmod +x "${APP_DIR}/AppRun"

# Download and extract appimagetool if not present
mkdir -p "${TARGET_DIR}"
if ! command -v appimagetool >/dev/null 2>&1; then
    if [ ! -f /tmp/squashfs-root/AppRun ]; then
        echo "Downloading appimagetool..."
        curl -fsSL -o /tmp/appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${ARCH}.AppImage"
        chmod +x /tmp/appimagetool
        (cd /tmp && ./appimagetool --appimage-extract)
    fi
    TOOL="/tmp/squashfs-root/AppRun"
else
    TOOL="appimagetool"
fi

UPDATE_INFO="gh-releases-zsync|Kelvao|Scarlett-Control|latest|scarlett-control-*${ARCH}.AppImage.zsync"

ARCH="${ARCH}" "${TOOL}" -u "${UPDATE_INFO}" "${APP_DIR}" "${TARGET_DIR}/scarlett-control-${VERSION}-${ARCH}.AppImage"

rm -rf "${APP_DIR}"
echo "==> AppImage and zsync generated successfully in ${TARGET_DIR}/"

#!/bin/bash
set -euo pipefail

VERSION="${1:-1.0.0}"
ARCH="${2:-x86_64}"
TARGET_DIR="dist"
RPMBUILD_DIR="/tmp/scarlett-rpmbuild"

echo "==> Building RPM package for ${ARCH}..."
rm -rf "${RPMBUILD_DIR}"
mkdir -p "${RPMBUILD_DIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy files to SOURCES
cp scarlett-control "${RPMBUILD_DIR}/SOURCES/"
cp scarlett-control.desktop "${RPMBUILD_DIR}/SOURCES/"
mkdir -p "${RPMBUILD_DIR}/SOURCES/assets"
cp assets/scarlett-control.svg "${RPMBUILD_DIR}/SOURCES/assets/"
mkdir -p "${RPMBUILD_DIR}/SOURCES/locales"
cp locales/*.json "${RPMBUILD_DIR}/SOURCES/locales/"
cp packaging/scarlett-control.spec "${RPMBUILD_DIR}/SPECS/"

rpmbuild --define "_topdir ${RPMBUILD_DIR}" \
         --define "_sourcedir ${RPMBUILD_DIR}/SOURCES" \
         --define "pkg_version ${VERSION}" \
         --target "${ARCH}" \
         -bb "${RPMBUILD_DIR}/SPECS/scarlett-control.spec"

mkdir -p "${TARGET_DIR}"
cp "${RPMBUILD_DIR}/RPMS/${ARCH}"/*.rpm "${TARGET_DIR}/"
rm -rf "${RPMBUILD_DIR}"
echo "==> RPM package generated in ${TARGET_DIR}/"

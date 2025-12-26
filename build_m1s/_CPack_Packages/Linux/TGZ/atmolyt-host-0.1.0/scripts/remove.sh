#!/bin/bash
# Uninstaller script for atmolyt-host
# Removes System V init service and files

set -e

PROJECT_NAME="atmolyt-host"
SERVICE_NAME="atmolyt"
INSTALL_DIR="/opt/${PROJECT_NAME}"
CONFIG_DIR="/etc/${PROJECT_NAME}"

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    exit 1
fi

echo "Removing ${PROJECT_NAME}..."

if [ -f "/etc/init.d/${SERVICE_NAME}" ]; then
    /etc/init.d/${SERVICE_NAME} stop 2>/dev/null || true
fi

if command -v update-rc.d >/dev/null 2>&1; then
    update-rc.d "${SERVICE_NAME}" remove 2>/dev/null || true
elif command -v chkconfig >/dev/null 2>&1; then
    chkconfig --del "${SERVICE_NAME}" 2>/dev/null || true
fi

rm -f "/etc/init.d/${SERVICE_NAME}"
rm -f "/etc/init.d/S99${SERVICE_NAME}"

rm -f "/var/run/atmolyt.pid"

rm -rf "$INSTALL_DIR"
rm -rf "$CONFIG_DIR"

echo "Removal complete!"
echo "Service ${SERVICE_NAME} has been removed"
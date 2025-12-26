#!/bin/bash
# Installer script for atmolyt-host
# Sets up System V init service from extracted package

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="atmolyt-host"
SERVICE_NAME="atmolyt"
INSTALL_DIR="/opt/${PROJECT_NAME}"
CONFIG_DIR="/etc/${PROJECT_NAME}"

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    exit 1
fi

echo "Installing ${PROJECT_NAME}..."

mkdir -p "$INSTALL_DIR/bin"
cp "${SCRIPT_DIR}/../bin/${PROJECT_NAME}" "${INSTALL_DIR}/bin/"
chmod +x "${INSTALL_DIR}/bin/${PROJECT_NAME}"

mkdir -p "$CONFIG_DIR"
cp -r "${SCRIPT_DIR}/../config/"* "$CONFIG_DIR/"

cat > "/etc/init.d/S99${SERVICE_NAME}" << 'EOF'
#!/bin/sh

DAEMON="/opt/atmolyt-host/bin/atmolyt-host"
DAEMON_ARGS="--config /etc/atmolyt-host/atmolyt.json"
PIDFILE="/var/run/atmolyt.pid"

start() {
    echo "Starting atmolyt..."
    start-stop-daemon -S -q -p $PIDFILE -m -b -x $DAEMON -- $DAEMON_ARGS
    echo "OK"
}

stop() {
    echo "Stopping atmolyt..."
    start-stop-daemon -K -q -p $PIDFILE
    rm -f $PIDFILE
    echo "OK"
}

restart() {
    stop
    sleep 1
    start
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart|reload)
        restart
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?
EOF

chmod +x "/etc/init.d/S99${SERVICE_NAME}"

rm -f "/etc/init.d/${SERVICE_NAME}"

echo "Note: Service will start automatically on boot (S99atmolyt)"

echo "Installation complete!"
echo "Service installed as: ${SERVICE_NAME}"
echo "Binary location: ${INSTALL_DIR}/bin/${PROJECT_NAME}"
echo "Config location: ${CONFIG_DIR}"
echo ""
echo "To start the service: /etc/init.d/S99${SERVICE_NAME} start"
echo "To stop the service: /etc/init.d/S99${SERVICE_NAME} stop"
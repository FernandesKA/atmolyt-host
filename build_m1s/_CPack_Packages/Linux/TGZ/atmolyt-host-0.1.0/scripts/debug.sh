#!/bin/bash

# This script is packaged with the application and run manually on the remote machine

set -e

# Paths (relative to the package root)
BINARY="./bin/atmolyt-host"
CONFIG="./config/atmolyt.json"

# Port selection priority:
#  1) 1st CLI arg
#  2) env GDBSERVER_PORT
#  3) default 2345
GDBSERVER_PORT="${1:-${GDBSERVER_PORT:-2345}}"

echo "Starting atmolyt-host with gdbserver on port $GDBSERVER_PORT..."

gdbserver ":${GDBSERVER_PORT}" "$BINARY" --config "$CONFIG"

echo "gdbserver finished."

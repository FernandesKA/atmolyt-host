#!/bin/bash

# This script is packaged with the application and run manually on the remote machine

set -e

# Paths (relative to the package root)
BINARY="./bin/atmolyt-host"
CONFIG="./config/atmolyt.json"
GDBSERVER_PORT=${GDBSERVER_PORT:2345}

echo "Starting atmolyt-host with gdbserver on port $GDBSERVER_PORT..."

gdbserver :$GDBSERVER_PORT "$BINARY" --config "$CONFIG"

echo "gdbserver finished."
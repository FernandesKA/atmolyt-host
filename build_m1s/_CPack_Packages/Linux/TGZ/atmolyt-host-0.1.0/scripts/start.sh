#!/bin/bash

# This script is packaged with the application and run manually on the remote machine

set -e

# Paths (relative to the package root)
BINARY="./bin/atmolyt-host"
CONFIG="./config/atmolyt.json"

echo "Starting atmolyt-host..."

"$BINARY" --config "$CONFIG"

echo "Application finished."
#!/usr/bin/env bash
# run.sh  –  Build and run the CodeX-PBL OS Simulation (IPC Addition Demo)
#
# Usage:
#   ./run.sh          # build and launch the UI server
#   ./run.sh clean    # remove built binaries

set -euo pipefail

MODE="${1:-start}"

if [ "$MODE" = "clean" ]; then
    make clean
    exit 0
fi

echo "=== Building Project ==="
make all

echo ""
echo "=== Starting Logger (background) ==="
rm -f sim.log*
build/logger &
LOGGER_PID=$!
sleep 0.3

cleanup() {
    echo ""
    echo "=== Shutting down Logger (PID $LOGGER_PID) ==="
    build/tests/test_sender shutdown 2>/dev/null || true
    wait $LOGGER_PID 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo ""
echo "=== Starting UI Server Bridge ==="
echo "Navigate to http://localhost:8080"
python3 ui/server.py

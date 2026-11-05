#!/usr/bin/env bash
# ==============================================================================
# run.sh - Build and Run the POSIX IPC Mechanisms Project
#
# Usage:
#   ./run.sh          # Build and launch interactive menu (C CLI)
#   ./run.sh gui      # Launch C-based Graphical User Interface
#   ./run.sh web      # Launch Web UI Dashboard & REST Bridge (http://localhost:8080)
#   ./run.sh test     # Run automated tests on all 6 IPC mechanisms
#   ./run.sh clean    # Clean build artifacts
# ==============================================================================

set -euo pipefail

MODE="${1:-menu}"

echo "============================================="
echo "   POSIX IPC MECHANISMS DEMONSTRATOR         "
echo "============================================="

if [ "$MODE" = "clean" ]; then
    echo "[*] Cleaning build artifacts..."
    make clean
    rm -f sim.log*
    exit 0
fi

echo "[*] Compiling C source files..."
make all

if [ "$MODE" = "test" ] || [ "$MODE" = "all" ]; then
    echo ""
    echo "[*] Running Automated IPC Tests..."
    ./ipc_project -a
    exit 0
fi

if [ "$MODE" = "gui" ]; then
    echo ""
    echo "[*] Launching C Graphical User Interface..."
    ./ipc_project --gui
    exit 0
fi

if [ "$MODE" = "web" ]; then
    echo ""
    echo "[*] Launching Web UI Dashboard Server..."
    echo "[*] Open your browser and navigate to: http://localhost:8080"
    python3 ui/server.py
    exit 0
fi

# Default: Interactive C Menu
./ipc_project

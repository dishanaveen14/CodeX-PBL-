#!/usr/bin/env bash
# run.sh  –  Build and run the Logger + Core together.
#
# Usage:
#   chmod +x run.sh
#   ./run.sh              # full demo (Logger + Core)
#   ./run.sh test         # Logger robustness tests only
#   ./run.sh test-core    # Core unit tests (with Logger)
#   ./run.sh clean        # remove built binaries

set -euo pipefail

MODE="${1:-demo}"

build() {
    echo "=== Building ==="
    make all
}

case "$MODE" in
demo)
    build
    echo ""
    echo "=== Starting Logger (background) ==="
    build/logger &
    LOGGER_PID=$!
    # Give the Logger a moment to create /sim_log
    sleep 0.3
    echo ""
    echo "=== Running Core ==="
    build/core/core
    echo ""
    echo "=== Sending Logger shutdown ==="
    build/tests/test_sender shutdown
    wait $LOGGER_PID
    echo ""
    echo "=== sim.log (last 30 lines) ==="
    tail -30 sim.log 2>/dev/null || echo "(no sim.log found)"
    ;;
test)
    make tests
    echo ""
    echo "=== Logger robustness tests ==="
    make test
    ;;
test-core)
    build
    echo ""
    echo "=== Starting Logger (background) ==="
    build/logger &
    LOGGER_PID=$!
    sleep 0.3
    echo ""
    echo "=== Core unit tests ==="
    build/core/test_driver
    echo ""
    echo "=== Sending Logger shutdown ==="
    build/tests/test_sender shutdown
    wait $LOGGER_PID
    echo ""
    echo "=== sim.log (last 40 lines) ==="
    tail -40 sim.log 2>/dev/null || echo "(no sim.log found)"
    ;;
clean)
    make clean
    ;;
*)
    echo "Usage: $0 [demo|test|test-core|clean]"
    exit 1
    ;;
esac

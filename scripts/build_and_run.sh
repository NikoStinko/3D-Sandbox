#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
LOG_DIR="$ROOT_DIR/logs"
mkdir -p "$BUILD_DIR" "$LOG_DIR"
{
  echo "[build] $(date) Configuring..."
  cmake ..
  echo "[build] $(date) Building..."
  make -j$(nproc)
  echo "[run] $(date) Running..."
  ./3D-Sandbox
} 2>&1 | tee -a "$LOG_DIR/App.log"

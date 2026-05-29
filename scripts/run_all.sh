#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build
cmake --build build -j

./build/detector_work macros/gamma.mac QGSP_BIC_HP
./build/detector_work macros/proton.mac QGSP_BIC_HP
./build/detector_work macros/bnct_uniform.mac QGSP_BIC_HP
./build/detector_work macros/bnct_shell.mac QGSP_BIC_HP

#!/bin/sh
# Build MARKET.COM with the flat assembler (fasm) on Linux/Unix
set -e
cd "$(dirname "$0")/SRC"
fasm MARKET.ASM ../MARKET.COM

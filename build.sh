#!/bin/sh
# Build MARKET.COM with the flat assembler
set -e

mkdir -p bin

fasm src/MARKET.ASM bin/MARKET.COM

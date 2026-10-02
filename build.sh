#!/bin/sh
# Build MARKET.COM with the flat assembler and the DOS text documentation
set -e

mkdir -p bin

fasm src/MARKET.ASM bin/MARKET.COM

python3 tools/md2txt.py README_tr-TR.md bin/READMETR.TXT cp857
python3 tools/md2txt.py README_en-US.md bin/READMEUS.TXT ascii
python3 tools/md2txt.py README_en-UK.md bin/READMEUK.TXT ascii

@ECHO OFF
REM Build MARKET.COM with the flat assembler (FASM for DOS)
CD SRC
FASM MARKET.ASM ..\MARKET.COM
CD ..

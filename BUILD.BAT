@ECHO OFF
REM Build MARKET.COM with the flat assembler
IF NOT EXIST bin\NUL MD bin
FASM src\MARKET.ASM bin\MARKET.COM
